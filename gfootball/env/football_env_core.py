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


"""Football environment as close as possible to a GYM environment."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import copy

import gfootball_engine as libgame
from gfootball.env import config as cfg
from gfootball.env import constants
from gfootball.env import football_action_set
import numpy as np
from six.moves import range
import timeit

_engine = None
_engine_in_use = False
_rendering_supported = False


class FootballEnvCore(object):

  def __init__(self, config):
    self._env = None
    self._config = config

  @cfg.log
  def reset(self, trace):
    global _engine
    global _engine_in_use
    global _rendering_supported
    """Reset environment for a new episode using a given config."""
    self._waiting_for_game_count = 0
    self._steps_time = 0
    self._step = 0
    self._trace = trace
    self._observation = None
    self._info = None
    self._done = False
    self._config.NewScenario()
    self._scenario_cfg = self._config.ScenarioConfig()
    if not self._env:
      assert not _engine_in_use, ('Environment does not support multiple '
                                  'instances of the game in the same process.')
      if not _engine:
        _engine = libgame.GameEnv()
        _engine.start_game(self._config.GameConfig())
        _rendering_supported = self._config['render']
      else:
        assert _rendering_supported or not self._config['render'], ('Enabling '
            'rendering when initially it was disabled is not supported.')
      _engine_in_use = True
      self._env = _engine
    self._left_controllers = []
    self._right_controllers = []
    for _ in range(self._scenario_cfg.left_agents):
      controller = football_action_set.StickyWrapper(self._config, self)
      self._left_controllers.append(controller)
    for _ in range(self._scenario_cfg.right_agents):
      controller = football_action_set.StickyWrapper(self._config, self)
      self._right_controllers.append(controller)
    self._env.reset(self._config.ScenarioConfig())
    while not self._retrieve_observation():
      self._env.step()
    return True

  def close(self):
    if self._env:
      global _engine_in_use
      _engine_in_use = False
      self._env = None

  def __del__(self):
    self.close()

  @cfg.log
  def step(self, action):
    # If agent 'holds' the game for too long, just start it.
    if self._waiting_for_game_count > 20:
      self._waiting_for_game_count = 0
      action = [football_action_set.action_short_pass] * (
          self._scenario_cfg.left_agents + self._scenario_cfg.right_agents)

    assert len(action) == (
        self._scenario_cfg.left_agents + self._scenario_cfg.right_agents)
    debug = {}
    if self._done:
      return copy.deepcopy(self._observation), 0, self._done, debug
    self._step += 1
    debug['action'] = action
    if self._step >= self._config['game_duration']:
      self._done = True
    self._left_team = True
    self._player_id = 0
    action_index = 0
    for _ in range(self._scenario_cfg.left_agents):
      player_action = action[action_index]
      action_index += 1
      assert isinstance(player_action, football_action_set.CoreAction)
      self._left_controllers[self._player_id].perform_action(player_action)
      self._player_id += 1
    self._player_id = 0
    self._left_team = False
    for _ in range(self._scenario_cfg.right_agents):
      player_action = action[action_index]
      action_index += 1
      assert isinstance(player_action, football_action_set.CoreAction)
      self._right_controllers[self._player_id].perform_action(player_action)
      self._player_id += 1
    while True:
      enter_time = timeit.default_timer()
      self._env.step()
      self._steps_time += timeit.default_timer() - enter_time
      if self._retrieve_observation():
        break
      if 'frame' in self._observation:
        self._trace.add_frame(self._observation['frame'])
    debug['frame_cnt'] = self._step
    if self._step == 1:
      # Put environment config into the debug, so that we can replay a given
      # scenario from the dump.
      debug['config'] = self._config.get_dictionary()

    previous_score_diff = 0
    if self._trace.len() > 0:
      a = self._trace[-1]['observation']['score'][0]
      b = self._trace[-1]['observation']['score'][1]
      previous_score_diff = a - b

    # Finish the episode on score.
    if self._config['end_episode_on_score']:
      if self._observation['score'][0] > 0 or self._observation['score'][1] > 0:
        self._done = True

    # Finish the episode if the game is out of play (e.g. foul, corner etc)
    if (self._config['end_episode_on_out_of_play'] and self._trace.len() > 0 and
        self._observation['game_mode'] != int(
            libgame.e_GameMode.e_GameMode_Normal) and
        self._trace[-1]['observation']['game_mode'] == int(
            libgame.e_GameMode.e_GameMode_Normal)):
      self._done = True

    # End episode when team possessing the ball changes.
    if (self._config['end_episode_on_possession_change'] and
        self._trace.len() > 0 and self._observation['ball_owned_team'] != -1):
      # We need to find the previous step with 'ball_owned_team' != -1 and
      # compare it to the 'ball_owned_team' from the recent but one (-2) step.
      current_posession = self._observation['ball_owned_team']
      prev_posession_id = self._trace.len() - 1
      while prev_posession_id > 0 and self._trace[prev_posession_id][
          'observation']['ball_owned_team'] == -1:
        prev_posession_id -= 1
      prev_posession = self._trace[prev_posession_id]['observation'][
          'ball_owned_team']
      if prev_posession != -1 and prev_posession != current_posession:
        self._done = True

    reward = (
        self._observation['score'][0] - self._observation['score'][1] -
        previous_score_diff)
    if reward == 1:
      self._trace.write_dump('score')
    elif reward == -1:
      self._trace.write_dump('lost_score')
    debug['reward'] = reward
    if self._observation['game_mode'] != int(
        libgame.e_GameMode.e_GameMode_Normal):
      self._waiting_for_game_count += 1
    else:
      self._waiting_for_game_count = 0
    return self._observation, reward, self._done, debug

  @cfg.log
  def _retrieve_observation(self):
    """Constructs observations exposed by the environment.

    Returns whether game
       is on or not.
    """
    info = self._env.get_info()
    if info.done:
      self._done = True
    result = {}
    if self._config['render']:
      frame = self._env.get_frame()
      frame = np.frombuffer(frame, dtype=np.uint8)
      frame = np.reshape(frame, [1280, 720, 3])
      frame = np.reshape(
          np.concatenate([frame[:, :, 0], frame[:, :, 1], frame[:, :, 2]]),
          [3, 720, 1280])
      frame = np.transpose(frame, [1, 2, 0])
      frame = np.flip(frame, 0)
      result['frame'] = frame
    result['ball'] = np.array(
        [info.ball_position[0], info.ball_position[1], info.ball_position[2]])
    # Ball's movement direction represented as [x, y] distance per step.
    result['ball_direction'] = np.array([
        info.ball_direction[0], info.ball_direction[1], info.ball_direction[2]
    ])
    # Ball's rotation represented as [x, y, z] rotation angle per step.
    result['ball_rotation'] = np.array(
        [info.ball_rotation[0], info.ball_rotation[1], info.ball_rotation[2]])

    self.convert_players_observation(info.left_team, 'left_team', result)
    self.convert_players_observation(info.right_team, 'right_team', result)
    result['left_agent_sticky_actions'] = []
    result['left_agent_controlled_player'] = []
    result['right_agent_sticky_actions'] = []
    result['right_agent_controlled_player'] = []
    for i in range(self._scenario_cfg.left_agents):
      if i >= len(info.left_controllers):
        result['left_agent_controlled_player'].append(-1)
        result['left_agent_sticky_actions'].append(
            np.zeros((len(football_action_set.get_sticky_actions(
                self._config))), dtype=np.uint8))
        continue
      result['left_agent_controlled_player'].append(
          info.left_controllers[i].controlled_player)
      result['left_agent_sticky_actions'].append(np.array(
          self._left_controllers[i].active_sticky_actions(), dtype=np.uint8))
    for i in range(self._scenario_cfg.right_agents):
      if i >= len(info.right_controllers):
        result['right_agent_controlled_player'].append(-1)
        result['right_agent_sticky_actions'].append(
            np.zeros((len(football_action_set.get_sticky_actions(
                self._config))), dtype=np.uint8))
        continue
      result['right_agent_controlled_player'].append(
          info.right_controllers[i].controlled_player)
      result['right_agent_sticky_actions'].append(np.array(
          self._right_controllers[i].active_sticky_actions(), dtype=np.uint8))
    result['game_mode'] = int(info.game_mode)
    result['score'] = [info.left_goals, info.right_goals]
    result['ball_owned_team'] = info.ball_owned_team
    result['ball_owned_player'] = info.ball_owned_player
    result['steps_left'] = self._config['game_duration'] - self._step
    self._observation = result
    self._info = info
    return info.is_in_play

  def convert_players_observation(self, players, name, result):
    """Converts internal players representation to the public one.

       Internal representation comes directly from gameplayfootball engine.
       Public representation is part of environment observations.

    Args:
      players: collection of team players to convert.
      name: name of the team being converted (left_team or right_team).
      result: collection where conversion result is added.
    """
    positions = []
    directions = []
    tired_factors = []
    active = []
    yellow_cards = []
    roles = []
    for player in players:
      positions.append(player.position[0])
      positions.append(player.position[1])
      directions.append(player.direction[0])
      directions.append(player.direction[1])
      tired_factors.append(player.tired_factor)
      active.append(player.is_active)
      yellow_cards.append(player.has_card)
      roles.append(player.role)
    result[name] = np.reshape(np.array(positions), [-1, 2])
    # Players' movement direction represented as [x, y] distance per step.
    result['{}_direction'.format(name)] = np.reshape(
        np.array(directions), [-1, 2])
    # Players' tired factor in the range [0, 1] (0 means not tired).
    result['{}_tired_factor'.format(name)] = np.array(tired_factors)
    result['{}_active'.format(name)] = np.array(active)
    result['{}_yellow_card'.format(name)] = np.array(yellow_cards)
    result['{}_roles'.format(name)] = np.array(roles)

  @cfg.log
  def observation(self):
    """Returns the current observation of the game."""
    return copy.deepcopy(self._observation)

  def perform_action(self, action):
    # Left team player 0 action...
    self._env.perform_action(action, self._left_team, self._player_id)
