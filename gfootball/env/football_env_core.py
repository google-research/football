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

from absl import logging

import copy

try:
  import gfootball_engine as libgame
  from gfootball_engine import GameState
except ImportError:
  print ('Cannot import gfootball_engine. Package was not installed properly.')
from gfootball.env import config as cfg
from gfootball.env import constants
from gfootball.env import football_action_set
from gfootball.env import observation_processor
import numpy as np
import six.moves.cPickle
from six.moves import range
import timeit

_unused_engines = []
_unused_rendering_engine = None
_active_rendering = False

try:
  import cv2
except ImportError:
  import cv2


class FootballEnvCore(object):

  def __init__(self, config):
    global _unused_engines
    self._config = config
    self._sticky_actions = football_action_set.get_sticky_actions(config)
    self._use_rendering_engine = False
    if _unused_engines:
      self._env = _unused_engines.pop()
    else:
      self._env = libgame.GameEnv()
    self._env.game_config.physics_steps_per_frame = config['physics_steps_per_frame']
    # Reset is needed here to make sure render() API call before reset() API
    # call works fine (get/setState makes sure env. config is the same).
    self.reset(inc=0)

  def _reset(self, animations, inc):
    global _unused_engines
    global _unused_rendering_engine
    assert (self._env.state == GameState.game_created or
            self._env.state == GameState.game_running or
            self._env.state == GameState.game_done)
    self._steps_time = 0
    self._step = 0
    self._observation = None
    self._info = None
    self._config.NewScenario(inc=inc)
    if self._env.state == GameState.game_created:
      self._env.start_game()
    self._env.reset(self._config.ScenarioConfig(), animations)
    self._env.state = GameState.game_running

  def reset(self, inc=1):
    """Reset environment for a new episode using a given config."""
    self._episode_start = timeit.default_timer()
    self._action_set = football_action_set.get_action_set(self._config)
    trace = observation_processor.ObservationProcessor(self._config)
    self._cumulative_reward = 0
    self._step_count = 0
    self._trace = trace
    self._reset(self._env.game_config.render, inc=inc)
    while not self._retrieve_observation():
      self._env.step()
    return True

  def _rendering_in_use(self):
    global _active_rendering
    if not self._use_rendering_engine:
      assert not _active_rendering, ('Environment does not support multiple '
                                     'rendering instances in the same process.')
      _active_rendering = True
      self._use_rendering_engine = True
    self._env.game_config.render = True

  def close(self):
    global _unused_engines
    global _unused_rendering_engine
    global _active_rendering
    if self._env:
      if self._use_rendering_engine:
        assert not _unused_rendering_engine
        _unused_rendering_engine = self._env
        _active_rendering = False
      else:
        _unused_engines.append(self._env)
      self._env = None

  def __del__(self):
    self.close()

  def step(self, action, extra_data={}):
    assert self._env.state != GameState.game_done, (
        'Cant call step() once episode finished (call reset() instead)')
    assert self._env.state == GameState.game_running, (
        'reset() must be called before step()')
    action = [
        football_action_set.named_action_from_action_set(self._action_set, a)
        for a in action
    ]
    self._step_count += 1
    # If agent 'holds' the game for too long, just start it.
    if self._env.waiting_for_game_count > 20:
      self._env.waiting_for_game_count = 0
      action = [football_action_set.action_short_pass] * (
          self._env.config.left_agents + self._env.config.right_agents)

    assert len(action) == (
        self._env.config.left_agents + self._env.config.right_agents)
    debug = {}
    debug['action'] = action
    action_index = 0
    for i in range(self._env.config.left_agents):
      player_action = action[action_index]
      action_index += 1
      assert isinstance(player_action, football_action_set.CoreAction)
      self._env.perform_action(player_action._backend_action, True, i)
    for i in range(self._env.config.right_agents):
      player_action = action[action_index]
      action_index += 1
      assert isinstance(player_action, football_action_set.CoreAction)
      self._env.perform_action(player_action._backend_action, False, i)
    while True:
      enter_time = timeit.default_timer()
      self._env.step()
      self._steps_time += timeit.default_timer() - enter_time
      if self._retrieve_observation():
        break
      if 'frame' in self._observation:
        self._trace.add_frame(self._observation['frame'])
    debug['frame_cnt'] = self._step

    previous_score_diff = 0
    if self._trace.len() > 0:
      a = self._trace[-1]['observation']['score'][0]
      b = self._trace[-1]['observation']['score'][1]
      previous_score_diff = a - b

    # Finish the episode on score.
    if self._env.config.end_episode_on_score:
      if self._observation['score'][0] > 0 or self._observation['score'][1] > 0:
        self._env.state = GameState.game_done

    # Finish the episode if the game is out of play (e.g. foul, corner etc)
    if (self._env.config.end_episode_on_out_of_play and
        self._trace.len() > 0 and self._observation['game_mode'] != int(
            libgame.e_GameMode.e_GameMode_Normal) and
        self._trace[-1]['observation']['game_mode'] == int(
            libgame.e_GameMode.e_GameMode_Normal)):
      self._env.state = GameState.game_done

    # End episode when team possessing the ball changes.
    if (self._env.config.end_episode_on_possession_change and
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
        self._env.state = GameState.game_done

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
      self._env.waiting_for_game_count += 1
    else:
      self._env.waiting_for_game_count = 0
    if self._step >= self._env.config.game_duration:
      self._env.state = GameState.game_done

    episode_done = self._env.state == GameState.game_done
    debug['time'] = timeit.default_timer()
    debug.update(extra_data)
    self._cumulative_reward += reward
    single_observation = copy.deepcopy(self._observation)
    trace = {
        'debug': debug,
        'observation': single_observation,
        'reward': reward,
        'cumulative_reward': self._cumulative_reward
    }
    self._trace.update(trace)
    if episode_done:
      self.write_dump('episode_done')
      fps = self._step_count / (debug['time'] - self._episode_start)
      game_fps = self._step_count / self._steps_time
      logging.info(
          'Episode reward: %.2f score: [%d, %d], steps: %d, '
          'FPS: %.1f, gameFPS: %.1f', self._cumulative_reward,
          single_observation['score'][0], single_observation['score'][1],
          self._step_count, fps, game_fps)
    return self._observation, reward, episode_done


  def _retrieve_observation(self):
    """Constructs observations exposed by the environment.

    Returns whether game
       is on or not.
    """
    info = self._env.get_info()
    result = {}
    if self._env.game_config.render:
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

    self._convert_players_observation(info.left_team, 'left_team', result)
    self._convert_players_observation(info.right_team, 'right_team', result)
    result['left_agent_sticky_actions'] = []
    result['left_agent_controlled_player'] = []
    result['right_agent_sticky_actions'] = []
    result['right_agent_controlled_player'] = []
    for i in range(self._env.config.left_agents):
      result['left_agent_controlled_player'].append(
          info.left_controllers[i].controlled_player)
      result['left_agent_sticky_actions'].append(
          np.array(self.sticky_actions_state(True, i), dtype=np.uint8))
    for i in range(self._env.config.right_agents):
      result['right_agent_controlled_player'].append(
          info.right_controllers[i].controlled_player)
      result['right_agent_sticky_actions'].append(
          np.array(self.sticky_actions_state(False, i), dtype=np.uint8))
    result['game_mode'] = int(info.game_mode)
    result['score'] = [info.left_goals, info.right_goals]
    result['ball_owned_team'] = info.ball_owned_team
    result['ball_owned_player'] = info.ball_owned_player
    result['steps_left'] = self._env.config.game_duration - info.step
    self._observation = result
    self._step = info.step
    self._info = info
    return info.is_in_play

  def _convert_players_observation(self, players, name, result):
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

  def observation(self):
    """Returns the current observation of the game."""
    assert (self._env.state == GameState.game_running or
            self._env.state == GameState.game_done), (
                'reset() must be called before observation()')
    return copy.deepcopy(self._observation)

  def sticky_actions_state(self, left_team, player_id):
    result = []
    for a in self._sticky_actions:
      result.append(
          self._env.sticky_action_state(a._backend_action, left_team,
                                        player_id))
    return np.uint8(result)

  def get_state(self, to_pickle):
    assert (self._env.state == GameState.game_running or
            self._env.state == GameState.game_done), (
                'reset() must be called before get_state()')
    pickle = six.moves.cPickle.dumps(to_pickle)
    return self._env.get_state(pickle)

  def set_state(self, state):
    assert (self._env.state == GameState.game_running or
            self._env.state == GameState.game_done), (
                'reset() must be called before set_state()')
    res = self._env.set_state(state)
    assert self._retrieve_observation()
    return six.moves.cPickle.loads(res)

  def tracker_setup(self, start, end):
    self._env.tracker_setup(start, end)

  def write_dump(self, name):
    return self._trace.write_dump(name)

  def render(self, mode):
    global _unused_rendering_engine
    if self._env.state == GameState.game_created:
      self._rendering_in_use()
      return False
    if not self._env.game_config.render:
      if not self._use_rendering_engine:
        if self._env.state != GameState.game_created:
          state = self.get_state("")
          self.close()
          if _unused_rendering_engine:
            self._env = _unused_rendering_engine
            _unused_rendering_engine = None
          else:
            self._env = libgame.GameEnv()
          self._rendering_in_use()
          self._reset(animations=False, inc=0)
          self.set_state(state)
          # We call render twice, as the first call has bad camera position.
          self._env.render(False)
      else:
        self._env.game_config.render = True
      self._env.render(True)
      self._retrieve_observation()
    if mode == 'rgb_array':
      frame = self._observation['frame']
      b,g,r = cv2.split(frame)
      return cv2.merge((r,g,b))
    elif mode == 'human':
      return True
    return False

  def disable_render(self):
    self._env.game_config.render = False
