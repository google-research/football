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

import collections
import cv2
from gfootball.env import observation_preprocessing
import gym
import numpy as np


class GetStateWrapper(gym.Wrapper):
  """A wrapper that only dumps traces/videos periodically."""

  def __init__(self, env):
    gym.Wrapper.__init__(self, env)
    self._wrappers_with_support = {
        'CheckpointRewardWrapper', 'FrameStack', 'GetStateWrapper',
        'SingleAgentRewardWrapper', 'SingleAgentObservationWrapper',
        'SMMWrapper', 'PeriodicDumpWriter', 'Simple115StateWrapper',
        'PixelsStateWrapper'
    }

  def _check_state_supported(self):
    o = self
    while True:
      name = o.__class__.__name__
      if o.__class__.__name__ == 'FootballEnv':
        break
      assert name in self._wrappers_with_support, (
          'get/set state not supported'
          ' by {} wrapper').format(name)
      o = o.env

  def get_state(self):
    self._check_state_supported()
    to_pickle = {}
    return self.env.get_state(to_pickle)

  def set_state(self, state):
    self._check_state_supported()
    self.env.set_state(state)


class PeriodicDumpWriter(gym.Wrapper):
  """A wrapper that only dumps traces/videos periodically."""

  def __init__(self, env, dump_frequency):
    gym.Wrapper.__init__(self, env)
    self._dump_frequency = dump_frequency
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
      self.env.render()
    else:
      self.env._config.update({'write_video': False,
                               'dump_full_episodes': False,
                               'dump_scores': False})
      self.env.disable_render()
    self._current_episode_number += 1
    return self.env.reset()


class Simple115StateWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to 115-features state."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    shape = (self.env.unwrapped._config.number_of_players_agent_controls(), 115)
    self.observation_space = gym.spaces.Box(
        low=-1, high=1, shape=shape, dtype=np.float32)

  def observation(self, observation):
    """Converts an observation into simple115 format.

    Args:
      observation: observation that the environment returns

    Returns:
      (N, 115) shaped representation, where N stands for the number of players
      being controlled.
    """
    final_obs = []
    for obs in observation:
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
      if obs['active'] != -1:
        active[obs['active']] = 1
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
    self.observation_space = gym.spaces.Box(
        low=0, high=255,
        shape=(self.env.unwrapped._config.number_of_players_agent_controls(),
               channel_dimensions[1], channel_dimensions[0],
               1 if grayscale else 3),
        dtype=np.uint8)

  def observation(self, obs):
    o = []
    for observation in obs:
      frame = observation['frame']
      if self._grayscale:
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
      frame = cv2.resize(frame, (self._channel_dimensions[0],
                                 self._channel_dimensions[1]),
                         interpolation=cv2.INTER_AREA)
      if self._grayscale:
        frame = np.expand_dims(frame, -1)
      o.append(frame)
    return np.array(o, dtype=np.uint8)


class SMMWrapper(gym.ObservationWrapper):
  """A wrapper that returns an observation only for the first agent."""

  def __init__(self, env,
               channel_dimensions=(observation_preprocessing.SMM_WIDTH,
                                   observation_preprocessing.SMM_HEIGHT)):
    gym.ObservationWrapper.__init__(self, env)
    self._channel_dimensions = channel_dimensions
    shape = (self.env.unwrapped._config.number_of_players_agent_controls(),
             channel_dimensions[1], channel_dimensions[0],
             len(observation_preprocessing.get_smm_layers(
                 self.env.unwrapped._config)))
    self.observation_space = gym.spaces.Box(
        low=0, high=255, shape=shape, dtype=np.uint8)

  def observation(self, obs):
    return observation_preprocessing.generate_smm(
        obs, channel_dimensions=self._channel_dimensions,
        config=self.env.unwrapped._config)


