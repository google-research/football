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


"""Football environment wrapper which supports multiple as close as possible to a GYM environment."""

import copy
import logging
import timeit

from gfootball.env import config as cfg
from gfootball.env import football_action_set
from gfootball.env import football_env_core
from gfootball.env import observation_processor


class FootballEnvWrapper(object):

  def __init__(self, config):
    self._config = config
    self._env = football_env_core.FootballEnvCore(config)
    self._env_state = 'initialized'

  @cfg.log
  def state(self):
    return self._env_state

  @cfg.log
  def reset(self):
    """Reset environment for a new episode using a given config."""
    self._episode_start = timeit.default_timer()
    self._action_set = football_action_set.get_action_set(self._config)
    self._trace = observation_processor.ObservationProcessor(self._config)
    self._cumulative_reward = 0
    self._step_count = 0
    self._env.reset(self._trace)
    self._env_state = 'game_started'

  @cfg.log
  def write_dump(self, name):
    return self._trace.write_dump(name)

  @cfg.log
  def step(self, action, extra_data={}):
    if self._env_state != 'game_started':
      raise RuntimeError('invalid game state: {}'.format(self._env_state))
    action = [
        football_action_set.named_action_from_action_set(self._action_set, a)
        for a in action
    ]
    self._step_count += 1
    observation, reward, done, debug = self._env.step(action)
    debug['time'] = timeit.default_timer()
    debug.update(extra_data)
    self._cumulative_reward += reward
    single_observation = copy.deepcopy(observation)
    trace = {
        'debug': debug,
        'observation': single_observation,
        'reward': reward,
        'cumulative_reward': self._cumulative_reward
    }
    self._trace.update(trace)
    if done:
      self.write_dump('episode_done')
    if done:
      self._env_state = 'game_done'
      fps = self._step_count / (debug['time'] - self._episode_start)
      game_fps = self._step_count / self._env._steps_time
      logging.info(
          'Episode reward: %.2f score: [%d, %d], steps: %d, '
          'FPS: %.1f, gameFPS: %.1f', self._cumulative_reward,
          single_observation['score'][0], single_observation['score'][1],
          self._step_count, fps, game_fps)
    return observation, reward, done

  @cfg.log
  def observation(self):
    return self._env.observation()

  def close(self):
    self._env.close()
