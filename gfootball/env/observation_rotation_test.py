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


"""Test for observation_rotation.py."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl.testing import absltest

from gfootball.env import config
from gfootball.env import football_action_set
from gfootball.env import observation_rotation
import numpy as np


class ObservationRotationTest(absltest.TestCase):

  def testObservationFlipping(self):
    cfg = config.Config()
    num_players = 11
    observation = {}
    observation['left_team'] = np.random.rand(num_players * 2) - 0.5
    observation['left_team_roles'] = np.random.rand(num_players)
    observation['left_team_direction'] = np.random.rand(num_players * 2) - 0.5
    observation['left_team_tired_factor'] = np.random.rand(num_players)
    observation['left_team_yellow_card'] = np.random.rand(num_players)
    observation['left_team_active'] = [3]
    observation['left_team_designated_player'] = 3
    observation['right_team'] = np.random.rand(num_players * 2) - 0.5
    observation['right_team_roles'] = np.random.rand(num_players)
    observation['right_team_direction'] = np.random.rand(num_players * 2) - 0.5
    observation['right_team_tired_factor'] = np.random.rand(num_players)
    observation['right_team_yellow_card'] = np.random.rand(num_players)
    observation['right_team_active'] = [0]
    observation['right_team_designated_player'] = 0
    observation['ball'] = np.array([1, -1, 0])
    observation['ball_direction'] = np.random.rand(3) - 0.5
    observation['ball_rotation'] = np.random.rand(3) - 0.5
    observation['ball_owned_team'] = 0
    observation['ball_owned_player'] = 7
    observation['left_agent_controlled_player'] = [4]
    observation['right_agent_controlled_player'] = [6]
    observation['game_mode'] = 123
    observation['left_agent_sticky_actions'] = [
        [np.random.rand(2) for _ in range(10)]]
    observation['right_agent_sticky_actions'] = [
        [np.random.rand(2) for _ in range(10)]]
    observation['score'] = [3, 5]
    observation['steps_left'] = 45
    # Flipping twice the observation is the identity.
    flipped_observation = observation_rotation.flip_observation(
        observation, cfg)
    original_observation = observation_rotation.flip_observation(
        flipped_observation, cfg)
    self.assertEqual(str(tuple(sorted(original_observation.items()))),
                     str(tuple(sorted(observation.items()))))

  def testActionFlipping(self):
    cfg = config.Config()
    for action in football_action_set.full_action_set:
      # Flipping twice should give the same action.
      action_id = observation_rotation.flip_single_action(
          observation_rotation.flip_single_action(action, cfg), cfg)
      self.assertEqual(action, action_id)


if __name__ == '__main__':
  absltest.main()
