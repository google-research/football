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


"""Base controller class."""

from gfootball.env import football_action_set
from gfootball.env import player_base


class Controller(player_base.PlayerBase):
  """Base controller class."""

  def __init__(self, player_config, env_config):
    player_base.PlayerBase.__init__(self, player_config)
    self._active_actions = {}
    self._env_config = env_config
    self._last_action = football_action_set.action_idle
    self._last_direction = football_action_set.action_idle
    self._current_direction = football_action_set.action_idle

  def _check_action(self, action, active_actions):
    """Compare (and update) controller's state with the set of active actions.

    Args:
      action: Action to check
      active_actions: Set of all active actions
    """
    assert isinstance(action, football_action_set.CoreAction)
    if not action.is_in_actionset(self._env_config):
      return
    state = active_actions.get(action, 0)
    if (self._last_action == football_action_set.action_idle and
        self._active_actions.get(action, 0) != state):
      self._active_actions[action] = state
      if state:
        self._last_action = action
      else:
        self._last_action = football_action_set.disable_action(action)
        assert self._last_action

  def _check_direction(self, action, state):
    """Compare (and update) controller's direction with the current direction.

    Args:
      action: Action to check
      state: Current state of the action being checked
    """
    assert isinstance(action, football_action_set.CoreAction)
    if not action.is_in_actionset(self._env_config):
      return
    if self._current_direction != football_action_set.action_idle:
      return
    if state:
      self._current_direction = action

  def get_env_action(self, left, right, top, bottom, active_actions):
    """For a given controller's state generate next environment action.

    Args:
      action: Action to check
      state: Current state of the action being checked
    """
    self._current_direction = football_action_set.action_idle
    self._check_direction(football_action_set.action_top_left, top and
                          left)
    self._check_direction(football_action_set.action_top_right, top and
                          right)
    self._check_direction(football_action_set.action_bottom_left,
                          bottom and left)
    self._check_direction(football_action_set.action_bottom_right,
                          bottom and right)
    if self._current_direction == football_action_set.action_idle:
      self._check_direction(football_action_set.action_right, right)
      self._check_direction(football_action_set.action_left, left)
      self._check_direction(football_action_set.action_top, top)
      self._check_direction(football_action_set.action_bottom, bottom)
    if self._current_direction != self._last_direction:
      self._last_direction = self._current_direction
      if self._current_direction == football_action_set.action_idle:
        return football_action_set.action_release_direction
      else:
        return self._current_direction
    self._last_action = football_action_set.action_idle
    self._check_action(football_action_set.action_long_pass,
                       active_actions)
    self._check_action(football_action_set.action_high_pass,
                       active_actions)
    self._check_action(football_action_set.action_short_pass,
                       active_actions)
    self._check_action(football_action_set.action_shot, active_actions)
    self._check_action(football_action_set.action_keeper_rush,
                       active_actions)
    self._check_action(football_action_set.action_sliding, active_actions)
    self._check_action(football_action_set.action_pressure,
                       active_actions)
    self._check_action(football_action_set.action_team_pressure,
                       active_actions)
    self._check_action(football_action_set.action_sprint, active_actions)
    self._check_action(football_action_set.action_dribble, active_actions)
    return self._last_action
