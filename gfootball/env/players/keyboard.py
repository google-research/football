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


"""Player with actions coming from the keyboard."""

import pygame

from gfootball.env import controller_base
from gfootball.env import football_action_set
from gfootball.env import event_queue


KEY_TO_ACTIONS = {
    pygame.K_s: [football_action_set.action_short_pass,
                 football_action_set.action_pressure],
    pygame.K_d: [football_action_set.action_shot,
                 football_action_set.action_team_pressure],
    pygame.K_a: [football_action_set.action_high_pass,
                 football_action_set.action_sliding],
    pygame.K_w: [football_action_set.action_long_pass,
                 football_action_set.action_keeper_rush],
    pygame.K_q: [football_action_set.action_switch],
    pygame.K_c: [football_action_set.action_dribble],
    pygame.K_e: [football_action_set.action_sprint],
}


class Player(controller_base.Controller):
  """Player with actions coming from the keyboard."""

  def __init__(self, player_config, env_config):
    controller_base.Controller.__init__(self, player_config, env_config)
    self._can_play_right = True
    self._init_done = False
    pygame.init()
    event_queue.add_controller('keyboard')

  def take_action(self, observations):
    assert len(observations) == 1, 'Keyboard does not support multiple player control'
    if not self._init_done:
      self._init_done = True
      pygame.display.set_mode((1, 1), pygame.NOFRAME)
    active_buttons = {}
    for event in event_queue.get('keyboard'):
      if event.type == pygame.KEYDOWN:
        actions = KEY_TO_ACTIONS.get(event.key, [])
        for a in actions:
          active_buttons[a] = 1
    keys = pygame.key.get_pressed()
    left = keys[pygame.K_LEFT]
    right = keys[pygame.K_RIGHT]
    top = keys[pygame.K_UP]
    bottom = keys[pygame.K_DOWN]
    for key, actions in KEY_TO_ACTIONS.items():
      if keys[key]:
        for a in actions:
          active_buttons[a] = 1
    return self.get_env_action(left, right, top, bottom, active_buttons)
