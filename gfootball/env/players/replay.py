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

"""Player with actions coming from specific game replay."""

from gfootball.env import player_base
from gfootball.env import script_helpers


class Player(player_base.PlayerBase):
  """Player with actions coming from specific game replay."""

  def __init__(self, player_config, env_config):
    player_base.PlayerBase.__init__(self, player_config)
    self._can_play_right = True
    self._replay = script_helpers.ScriptHelpers().load_dump(player_config['path'])
    self._step = 0
    self._player = player_config['index']

  def take_action(self, observations):
    if self._step == len(self._replay):
      print("Replay finished.")
      exit(0)
    actions = self._replay[self._step]['debug']['action'][
        self._player:self.num_controlled_players() + self._player]
    self._step += 1
    return actions