class SingleAgentObservationWrapper(gym.ObservationWrapper):
  """A wrapper that returns a reward only for the first agent."""

  def __init__(self, env):
    gym.ObservationWrapper.__init__(self, env)
    self.observation_space = gym.spaces.Box(
        low=env.observation_space.low[0],
        high=env.observation_space.high[0],
        dtype=env.observation_space.dtype)

  def observation(self, obs):
    return obs[0]


class SingleAgentRewardWrapper(gym.RewardWrapper):
  """A wrapper that converts an observation to a minimap."""

  def __init__(self, env):
    gym.RewardWrapper.__init__(self, env)

  def reward(self, reward):
    return reward[0]


class CheckpointRewardWrapper(gym.RewardWrapper):
  """A wrapper that adds a dense checkpoint reward."""

  def __init__(self, env):
    gym.RewardWrapper.__init__(self, env)
    self._collected_checkpoints = {}
    self._num_checkpoints = 10
    self._checkpoint_reward = 0.1

  def reset(self):
    self._collected_checkpoints = {}
    return self.env.reset()

  def get_state(self, to_pickle):
    to_pickle['CheckpointRewardWrapper'] = self._collected_checkpoints
    return self.env.get_state(to_pickle)

  def set_state(self, state):
    from_pickle = self.env.set_state(state)
    self._collected_checkpoints = from_pickle['CheckpointRewardWrapper']
    return from_pickle

  def reward(self, reward):
    observation = self.env.unwrapped.observation()
    if observation is None:
      return reward

    assert len(reward) == len(observation)

    for rew_index in range(len(reward)):
      o = observation[rew_index]
      if reward[rew_index] == 1:
        reward[rew_index] += self._checkpoint_reward * (
            self._num_checkpoints -
            self._collected_checkpoints.get(rew_index, 0))
        self._collected_checkpoints[rew_index] = self._num_checkpoints
        continue

      # Check if the active player has the ball.
      if ('ball_owned_team' not in o or
          o['ball_owned_team'] != 0 or
          'ball_owned_player' not in o or
          o['ball_owned_player'] != o['active']):
        continue

      d = ((o['ball'][0] - 1) ** 2 + o['ball'][1] ** 2) ** 0.5

      # Collect the checkpoints.
      # We give reward for distance 1 to 0.2.
      while (self._collected_checkpoints.get(rew_index, 0) <
             self._num_checkpoints):
        if self._num_checkpoints == 1:
          threshold = 0.99 - 0.8
        else:
          threshold = (0.99 - 0.8 / (self._num_checkpoints - 1) *
                       self._collected_checkpoints.get(rew_index, 0))
        if d > threshold:
          break
        reward[rew_index] += self._checkpoint_reward
        self._collected_checkpoints[rew_index] = (
            self._collected_checkpoints.get(rew_index, 0) + 1)
    return reward

