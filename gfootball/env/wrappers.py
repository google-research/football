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
    shape = (self.env.unwrapped._config.number_of_players_agent_controls(), 115)
    self.observation_space = gym.spaces.Box(
        low=-1, high=1, shape=shape, dtype=np.float32)

  def observation(self, obs):
    """Converts an observation into simple115 format.

    Args:
      obs: observation that the environment returns

    Returns:
      (N, 155) shaped representation, where N stands for the number of players
      being controlled.
    """
    final_obs = []
    for a in obs['active']:
      o = []
      o.extend(obs['left_team'].flatten())
      o.extend(obs['left_team_direction'].flatten())
      o.extend(obs['right_team'].flatten())
      o.extend(obs['right_team_direction'].flatten())

      # If there were less than 11vs11 players we backfill missing values with
      # -1.
      # 88 = 11 (players) * 2 (teams) * 2 (positions & directions) * 2 (x & y)
      if len(o) < 88:
        o.extend([-1] * (88 - len(o)))

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
      active[a] = 1
      o.extend(active)

      game_mode = [0] * 7
      game_mode[obs['game_mode']] = 1
      o.extend(game_mode)
      final_obs.append(o)
    return np.array(final_obs, dtype=np.float32)


class PixelsStateWrapper(gym.ObservationWrapper):
  """A wrapper that extracts pixel representation."""

  def __init__(self, env, grayscale=True,
               channel_dimensions=(observation_preprocessing.SMM_WIDTH,
                                   observation_preprocessing.SMM_HEIGHT)):
    gym.ObservationWrapper.__init__(self, env)
    self._grayscale = grayscale
    self._channel_dimensions = channel_dimensions
    self._i=0
    self.observation_space = gym.spaces.Box(
        low=0, high=255,
        shape=(self.env.unwrapped._config.number_of_players_agent_controls(),
               channel_dimensions[1], channel_dimensions[0],
               1 if grayscale else 3),
        dtype=np.uint8)

  def observation(self, obs):
    frame = obs['frame']
    if self._grayscale:
      frame = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    frame = cv2.resize(frame, (self._channel_dimensions[0],
                               self._channel_dimensions[1]),
                       interpolation=cv2.INTER_AREA)
    if self._grayscale:
      frame = np.expand_dims(frame, -1)
    return ([frame] *
            self.env.unwrapped._config.number_of_players_agent_controls())


class SMMWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to a minimap."""

  def __init__(self, env,
               channel_dimensions=(observation_preprocessing.SMM_WIDTH,
                                   observation_preprocessing.SMM_HEIGHT)):
    gym.ObservationWrapper.__init__(self, env)
    self._channel_dimensions = channel_dimensions
    shape=(self.env.unwrapped._config.number_of_players_agent_controls(),
           channel_dimensions[1], channel_dimensions[0],
           len(observation_preprocessing.get_smm_layers(
               self.env.unwrapped._config)))
    self.observation_space = gym.spaces.Box(
        low=0, high=255, shape=shape, dtype=np.uint8)

  def observation(self, obs):
    return observation_preprocessing.generate_smm(
        obs, channel_dimensions=self._channel_dimensions,
        config=self.env.unwrapped._config)


class SingleAgentWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to a minimap."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    self.observation_space = gym.spaces.Box(
        low=env.observation_space.low[0],
        high=env.observation_space.high[0],
        dtype=env.observation_space.dtype)

  def observation(self, obs):
    return obs[0]


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
    is_left_to_right = o['is_active_left']

    # Check if left team player has the ball.
    if ('ball_owned_team' not in o or
        o['ball_owned_team'] != (0 if is_left_to_right else 1) or
        'ball_owned_player' not in o or o['ball_owned_player'] == -1):
      return reward

    if is_left_to_right:
      d = ((o['ball'][0] - 1) ** 2 + o['ball'][1] ** 2) ** 0.5
    else:
      d = ((o['ball'][0] + 1) ** 2 + o['ball'][1] ** 2) ** 0.5

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
