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
from gfootball.env import football_action_set
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

  def __init__(self, env, dump_frequency, render=False):
    gym.Wrapper.__init__(self, env)
    self._dump_frequency = dump_frequency
    self._render = render
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
      if self._render:
        self.env.render()
    else:
      self.env._config.update({'write_video': False,
                               'dump_full_episodes': False,
                               'dump_scores': False})
      if self._render:
        self.env.disable_render()
    self._current_episode_number += 1
    return self.env.reset()


class Simple115StateWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to 115-features state."""

  def __init__(self, env, fixed_positions=False):
    """Initializes the wrapper.

    Args:
      env: an envorinment to wrap
      fixed_positions: whether to fix observation indexes corresponding to teams
    Note: simple115v2 enables fixed_positions option.
    """
    gym.ObservationWrapper.__init__(self, env)
    action_shape = np.shape(self.env.action_space)
    shape = (action_shape[0] if len(action_shape) else 1, 115)
    self.observation_space = gym.spaces.Box(
        low=-np.inf, high=np.inf, shape=shape, dtype=np.float32)
    self._fixed_positions = fixed_positions

  def observation(self, observation):
    """Converts an observation into simple115 (or simple115v2) format."""
    return Simple115StateWrapper.convert_observation(observation, self._fixed_positions)

  @staticmethod
  def convert_observation(observation, fixed_positions):
    """Converts an observation into simple115 (or simple115v2) format.

    Args:
      observation: observation that the environment returns
      fixed_positions: Players and positions are always occupying 88 fields
                       (even if the game is played 1v1).
                       If True, the position of the player will be the same - no
                       matter how many players are on the field:
                       (so first 11 pairs will belong to the first team, even
                       if it has less players).
                       If False, then the position of players from team2
                       will depend on number of players in team1).

    Returns:
      (N, 115) shaped representation, where N stands for the number of players
      being controlled.
    """

    def do_flatten(obj):
      """Run flatten on either python list or numpy array."""
      if type(obj) == list:
        return np.array(obj).flatten()
      return obj.flatten()

    final_obs = []
    for obs in observation:
      o = []
      if fixed_positions:
        for i, name in enumerate(['left_team', 'left_team_direction',
                                  'right_team', 'right_team_direction']):
          o.extend(do_flatten(obs[name]))
          # If there were less than 11vs11 players we backfill missing values
          # with -1.
          if len(o) < (i + 1) * 22:
            o.extend([-1] * ((i + 1) * 22 - len(o)))
      else:
        o.extend(do_flatten(obs['left_team']))
        o.extend(do_flatten(obs['left_team_direction']))
        o.extend(do_flatten(obs['right_team']))
        o.extend(do_flatten(obs['right_team_direction']))

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
    action_shape = np.shape(self.env.action_space)
    self.observation_space = gym.spaces.Box(
        low=0,
        high=255,
        shape=(action_shape[0] if len(action_shape) else 1,
               channel_dimensions[1], channel_dimensions[0],
               1 if grayscale else 3),
        dtype=np.uint8)

  def observation(self, obs):
    o = []
    for observation in obs:
      assert 'frame' in observation, ("Missing 'frame' in observations. Pixel "
                                      "representation requires rendering and is"
                                      " supported only for players on the left "
                                      "team.")
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
  """A wrapper that convers observations into a minimap format."""

  def __init__(self, env,
               channel_dimensions=(observation_preprocessing.SMM_WIDTH,
                                   observation_preprocessing.SMM_HEIGHT)):
    gym.ObservationWrapper.__init__(self, env)
    self._channel_dimensions = channel_dimensions
    action_shape = np.shape(self.env.action_space)
    shape = (action_shape[0] if len(action_shape) else 1, channel_dimensions[1],
             channel_dimensions[0],
             len(
                 observation_preprocessing.get_smm_layers(
                     self.env.unwrapped._config)))
    self.observation_space = gym.spaces.Box(
        low=0, high=255, shape=shape, dtype=np.uint8)

  def observation(self, obs):
    return observation_preprocessing.generate_smm(
        obs, channel_dimensions=self._channel_dimensions,
        config=self.env.unwrapped._config)


class SingleAgentObservationWrapper(gym.ObservationWrapper):
  """A wrapper that returns an observation only for the first agent."""

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


class MultiAgentToSingleAgent(gym.Wrapper):
  """Converts raw multi-agent observations to single-agent observation.

  It returns observations of the designated player on the team, so that
  using this wrapper in multi-agent setup is equivalent to controlling a single
  player. This wrapper is used for scenarios with control_all_players set when
  agent controls only one player on the team. It can also be used
  in a standalone manner:

  env = gfootball.env.create_environment(env_name='tests/multiagent_wrapper',
      number_of_left_players_agent_controls=11)
  observations = env.reset()
  single_observation = MultiAgentToSingleAgent.get_observation(observations)
  single_action = agent.run(single_observation)
  actions = MultiAgentToSingleAgent.get_action(single_action, observations)
  env.step(actions)
  """

  def __init__(self, env, left_players, right_players):
    assert left_players < 2
    assert right_players < 2
    players = left_players + right_players
    gym.Wrapper.__init__(self, env)
    self._observation = None
    if players > 1:
      self.action_space = gym.spaces.MultiDiscrete([env._num_actions] * players)
    else:
      self.action_space = gym.spaces.Discrete(env._num_actions)

  def reset(self):
    self._observation = self.env.reset()
    return self._get_observation()

  def step(self, action):
    assert self._observation, 'Reset must be called before step'
    action = MultiAgentToSingleAgent.get_action(action, self._observation)
    self._observation, reward, done, info = self.env.step(action)
    return self._get_observation(), reward, done, info

  def _get_observation(self):
    return MultiAgentToSingleAgent.get_observation(self._observation)

  @staticmethod
  def get_observation(observation):
    assert 'designated' in observation[
        0], 'Only raw observations can be converted'
    result = []
    for obs in observation:
      if obs['designated'] == obs['active']:
        result.append(obs)
    return result

  @staticmethod
  def get_action(actions, orginal_observation):
    assert 'designated' in orginal_observation[
        0], 'Only raw observations can be converted'
    result = [football_action_set.action_builtin_ai] * len(orginal_observation)
    action_idx = 0
    for idx, obs in enumerate(orginal_observation):
      if obs['designated'] == obs['active']:
        assert action_idx < len(actions)
        result[idx] = actions[action_idx]
        action_idx += 1
    return result
