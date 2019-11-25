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


"""GFootball environment using OpenAI Gym test."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest

import gym
from absl.testing import parameterized


class GymTest(parameterized.TestCase):

  @parameterized.parameters(('scoring'), ('scoring,checkpoints'))
  def test_environment(self, rewards):
    # Tests it is possible to create and run an environment twice.
    for _ in range(2):
      env = gym.make('gfootball:GFootball-11_vs_11_easy_stochastic-SMM-v0',
                     stacked=True, rewards=rewards)
      env.reset()
      for _ in range(10):
        _, _, done, _ = env.step(env.action_space.sample())
        if done:
          env.reset()
      env.close()


if __name__ == '__main__':
  unittest.main()
