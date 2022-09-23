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
  print('Cannot import gfootball_engine. Package was not installed properly.')
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



class EnvState(object):

  def __init__(self):
    self.previous_score_diff = 0
    self.previous_game_mode = -1
    self.prev_ball_owned_team = -1


class FootballEnvCore(object):

  def __init__(self, config):
    global _unused_engines
    self._config = config
    self._sticky_actions = football_action_set.get_sticky_actions(config)
    self._use_rendering_engine = False
    if _unused_engines:
      self._env = _unused_engines.pop()
    else:
      self._env = self._get_new_env()
    # Reset is needed here to make sure render() API call before reset() API
    # call works fine (get/setState makes sure env. config is the same).
    self.reset(inc=0)

  def _get_new_env(self):
    env = libgame.GameEnv()
    env.game_config.physics_steps_per_frame = self._config[
        'physics_steps_per_frame']
    env.game_config.render_resolution_x = self._config['render_resolution_x']
    env.game_config.render_resolution_y = self._config['render_resolution_y']
    return env

  def _reset(self, animations, inc):
    global _unused_engines
    global _unused_rendering_engine
    assert (self._env.state == GameState.game_created or
            self._env.state == GameState.game_running or
            self._env.state == GameState.game_done)
    # Variables that are part of the set_state/get_state snapshot.
    self._state = EnvState()
    # Variables being re-computed upon set_state call, no need to snapshot.
    self._observation = None
    # Not snapshoted variables.
    self._steps_time = 0
    self._step = 0
    self._config.NewScenario(inc=inc)
    if self._env.state == GameState.game_created:
      self._env.start_game()
    self._env.state = GameState.game_running
    scenario_config = self._config.ScenarioConfig()
    assert (
        not scenario_config.dynamic_player_selection or
        not scenario_config.control_all_players
    ), ('For this scenario you need to control either 0 or all players on the '
        'team ({} for left team, {} for right team).').format(
            scenario_config.controllable_left_players,
            scenario_config.controllable_right_players)
    self._env.reset(scenario_config, animations)

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

  def _release_engine(self):
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

  def close(self):
    self._release_engine()
    if self._trace:
      del self._trace
      self._trace = None

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
    assert len(action) == (
        self._env.config.left_agents + self._env.config.right_agents)
    debug = {}
    debug['action'] = action
    action_index = 0
    for left_team in [True, False]:
      agents = self._env.config.left_agents if left_team else self._env.config.right_agents
      for i in range(agents):
        player_action = action[action_index]

        # If agent 'holds' the game for too long, just start it.
        if self._env.waiting_for_game_count == 20:
          player_action = football_action_set.action_short_pass
        elif self._env.waiting_for_game_count > 20:
          player_action = football_action_set.action_idle
          controlled_players = self._observation[
              'left_agent_controlled_player'] if left_team else self._observation[
                  'right_agent_controlled_player']
          if self._observation['ball_owned_team'] != -1 and self._observation[
              'ball_owned_team'] ^ left_team and controlled_players[
                  i] == self._observation['ball_owned_player']:
            if bool(self._env.waiting_for_game_count < 30) != bool(left_team):
              player_action = football_action_set.action_left
            else:
              player_action = football_action_set.action_right
        action_index += 1
        assert isinstance(player_action, football_action_set.CoreAction)
        self._env.perform_action(player_action._backend_action, left_team, i)
    while True:
      enter_time = timeit.default_timer()
      self._env.step()
      self._steps_time += timeit.default_timer() - enter_time
      if self._retrieve_observation():
        break
      if 'frame' in self._observation:
        self._trace.add_frame(self._observation['frame'])
    debug['frame_cnt'] = self._step

    # Finish the episode on score.
    if self._env.config.end_episode_on_score:
      if self._observation['score'][0] > 0 or self._observation['score'][1] > 0:
        self._env.state = GameState.game_done

    # Finish the episode if the game is out of play (e.g. foul, corner etc)
    if (self._env.config.end_episode_on_out_of_play and
        self._observation['game_mode'] != int(
            libgame.e_GameMode.e_GameMode_Normal) and
        self._state.previous_game_mode == int(
            libgame.e_GameMode.e_GameMode_Normal)):
      self._env.state = GameState.game_done
    self._state.previous_game_mode = self._observation['game_mode']

    # End episode when team possessing the ball changes.
    if (self._env.config.end_episode_on_possession_change and
        self._observation['ball_owned_team'] != -1 and
        self._state.prev_ball_owned_team != -1 and
        self._observation['ball_owned_team'] !=
        self._state.prev_ball_owned_team):
      self._env.state = GameState.game_done
    if self._observation['ball_owned_team'] != -1:
      self._state.prev_ball_owned_team = self._observation['ball_owned_team']

    # Compute reward.
    score_diff = self._observation['score'][0] - self._observation['score'][1]
    reward = score_diff - self._state.previous_score_diff
    self._state.previous_score_diff = score_diff
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
    info = {}
    self._trace.update(trace)
    dumps = self._trace.process_pending_dumps(episode_done)
    if dumps:
      info['dumps'] = dumps
    if episode_done:
      del self._trace
      self._trace = None
      fps = self._step_count / (debug['time'] - self._episode_start)
      game_fps = self._step_count / self._steps_time
      logging.info(
          'Episode reward: %.2f score: [%d, %d], steps: %d, '
          'FPS: %.1f, gameFPS: %.1f', self._cumulative_reward,
          single_observation['score'][0], single_observation['score'][1],
          self._step_count, fps, game_fps)
    if self._step_count == 1:
      # Start writing episode_done
      self.write_dump('episode_done')
    return self._observation, reward, episode_done, info

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
      frame = np.reshape(frame, [
          self._config['render_resolution_x'],
          self._config['render_resolution_y'], 3
      ])
      frame = np.reshape(
          np.concatenate([frame[:, :, 0], frame[:, :, 1], frame[:, :, 2]]), [
              3, self._config['render_resolution_y'],
              self._config['render_resolution_x']
          ])
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
    designated_player = -1
    for id, player in enumerate(players):
      positions.append(player.position[0])
      positions.append(player.position[1])
      directions.append(player.direction[0])
      directions.append(player.direction[1])
      tired_factors.append(player.tired_factor)
      active.append(player.is_active)
      yellow_cards.append(player.has_card)
      roles.append(player.role)
      if player.designated_player:
        designated_player = id
    result[name] = np.reshape(np.array(positions), [-1, 2])
    # Players' movement direction represented as [x, y] distance per step.
    result['{}_direction'.format(name)] = np.reshape(
        np.array(directions), [-1, 2])
    # Players' tired factor in the range [0, 1] (0 means not tired).
    result['{}_tired_factor'.format(name)] = np.array(tired_factors)
    result['{}_active'.format(name)] = np.array(active)
    result['{}_yellow_card'.format(name)] = np.array(yellow_cards)
    result['{}_roles'.format(name)] = np.array(roles)
    result['{}_designated_player'.format(name)] = designated_player

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
    to_pickle['FootballEnvCore'] = self._state
    pickle = six.moves.cPickle.dumps(to_pickle)
    return self._env.get_state(pickle)

  def set_state(self, state):
    assert (self._env.state == GameState.game_running or
            self._env.state == GameState.game_done), (
                'reset() must be called before set_state()')
    res = self._env.set_state(state)
    assert self._retrieve_observation()
    from_picle = six.moves.cPickle.loads(res)
    self._state = from_picle['FootballEnvCore']
    if self._trace is None:
      self._trace = observation_processor.ObservationProcessor(self._config)
    return from_picle

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
          state = self.get_state({})
          self._release_engine()
          if _unused_rendering_engine:
            self._env = _unused_rendering_engine
            _unused_rendering_engine = None
          else:
            self._env = self._get_new_env()
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
      b, g, r = cv2.split(frame)
      return cv2.merge((r, g, b))
    elif mode == 'human':
      return True
    return False

  def disable_render(self):
    self._env.game_config.render = False
