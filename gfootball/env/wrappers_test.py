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


"""Tests for gym wrappers."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import gym
import numpy as np
from absl.testing import absltest


class SingleAgentWrapperTest(absltest.TestCase):

  def test_consistent_observation_and_action(self):
    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-SMM-v0',
        number_of_left_players_agent_controls=1)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(72, 96, 4), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((72, 96, 4), obs.shape)
    env.close()

    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-SMM-v0',
        number_of_left_players_agent_controls=2)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(2, 72, 96, 4), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((2, 72, 96, 4), obs.shape)
    env.close()

  def test_consistent_observation_and_action_stacked(self):
    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-SMM-v0',
        number_of_left_players_agent_controls=1,
        stacked=True)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(72, 96, 16), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((72, 96, 16), obs.shape)
    env.close()

    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-SMM-v0',
        number_of_left_players_agent_controls=2,
        stacked=True)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(2, 72, 96, 16), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((2, 72, 96, 16), obs.shape)
    env.close()

  def test_consistent_observation_and_action_pixels(self):
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Forge doesn't support rendering.
      return
    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-Pixels-v0',
        number_of_left_players_agent_controls=1,
        render=True)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(72, 96, 3), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((72, 96, 3), obs.shape)
    env.close()

    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-Pixels-v0',
        number_of_left_players_agent_controls=2,
        render=True)

    self.assertEqual(
        gym.spaces.Box(low=0, high=255, shape=(2, 72, 96, 3), dtype=np.uint8),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((2, 72, 96, 3), obs.shape)
    env.close()

  def test_consistent_observation_and_action_floats(self):
    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-simple115-v0',
        number_of_left_players_agent_controls=1)

    self.assertEqual(
        gym.spaces.Box(
            low=-np.inf, high=np.inf, shape=(115,), dtype=np.float32),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((115,), obs.shape)
    env.close()

    env = gym.make(
        'gfootball:GFootball-11_vs_11_easy_stochastic-simple115-v0',
        number_of_left_players_agent_controls=2)

    self.assertEqual(
        gym.spaces.Box(
            low=-np.inf, high=np.inf, shape=(2, 115), dtype=np.float32),
        env.observation_space)

    env.reset()
    obs, _, _, _ = env.step(env.action_space.sample())
    self.assertEqual((2, 115), obs.shape)
    env.close()

if __name__ == '__main__':
  absltest.main()
