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

  def __init__(self, player_config):
    self._num_controlled_players = int(player_config.get('players', 1))

  def num_controlled_players(self):
    return self._num_controlled_players

  def reset(self):
    pass
