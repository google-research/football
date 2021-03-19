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


"""Football action set tests."""

from absl.testing import absltest
from gfootball.env import football_action_set
import numpy as np

named_action_from_action_set = football_action_set.named_action_from_action_set


class FootballActionSetTest(absltest.TestCase):

  def test_action_from_basic_action_set(self):
    action_set = football_action_set.get_action_set({'action_set': 'default'})
    self.assertEqual(
        named_action_from_action_set(action_set, 1),
        football_action_set.action_left)
    self.assertEqual(
        named_action_from_action_set(action_set, 12),
        football_action_set.action_shot)

    self.assertEqual(named_action_from_action_set(action_set, np.int32(1)),
                     football_action_set.action_left)
    self.assertRaises(Exception, named_action_from_action_set, action_set,
                      np.int32(100))

    self.assertEqual(
        named_action_from_action_set(action_set,
                                     football_action_set.action_left),
        football_action_set.action_left)
    self.assertRaises(Exception, named_action_from_action_set, action_set, 100)

  def test_action_set_full(self):
    self.assertEqual(football_action_set.full_action_set[0],
                     football_action_set.action_idle)

  def test_disable_action(self):
    self.assertEqual(
        football_action_set.disable_action(
            football_action_set.action_left),
        football_action_set.action_release_direction)
    self.assertEqual(
        football_action_set.disable_action(
            football_action_set.action_release_direction),
        football_action_set.action_release_direction)

  def test_sticky_actions_have_release(self):
    for i in football_action_set.action_set_dict:
      action_set = football_action_set.action_set_dict[i]
      for action in action_set:
        if action._sticky:
          reverse = football_action_set.disable_action(action)
          self.assertTrue(
              reverse in action_set,
              'Action {} has no release action in action set {}'.format(
                  action._name, i))


if __name__ == '__main__':
  absltest.main()
