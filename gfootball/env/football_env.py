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

from gfootball.env import constants
from gfootball.env import football_action_set
from gfootball.env import football_env_wrapper
from gfootball.env import observation_rotation
import gym
import numpy as np
from six.moves import range


class FootballEnv(gym.Env):
  """Allows multiple players to play in the same environment."""

  def __init__(self, config):
    self._config = config
    player_config = {'index': 0, 'action_set': self._config['action_set']}
    # There can be at most one agent at a time. We need to remember its
    # team and the index on the team to generate observations appropriately.
    self._agent = None
    self._agent_home_team = True
    self._agent_index = -1
    self._home_players = self._construct_players(config['home_players'],
                                                 player_config, True)
    self._away_players = self._construct_players(config['away_players'],
                                                 player_config, False)
    self._env = football_env_wrapper.FootballEnvWrapper(self._config)
    self._num_actions = len(football_action_set.get_action_set(self._config))
    self.last_observation = None

  @property
  def action_space(self):
    return gym.spaces.Discrete(self._num_actions)

  def _construct_players(self, definitions, config, home_team):
    result = []
    for definition in definitions:
      # Additional param passed by the command line to the player.
      config['param'] = ''
      if '=' in definition:
        (name, param) = definition.split('=')
        config['param'] = param
      else:
        name = definition
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
      player = player_factory.Player(config)
      if name == 'agent':
        assert not self._agent, 'Only one \'agent\' player allowed'
        self._agent = player
        self._agent_home_team = home_team
        self._agent_index = len(result)
      result.append(player)
      config['index'] += 1
    return result

  def _convert_observations(self, original, home_team, player_index):
    """Converts generic observations returned by the environment to
       the player specific observations.

    Args:
      original: original observations from the environment.
      home_team: is the player on the home team or not.
      player_index: index of the player for which to generate observations.
    """
    observations = {}
    for v in constants.EXPOSED_OBSERVATIONS:
      # Active and sticky_actions are added below.
      if v != 'active' and v != 'sticky_actions':
        observations[v] = copy.deepcopy(original[v])
    if home_team:
      observations['active'] = copy.deepcopy(
          original['home_agent_controlled_player'][player_index])
      observations['sticky_actions'] = copy.deepcopy(
          original['home_agent_sticky_actions'][player_index])
      if 'frame' in original:
        observations['frame'] = original['frame']
    else:
      # Currently we don't support rotating of the 'frame'.
      observations['opponent_active'] = copy.deepcopy(
          original['away_agent_controlled_player'][player_index])
      observations['opponent_sticky_actions'] = copy.deepcopy(
          original['away_agent_sticky_actions'][player_index])
      observations = observation_rotation.flip_observation(observations)
    diff = constants.EXPOSED_OBSERVATIONS.difference(observations.keys())
    assert not diff or (len(diff) == 1 and 'frame' in observations)
    return observations

  def _get_actions(self):
    obs = self._env.observation()
    actions = []
    for i in range(len(self._home_players)):
      adopted_obs = self._convert_observations(obs, True, i)
      actions.append(self._home_players[i].take_action(adopted_obs))
    for i in range(len(self._away_players)):
      adopted_obs = self._convert_observations(obs, False, i)
      action = self._away_players[i].take_action(adopted_obs)
      if self._away_players[i].can_play_right_to_left():
        action = observation_rotation.flip_action(action)
      actions.append(action)
    return actions

  def step(self, action):
    if self._agent:
      self._agent.set_action(action)
    observation, reward, done = self._env.step(self._get_actions())
    if self._agent:
      observation = self._convert_observations(observation,
                                               self._agent_home_team,
                                               self._agent_index)
    self.last_observation = observation
    return observation, np.array(reward, dtype=np.float32), done, {}

  def reset(self, config=None):
    if config is None:
      config = self._config
    self._env.reset(config)
    for player in self._home_players + self._away_players:
      player.reset()
    observation = self._env.observation()
    if self._agent:
      observation = self._convert_observations(observation,
                                               self._agent_home_team,
                                               self._agent_index)
    self.last_observation = observation
    return observation

  def write_dump(self, name):
    return self._env.write_dump(name)

  def close(self):
    pass