class PassingRewardWrapper(gym.RewardWrapper):
  """A wrapper that will give dense rewards for passing.
  This will only work for single agent environments.
  """

  #TODO: pass completion

  def __init__(self, env):
      gym.RewardWrapper.__init__(self, env)

      self.short_passes_completed = 0
      self.aux_reward_amount = 0.1
      self.max_cumulative_award = 1.0
      self.running_award = 0.0
      self.target_action = 11 # short pass
      self.reward_returned = 0.0 #The actual reward amount used

      # _action_set = [idle, left, top_left, top, top_right, right,
      #                   bottom_right, bottom, bottom_left, long_pass,
      #                   high_pass, short_pass, shot, sprint, release_direction,
      #                   release_sprint, sliding, dribble, release_dribble]
      print ("Creating the passing reward wrapper")

  def reset(self):
      print ("running award was {}, resetting to 0".format(self.running_award))
      print ("{} passes were recorded, resetting to 0".format(self.short_passes_completed))
      print ("pass reward used was {}".format(self.reward_returned))
      self.running_award = 0.0
      self.short_passes_completed = 0
      self.reward_returned = 0.0
      return self.env.reset()

  def reward(self, reward):
      # print ("Processing passing reward")
      if len(reward) > 1:
          print ("Passing reward wrapper is only implemented/tested for single agent")
          exit()


      action_taken = self.env._get_actions()[0]
      #action 11 is short pass

      #if the action sampled is the action we are looking for:
      if action_taken == self.target_action:

          #keep track of short passes
          self.short_passes_completed += 1
          #keep track of awards we collect
          self.running_award += self.aux_reward_amount
          #don't go past some amount as we don't want to confuse the agent
          if self.running_award <= self.max_cumulative_award:
              short_pass_rew = self.aux_reward_amount
              self.reward_returned += self.aux_reward_amount
          else:
              short_pass_rew = 0.0

      else:
          short_pass_rew = 0.0


      #indexed at zero because we are dealing with a single agent
      #if it were multiple agaents we w ould have to iterate through a list of
      #multi-agent observations
      observation = self.env.unwrapped.observation()[0]
      if observation is None:
          print ("Observation is none")
          return reward


      # '''Processing observation dict below. We can ignore for now.'''
      # left_team_active = observation['left_team_active']
      # right_team_active = observation['right_team_active']
      # ball_owned_player = observation['ball_owned_player']
      # ball_owned_team = observation['ball_owned_team']
      # steps_left = observation['steps_left']
      # ball = observation['ball']
      # score = observation['score']
      # right_team_direction = observation['right_team_direction']
      # '''Just ignore the top for now'''


      reward[0] += short_pass_rew

      return reward

class ShotRewardWrapper(gym.RewardWrapper):

    def __init__(self, env):
        gym.RewardWrapper.__init__(self, env)
        print ("Creating shot reward wrapper")
        self.action_key = ['idle', 'left', 'top_left', 'top', 'top_right', 'right',
                          'bottom_right', 'bottom', 'bottom_left', 'long_pass',
                          'high_pass', 'short_pass', 'shot', 'sprint', 'release_direction',
                          'release_sprint', 'sliding', 'dribble', 'release_dribble']


    def reward(self, reward):
     # print ("Processing passing reward")
     if len(reward) > 1:
         print ("Passing reward wrapper is only implemented/tested for single agent")
         exit()

     action_taken = self.env._get_actions()[0]

     #indexed at zero because we are dealing with a single agent
     #if it were multiple agaents we w ould have to iterate through a list of
     #multi-agent observations
     observation = self.env.unwrapped.observation()[0]
     if observation is None:
         print ("Observation is none")
         return reward
     #check if the action is a shot. action is 12

     ball_owned = observation['ball_owned_team']
     ball_owned_player = observation['ball_owned_player'] #0-11


     print (ball_owned, ball_owned_player, self.action_key[action_taken])

     # if action_taken == 12:
     #     print (observation)
     #     print (action_taken)
     #     print ("Shot")

         #who shot?
         #award our agent for a shot on target
         #penalize our agent for an opposition shot on target


         # exit()


     return reward

class FrameStack(gym.Wrapper):
  """Stack k last observations."""

  def __init__(self, env, k):
    gym.Wrapper.__init__(self, env)
    self.obs = collections.deque([], maxlen=k)
    low = env.observation_space.low
    high = env.observation_space.high
    low = np.concatenate([low] * k, axis=-1)
    high = np.concatenate([high] * k, axis=-1)
    self.observation_space = gym.spaces.Box(
        low=low, high=high, dtype=env.observation_space.dtype)

  def reset(self):
    observation = self.env.reset()
    self.obs.extend([observation] * self.obs.maxlen)
    return self._get_observation()

  def get_state(self, to_pickle):
    to_pickle['FrameStack'] = self.obs
    return self.env.get_state(to_pickle)

  def set_state(self, state):
    from_pickle = self.env.set_state(state)
    self.obs = from_pickle['FrameStack']
    return from_pickle

  def step(self, action):
    observation, reward, done, info = self.env.step(action)
    self.obs.append(observation)
    return self._get_observation(), reward, done, info

  def _get_observation(self):
    return np.concatenate(list(self.obs), axis=-1)
