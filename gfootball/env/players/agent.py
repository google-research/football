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


"""Agent player controlled by the training policy and using step/reset API."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import copy

from gfootball.env import player_base


class Player(player_base.PlayerBase):

  def __init__(self, player_config, env_config):
    player_base.PlayerBase.__init__(self, player_config)
    assert player_config['player_agent'] == 0, 'Only one \'agent\' player allowed'
    self._action = None

  def set_action(self, action):
    self._action = action

  def take_action(self, observations):
    return copy.deepcopy(self._action)
