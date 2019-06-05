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


"""List of core actions / action sets supported by the gfootball environment."""

# ***** List of core actions *****
# Only add new ones, do not reorder so the numbering do not change.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from gfootball.env import controller_actions
import numpy
from six.moves import range


class SingleAction(object):

  def __init__(self, controller_action, sticky_duration=0):
    self.controller_action = controller_action
    self.sticky_duration = sticky_duration


class CoreAction(object):

  def __init__(self, action_id, name, action_sequence):
    self._action_id = action_id
    self.action_sequence = action_sequence
    self.name = name

  def __eq__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self.name == other.name

  def __ne__(self, other):
    return not self.__eq__(other)

  def __lt__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._action_id < other._action_id

  def __le__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._action_id <= other._action_id

  def __gt__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._action_id > other._action_id

  def __ge__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._action_id >= other._action_id

  def __hash__(self):
    return self._action_id

  def __repr__(self):
    return self.name


# Those actions are basically a one to one mapping with the actions supported
# by the controller but this doesn't have to be.
core_action_idle = CoreAction(0, "idle",
                              SingleAction(controller_actions.game_idle, 0))
core_action_left = CoreAction(1, "left",
                              SingleAction(controller_actions.game_left, 10000))
core_action_top_left = CoreAction(
    2, "top_left", SingleAction(controller_actions.game_top_left, 10000))
core_action_top = CoreAction(3, "top",
                             SingleAction(controller_actions.game_top, 10000))
core_action_top_right = CoreAction(
    4, "top_right", SingleAction(controller_actions.game_top_right, 10000))
core_action_right = CoreAction(
    5, "right", SingleAction(controller_actions.game_right, 10000))
core_action_bottom_right = CoreAction(
    6, "bottom_right", SingleAction(controller_actions.game_bottom_right,
                                    10000))
core_action_bottom = CoreAction(
    7, "bottom", SingleAction(controller_actions.game_bottom, 10000))
core_action_bottom_left = CoreAction(
    8, "bottom_left", SingleAction(controller_actions.game_bottom_left, 10000))
core_action_long_pass = CoreAction(
    9, "long_pass", SingleAction(controller_actions.game_long_pass, 0.1))
core_action_high_pass = CoreAction(
    10, "high_pass", SingleAction(controller_actions.game_high_pass, 0.1))
core_action_short_pass = CoreAction(
    11, "short_pass", SingleAction(controller_actions.game_short_pass, 0.1))
core_action_shot = CoreAction(12, "shot",
                              SingleAction(controller_actions.game_shot, 0.1))
core_action_keeper_rush = CoreAction(
    13, "keeper_rush", SingleAction(controller_actions.game_keeper_rush, 10000))
core_action_sliding = CoreAction(
    14, "sliding", SingleAction(controller_actions.game_sliding, 0.1))
core_action_pressure = CoreAction(
    15, "pressure", SingleAction(controller_actions.game_pressure, 10000))
core_action_team_pressure = CoreAction(
    16, "team_pressure",
    SingleAction(controller_actions.game_team_pressure, 10000))
core_action_switch = CoreAction(
    17, "switch", SingleAction(controller_actions.game_switch, 0.1))
core_action_sprint = CoreAction(
    18, "sprint", SingleAction(controller_actions.game_sprint, 10000))
core_action_dribble = CoreAction(
    19, "dribble", SingleAction(controller_actions.game_dribble, 10000))
core_action_release_direction = CoreAction(
    20, "release_direction",
    SingleAction(controller_actions.game_release_direction))
core_action_release_long_pass = CoreAction(
    21, "release_long_pass",
    SingleAction(controller_actions.game_release_long_pass))
core_action_release_high_pass = CoreAction(
    22, "release_high_pass",
    SingleAction(controller_actions.game_release_high_pass))
core_action_release_short_pass = CoreAction(
    23, "release_short_pass",
    SingleAction(controller_actions.game_release_short_pass))
core_action_release_shot = CoreAction(
    24, "release_shot", SingleAction(controller_actions.game_release_shot))
