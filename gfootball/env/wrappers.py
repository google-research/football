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


"""Environment that can be used with OpenAI Baselines."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from gfootball.env import observation_preprocessing
import gfootball_engine as libgame
import gym
import numpy as np
import cv2


class PeriodicDumpWriter(gym.Wrapper):
  """A wrapper that only dumps traces/videos periodically."""

  def __init__(self, env, dump_frequency):
    gym.Wrapper.__init__(self, env)
    self._dump_frequency = dump_frequency
    self._original_render = env._config['render']
    self._original_dump_config = {
        'write_video': env._config['write_video'],
        'dump_full_episodes': env._config['dump_full_episodes'],
        'dump_scores': env._config['dump_scores'],
    }
    self._current_episode_number = 0

  def step(self, action):
    return self.env.step(action)

  def reset(self):
    if (self._dump_frequency > 0 and
        (self._current_episode_number % self._dump_frequency == 0)):
      self.env._config.update(self._original_dump_config)
      self.env._config.update({'render': True})
    else:
      self.env._config.update({'render': self._original_render,
                               'write_video': False,
                               'dump_full_episodes': False,
                               'dump_scores': False})
    self._current_episode_number += 1
    return self.env.reset()


class Simple115StateWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to 115-features state."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    self.observation_space = gym.spaces.Box(
        low=-1, high=1, shape=(115,), dtype=np.float32)

  def observation(self, obs):
    o = []
    # ball position
    o.extend(obs['ball'])
    # ball direction
    o.extend(obs['ball_direction'])
    # one hot encoding of which team owns the ball
    if obs['ball_owned_team'] == -1:
      o.extend([1, 0, 0])
    if obs['ball_owned_team'] == 0:
      o.extend([0, 1, 0])
    if obs['ball_owned_team'] == 1:
      o.extend([0, 0, 1])

    active = [0] * 11
    active[obs['active']] = 1
    o.extend(active)
    o.extend(obs['home_team'].flatten())
    o.extend(obs['home_team_direction'].flatten())
    o.extend(obs['away_team'].flatten())
    o.extend(obs['away_team_direction'].flatten())

    game_mode = [0] * 7
    game_mode[obs['game_mode']] = 1
    o.extend(game_mode)
    return np.array(o, dtype=np.float32)


class Simple21StateWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to a 21-features state."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    self.observation_space = gym.spaces.Box(
        low=-1, high=1, shape=(21,), dtype=np.float32)

  def observation(self, obs):
    o = []
    # ball position
    o.extend(obs['ball'])
    # ball direction
    o.extend(obs['ball_direction'])
    # one hot encoding of which team owns the ball
    if obs['ball_owned_team'] == -1:
      o.extend([1, 0, 0])
    if obs['ball_owned_team'] == 0:
      o.extend([0, 1, 0])
    if obs['ball_owned_team'] == 1:
      o.extend([0, 0, 1])
    # position of an active player
    active_position = obs['home_team'][obs['active']]
    o.extend(active_position)
    # direction of an active player
    o.extend(obs['home_team_direction'][obs['active']])
    # away goalkeeper position
    for idx, role in enumerate(obs['away_team_roles']):
      if role == libgame.e_PlayerRole.e_PlayerRole_GK:
        o.extend(obs['away_team'][idx])
        o.extend(obs['away_team_direction'][idx])
        break
    # closest opponent
    _, opp_idx = min(
        [((opp_x - active_position[0])**2 + (opp_y - active_position[1])**2,
          idx)
         for idx, (opp_x, opp_y) in enumerate(obs['away_team'])])
    o.extend(obs['away_team'][opp_idx])
    o.extend(obs['away_team_direction'][opp_idx])
    return np.array(o, dtype=np.float32)


class PixelsStateWrapper(gym.ObservationWrapper):
  """A wrapper that extracts pixel representation."""

  def __init__(self, env, grayscale=True):
    gym.ObservationWrapper.__init__(self, env)
    self._grayscale = grayscale
    self._i=0
    self.observation_space = gym.spaces.Box(
        low=0, high=255,
        shape=(observation_preprocessing.SMM_HEIGHT,
               observation_preprocessing.SMM_WIDTH,
               1 if grayscale else 3),
        dtype=np.uint8)

  def observation(self, obs):
    frame = obs['frame']
    if self._grayscale:
      frame = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    frame = cv2.resize(frame, (observation_preprocessing.SMM_WIDTH,
                               observation_preprocessing.SMM_HEIGHT),
                       interpolation=cv2.INTER_AREA)
    if self._grayscale:
      frame = np.expand_dims(frame, -1)
    return frame


class SMMWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to a minimap."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    self.observation_space = gym.spaces.Box(
        low=0, high=255,
        shape=(observation_preprocessing.SMM_HEIGHT,
               observation_preprocessing.SMM_WIDTH,
               len(observation_preprocessing.SMM_LAYERS)),
        dtype=np.uint8)

  def observation(self, obs):
    return observation_preprocessing.generate_smm(obs)


class CheckpointRewardWrapper(gym.RewardWrapper):
  """A wrapper that adds a dense checkpoint reward."""

  def __init__(self, env):
    gym.RewardWrapper.__init__(self, env)
    self._collected_checkpoints = 0
    self._num_checkpoints = 10
    self._checkpoint_reward = 0.1

  def reset(self):
    self._collected_checkpoints = 0
    return self.env.reset()

  def reward(self, reward):
    if self.env.unwrapped.last_observation is None:
      return reward

    if reward == 1:
      reward += self._checkpoint_reward * (
          self._num_checkpoints - self._collected_checkpoints)
      self._collected_checkpoints = self._num_checkpoints
      return reward

    o = self.env.unwrapped.last_observation

    if 'active' not in o:
      return reward

    active = o['home_team'][o['active']]

    # Distance between the ball (or our player since we possess the ball)
    # and the goal of the opponent (coordinates = [1, 0]).
    d1 = ((o['ball'][0] - active[0]) ** 2 +
          (o['ball'][1] - active[1]) ** 2) ** 0.5
    if d1 > 0.03:
      return reward
    d = ((o['ball'][0] - 1) ** 2 + o['ball'][1] ** 2) ** 0.5

    # Collect the checkpoints.
    # We give reward for distance 1 to 0.2.
    while self._collected_checkpoints < self._num_checkpoints:
      if self._num_checkpoints == 1:
        threshold = 0.99 - 0.8
      else:
        threshold = (0.99 - 0.8 / (self._num_checkpoints - 1) *
                     self._collected_checkpoints)
      if d > threshold:
        break
      reward += self._checkpoint_reward
      self._collected_checkpoints += 1

    return reward

  def step(self, action):
    observation, reward, done, info = self.env.step(action)
    info['score_reward'] = float(reward)
    return observation, self.reward(reward), done, info
