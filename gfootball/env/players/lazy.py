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


"""Lazy player not moving at all."""


from gfootball.env import football_action_set
from gfootball.env import player_base

class Player(player_base.PlayerBase):
  """Lazy player not moving at all."""

  def __init__(self, player_config, env_config):
    player_base.PlayerBase.__init__(self, player_config)

  def take_action(self, observations):
    return [football_action_set.action_idle] * len(observations)
