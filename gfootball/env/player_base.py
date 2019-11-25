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


"""Base player class."""


class PlayerBase(object):
  """Base player class."""

  def __init__(self, player_config=None):
    self._num_left_controlled_players = 1
    self._num_right_controlled_players = 0
    self._can_play_right = False
    if player_config:
      self._num_left_controlled_players = int(player_config['left_players'])
      self._num_right_controlled_players = int(player_config['right_players'])

  def num_controlled_left_players(self):
    return self._num_left_controlled_players

  def num_controlled_right_players(self):
    return self._num_right_controlled_players

  def num_controlled_players(self):
    return (self._num_left_controlled_players +
            self._num_right_controlled_players)

  def reset(self):
    pass

  def can_play_right(self):
    return self._can_play_right
