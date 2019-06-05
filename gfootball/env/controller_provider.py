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


"""A wrapper that generates release action automatically for sticky actions."""

import timeit
from gfootball.env import controller_actions
import numpy as np


def get_controller(backend=None):
  return StickyWrapper(backend)

class Action(object):

  def __init__(self, action, reverse, expose_in_observations,
               directional=False):
    self._action = action
    self._reverse = reverse
    self._disable_timestamp = None
    self._directional = directional
    self._expose_in_observations = expose_in_observations


class StickyWrapper(object):

  def __init__(self, controller):
    self._controller = controller
    self._actions = {}
    self._activable_actions = []
    self.add_action(Action(controller_actions.game_idle, None, False))
    self.add_action(
        Action(
            controller_actions.game_left,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_top_left,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_top,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_top_right,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_right,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_bottom_right,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_bottom,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_bottom_left,
            controller_actions.game_release_direction,
            True,
            directional=True))
    self.add_action(
        Action(
            controller_actions.game_release_direction,
            None,
            False,
            directional=True))
    self.add_action(
        Action(controller_actions.game_long_pass,
               controller_actions.game_release_long_pass, False))
    self.add_action(
        Action(controller_actions.game_high_pass,
               controller_actions.game_release_high_pass, False))
    self.add_action(
        Action(controller_actions.game_short_pass,
               controller_actions.game_release_short_pass, False))
    self.add_action(
        Action(controller_actions.game_shot,
               controller_actions.game_release_shot, False))
    self.add_action(
        Action(controller_actions.game_keeper_rush,
               controller_actions.game_release_keeper_rush, True))
    self.add_action(
        Action(controller_actions.game_sliding,
               controller_actions.game_release_sliding, False))
    self.add_action(
        Action(controller_actions.game_pressure,
               controller_actions.game_release_pressure, True))
    self.add_action(
        Action(controller_actions.game_team_pressure,
               controller_actions.game_release_team_pressure, True))
    self.add_action(
        Action(controller_actions.game_switch,
               controller_actions.game_release_switch, False))
    self.add_action(
        Action(controller_actions.game_sprint,
               controller_actions.game_release_sprint, True))
    self.add_action(
        Action(controller_actions.game_dribble,
               controller_actions.game_release_dribble, True))
    self.add_action(
        Action(controller_actions.game_release_long_pass,
               controller_actions.game_long_pass, False))
    self.add_action(
        Action(controller_actions.game_release_high_pass,
               controller_actions.game_high_pass, False))
    self.add_action(
        Action(controller_actions.game_release_short_pass,
               controller_actions.game_short_pass, False))
    self.add_action(
        Action(controller_actions.game_release_shot,
               controller_actions.game_shot, False))
    self.add_action(
        Action(controller_actions.game_release_keeper_rush,
               controller_actions.game_keeper_rush, False))
    self.add_action(
        Action(controller_actions.game_release_sliding,
               controller_actions.game_sliding, False))
    self.add_action(
        Action(controller_actions.game_release_pressure,
               controller_actions.game_pressure, False))
    self.add_action(
        Action(controller_actions.game_release_team_pressure,
               controller_actions.game_team_pressure, False))
    self.add_action(
        Action(controller_actions.game_release_switch,
               controller_actions.game_switch, False))
    self.add_action(
        Action(controller_actions.game_release_sprint,
               controller_actions.game_sprint, False))
    self.add_action(
        Action(controller_actions.game_release_dribble,
               controller_actions.game_dribble, False))

  def add_action(self, action):
    self._actions[action._action] = action
    if action._expose_in_observations:
      self._activable_actions.append(action._action)

  def post_initialize(self):
    self._controller.post_initialize()

  def active_actions(self):
    result = []
    for a in self._activable_actions:
      result.append(1 if self._actions[a]._disable_timestamp else 0)
    return np.uint8(result)

  def perform_controller_action(self, action):
    """Directly pass an action to the controller.

    Args:
      action: An action from football_backend.action.
    """
    self._controller.perform_action(action)

  def perform_action(self, action, timestamp=None):
    """Performs a given CoreAction and updates sticky action states.

    Disables sticky actions active for too long. It also disables actions
    which are disjoint with the action being performed.

    Args:
      action: a CoreAction
      timestamp: Timestamp at which controller action takes place. Is used for
        determining sticky actions state.
    """
    assert hasattr(action, 'action_sequence')
    if timestamp is None:
      timestamp = timeit.default_timer()

    backend_actions = action.action_sequence
    if not isinstance(backend_actions, list):
      backend_actions = [backend_actions]

    # Disable sticky actions enabled for too long.
    for code in self._actions:
      a = self._actions[code]
      if a._disable_timestamp and a._disable_timestamp < timestamp and a._reverse:
        rev = self._actions[a._reverse]
        self._controller.perform_action(rev._action.backend_id)
        a._disable_timestamp = None

    for backend_action in backend_actions:
      assert hasattr(backend_action, 'controller_action')
      assert hasattr(backend_action, 'sticky_duration')
      action_data = self._actions[backend_action.controller_action]
      if backend_action.controller_action == controller_actions.game_idle:
        continue

      if action_data._directional:
        # Direction action disables all other direction actions.
        for code in self._actions:
          a = self._actions[code]
          if a._directional:
            a._disable_timestamp = None
      elif action_data._reverse:
        self._actions[action_data._reverse]._disable_timestamp = None

      self._controller.perform_action(action_data._action.backend_id)
      if (not action_data._disable_timestamp and
          backend_action.sticky_duration > 0):
        action_data._disable_timestamp = timestamp + backend_action.sticky_duration
