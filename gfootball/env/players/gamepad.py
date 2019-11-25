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


"""Player with actions coming from gamepad."""

from absl import logging
import pygame

from gfootball.env import controller_base
from gfootball.env import football_action_set
from gfootball.env import event_queue

BUTTON_TO_ACTIONS = {
    0: [football_action_set.action_short_pass,
        football_action_set.action_pressure],
    1: [football_action_set.action_shot,
        football_action_set.action_team_pressure],
    2: [football_action_set.action_high_pass,
        football_action_set.action_sliding],
    3: [football_action_set.action_long_pass,
        football_action_set.action_keeper_rush],
    4: [football_action_set.action_switch],
    5: [football_action_set.action_dribble],
}


class Player(controller_base.Controller):
  """Player with actions coming from gamepad."""

  def __init__(self, player_config, env_config):
    controller_base.Controller.__init__(self, player_config, env_config)
    self._can_play_right = True
    pygame.init()
    self._index = player_config['player_gamepad']
    event_queue.add_controller('gamepad', self._index)
    pygame.joystick.init()
    if pygame.joystick.get_count() < self._index:
      logging.error("You need %d physical controller(s) connected" % self._index)
      exit(1)
    self._joystick = pygame.joystick.Joystick(self._index)
    self._joystick.init()

  def take_action(self, observations):
    assert len(observations) == 1, 'Gamepad does not support multiple player control'
    x_axis = self._joystick.get_axis(0)
    y_axis = self._joystick.get_axis(1)
    left = x_axis < -0.5
    right = x_axis > 0.5
    top = y_axis < -0.5
    bottom = y_axis > 0.5
    active_buttons = {}
    for event in event_queue.get('gamepad', self._index):
      if event.type == pygame.JOYBUTTONDOWN:
        actions = BUTTON_TO_ACTIONS.get(event.button, [])
        for a in actions:
          active_buttons[a] = 1
      if (event.type == pygame.JOYAXISMOTION and event.axis == 5 and
          event.value > 0):
        active_buttons[football_action_set.action_sprint] = 1

    for button, actions in BUTTON_TO_ACTIONS.items():
      if self._joystick.get_button(button):
        for a in actions:
          active_buttons[a] = 1
    if self._joystick.get_axis(5) > 0:
      active_buttons[football_action_set.action_sprint] = 1
    return self.get_env_action(left, right, top, bottom, active_buttons)