core_action_release_keeper_rush = CoreAction(
    25, "release_keeper_rush",
    SingleAction(controller_actions.game_release_keeper_rush))
core_action_release_sliding = CoreAction(
    26, "release_sliding",
    SingleAction(controller_actions.game_release_sliding))
core_action_release_pressure = CoreAction(
    27, "release_pressure",
    SingleAction(controller_actions.game_release_pressure))
core_action_release_team_pressure = CoreAction(
    28, "release_team_pressure",
    SingleAction(controller_actions.game_release_team_pressure))
core_action_release_switch = CoreAction(
    29, "release_switch", SingleAction(controller_actions.game_release_switch))
core_action_release_sprint = CoreAction(
    30, "release_sprint", SingleAction(controller_actions.game_release_sprint))
core_action_release_dribble = CoreAction(
    31, "release_dribble",
    SingleAction(controller_actions.game_release_dribble))

# ***** Define some action sets *****

# Special action set that includes all the core actions in the same order.
full_action_set = [
    core_action_idle, core_action_left, core_action_top_left, core_action_top,
    core_action_top_right, core_action_right, core_action_bottom_right,
    core_action_bottom, core_action_bottom_left, core_action_long_pass,
    core_action_high_pass, core_action_short_pass, core_action_shot,
    core_action_keeper_rush, core_action_sliding, core_action_pressure,
    core_action_team_pressure, core_action_switch, core_action_sprint,
    core_action_dribble, core_action_release_direction,
    core_action_release_long_pass, core_action_release_high_pass,
    core_action_release_short_pass, core_action_release_shot,
    core_action_release_keeper_rush, core_action_release_sliding,
    core_action_release_pressure, core_action_release_team_pressure,
    core_action_release_switch, core_action_release_sprint,
    core_action_release_dribble
]

action_set_dict = {
    # Needed by play_game script.
    "full":
        full_action_set,
    "default": [
        core_action_idle, core_action_left, core_action_top_left,
        core_action_top, core_action_top_right, core_action_right,
        core_action_bottom_right, core_action_bottom, core_action_bottom_left,
        core_action_long_pass, core_action_high_pass, core_action_short_pass,
        core_action_shot,
        core_action_sprint,
        core_action_release_direction,
        core_action_release_sprint
    ],
}

disable_action_mapping = {
    core_action_left: core_action_release_direction,
    core_action_top_left: core_action_release_direction,
    core_action_top: core_action_release_direction,
    core_action_top_right: core_action_release_direction,
    core_action_right: core_action_release_direction,
    core_action_bottom_right: core_action_release_direction,
    core_action_bottom: core_action_release_direction,
    core_action_bottom_left: core_action_release_direction,
    core_action_high_pass: core_action_release_high_pass,
    core_action_long_pass: core_action_release_long_pass,
    core_action_short_pass: core_action_release_short_pass,
    core_action_shot: core_action_release_shot,
    core_action_keeper_rush: core_action_release_keeper_rush,
    core_action_sliding: core_action_release_sliding,
    core_action_pressure: core_action_release_pressure,
    core_action_team_pressure: core_action_release_team_pressure,
    core_action_switch: core_action_release_switch,
    core_action_sprint: core_action_release_sprint,
    core_action_dribble: core_action_release_dribble
}


# Returns action set specified by the config.
def get_action_set(config):
  action_set_name = config["action_set"]
  return action_set_dict[action_set_name]


# Converts different action representation to an action from a given action set.
def action_from_action_set(action_set, action):
  if (hasattr(action, "__dict__") and action_set and
      set(action.__dict__) == set(action_set[0].__dict__)):
    for x in range(len(action_set)):
      if action_set[x] == action:
        return x

  if (isinstance(action, numpy.int32) or isinstance(action, numpy.int64) or
      isinstance(action, int)):
    # The action can be given as a numpy.int32 which cannot be
    # serialized. First convert it to a proper python integer.
    action = int(action)
    assert action < len(action_set), "Action outside of action set"
    return action

  assert False, "Action {} not found in action set".format(action)


def disable_action(action):
  assert set(action.__dict__) == set(core_action_left.__dict__)
  return disable_action_mapping[action]
