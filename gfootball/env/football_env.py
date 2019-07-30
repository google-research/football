# coding=utf-8
# Copyright 2019 Google LLC
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""Allows different types of players to play against each other."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import copy
import importlib
import logging

from gfootball.env import config as cfg
from gfootball.env import constants
from gfootball.env import football_action_set
from gfootball.env import football_env_wrapper
import gym
import numpy as np


class FootballEnv(gym.Env):
  """Allows multiple players to play in the same environment."""

  def __init__(self, config):
    self._config = config
    player_config = {'index': 0}
    # There can be at most one agent at a time. We need to remember its
    # team and the index on the team to generate observations appropriately.
    self._agent = None
    self._agent_index = -1
    self._players = self._construct_players(config['players'], player_config)
    self._env = football_env_wrapper.FootballEnvWrapper(self._config)
    self._num_actions = len(football_action_set.get_action_set(self._config))
    self.last_observation = None
    self._last_swapped_sides = False

  @property
  def action_space(self):
    if self._config.number_of_players_agent_controls() > 1:
      return gym.spaces.MultiDiscrete(
          [self._num_actions] * self._config.number_of_players_agent_controls())
    return gym.spaces.Discrete(self._num_actions)

  def _construct_players(self, definitions, config):
    result = []
    left_position = 0
    right_position = 0
    for definition in definitions:
      (name, d) = cfg.parse_player_definition(definition)
      config_name = 'player_{}'.format(name)
      if config_name in config:
        config[config_name] += 1
      else:
        config[config_name] = 0
      try:
        player_factory = importlib.import_module(
            'gfootball.env.players.{}'.format(name))
      except ImportError as e:
        logging.warning('Failed loading player "%s"', name)
        logging.warning(e)
        exit(1)
      player_config = copy.deepcopy(config)
      player_config.update(d)
      player = player_factory.Player(player_config, self._config)
      if name == 'agent':
        assert not self._agent, 'Only one \'agent\' player allowed'
        self._agent = player
        self._agent_index = len(result)
        self._agent_left_position = left_position
        self._agent_right_position = right_position
      result.append(player)
      left_position += player.num_controlled_left_players()
      right_position += player.num_controlled_right_players()
      config['index'] += 1
    return result

  def _convert_observations(self, original, player,
                            left_player_position, right_player_position):
    """Converts generic observations returned by the environment to
       the player specific observations.

    Args:
      original: original observations from the environment.
      player: player for which to generate observations.
      left_player_position: index into observation corresponding to the left
          player.
      right_player_position: index into observation corresponding to the right
          player.
    """
    observations = []
    for is_left in [True, False]:
      prefix = 'left' if is_left else 'right'
      position = left_player_position if is_left else right_player_position
      for x in range(player.num_controlled_left_players() if is_left
                     else player.num_controlled_right_players()):
        o = {}
        for v in constants.EXPOSED_OBSERVATIONS:
          # Active and sticky_actions are added below.
          if v != 'active' and v != 'sticky_actions':
            o[v] = copy.deepcopy(original[v])
        assert (len(original[prefix + '_agent_controlled_player']) ==
                len(original[prefix + '_agent_sticky_actions']))
        if position + x >= len(original[prefix + '_agent_controlled_player']):
          o['active'] = -1
          o['sticky_actions'] = []
        else:
          o['active'] = (
              original[prefix + '_agent_controlled_player'][position + x])
          o['sticky_actions'] = copy.deepcopy(
              original[prefix + '_agent_sticky_actions'][position + x])
        o['is_left'] = is_left
        if 'frame' in original:
          o['frame'] = original['frame']
        observations.append(o)
    return observations

  def _get_actions(self):
    obs = self._env.observation()
    actions = []
    left_player_position = 0
    right_player_position = 0
    for player in self._players:
      adopted_obs = self._convert_observations(obs, player,
                                               left_player_position,
                                               right_player_position)
      left_player_position += player.num_controlled_left_players()
      right_player_position += player.num_controlled_right_players()
      a = player.take_action(adopted_obs)
      if isinstance(a, np.ndarray):
        a = a.tolist()
      if not isinstance(a, list):
        a = [a]
      assert len(adopted_obs) == len(
          a), 'Player returned {} actions instead of {}.'.format(
              len(a), len(adopted_obs))
      actions.extend(a)
    return actions

  def step(self, action):
    if self._agent:
      self._agent.set_action(action)
    observation, reward, done = self._env.step(self._get_actions())
    score_reward = reward
    if self._agent:
      observation = self._convert_observations(observation, self._agent,
                                               self._agent_left_position,
                                               self._agent_right_position)
      reward = ([reward] * self._agent.num_controlled_left_players() +
                [-reward] * self._agent.num_controlled_right_players())
    self.last_observation = observation
    return (observation, np.array(reward, dtype=np.float32), done,
            {'score_reward': score_reward})

  def reset(self):
    self._env.reset()
    if self._config['swap_sides'] != self._last_swapped_sides:
      for player in self._players:
        player.swap_sides()
      self._agent_left_position, self._agent_right_position = (
          self._agent_right_position, self._agent_left_position)
      self._last_swapped_sides = self._config['swap_sides']
    for player in self._players:
      player.reset()
    observation = self._env.observation()
    if self._agent:
      observation = self._convert_observations(observation, self._agent,
                                               self._agent_left_position,
                                               self._agent_right_position)
    self.last_observation = observation
    return observation

  def write_dump(self, name):
    return self._env.write_dump(name)

  def close(self):
    self._env.close()
