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

from gfootball_engine import e_BackendAction
import numpy
from six.moves import range


class CoreAction(object):

  def __init__(self, backend_action, name, sticky=False, directional=False):
    self._backend_action = backend_action
    self._name = name
    self._sticky = sticky
    self._directional = directional

  def is_in_actionset(self, config):
    return self in get_action_set(config)

  def __eq__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._name == other._name

  def __ne__(self, other):
    return not self.__eq__(other)

  def __lt__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._backend_action < other._backend_action

  def __le__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._backend_action <= other._backend_action

  def __gt__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._backend_action > other._backend_action

  def __ge__(self, other):
    assert set(other.__dict__) == set(self.__dict__)
    return self._backend_action >= other._backend_action

  def __hash__(self):
    return self._backend_action

  def __repr__(self):
    return self._name


action_idle = CoreAction(e_BackendAction.idle, "idle")
action_builtin_ai = CoreAction(e_BackendAction.builtin_ai, "builtin_ai")
action_left = CoreAction(
    e_BackendAction.left, "left", sticky=True, directional=True)
action_top_left = CoreAction(
    e_BackendAction.top_left, "top_left", sticky=True, directional=True)
action_top = CoreAction(
    e_BackendAction.top, "top", sticky=True, directional=True)
action_top_right = CoreAction(
    e_BackendAction.top_right, "top_right", sticky=True, directional=True)
action_right = CoreAction(
    e_BackendAction.right, "right", sticky=True, directional=True)
action_bottom_right = CoreAction(
    e_BackendAction.bottom_right, "bottom_right", sticky=True, directional=True)
action_bottom = CoreAction(
    e_BackendAction.bottom, "bottom", sticky=True, directional=True)
action_bottom_left = CoreAction(
    e_BackendAction.bottom_left, "bottom_left", sticky=True, directional=True)
action_long_pass = CoreAction(e_BackendAction.long_pass, "long_pass")
action_high_pass = CoreAction(e_BackendAction.high_pass, "high_pass")
action_short_pass = CoreAction(e_BackendAction.short_pass, "short_pass")
action_shot = CoreAction(e_BackendAction.shot, "shot")
action_keeper_rush = CoreAction(
    e_BackendAction.keeper_rush, "keeper_rush", sticky=True)
action_sliding = CoreAction(e_BackendAction.sliding, "sliding")
action_pressure = CoreAction(
    e_BackendAction.pressure, "pressure", sticky=True)
action_team_pressure = CoreAction(
    e_BackendAction.team_pressure, "team_pressure", sticky=True)
action_switch = CoreAction(e_BackendAction.switch, "switch")
action_sprint = CoreAction(e_BackendAction.sprint, "sprint", sticky=True)
action_dribble = CoreAction(
    e_BackendAction.dribble, "dribble", sticky=True)
action_release_direction = CoreAction(
    e_BackendAction.release_direction, "release_direction", directional=True)
action_release_long_pass = CoreAction(e_BackendAction.release_long_pass,
                                      "release_long_pass")
action_release_high_pass = CoreAction(e_BackendAction.release_high_pass,
                                      "release_high_pass")
action_release_short_pass = CoreAction(e_BackendAction.release_short_pass,
                                       "release_short_pass")
action_release_shot = CoreAction(e_BackendAction.release_shot, "release_shot")
action_release_keeper_rush = CoreAction(e_BackendAction.release_keeper_rush,
                                        "release_keeper_rush")
action_release_sliding = CoreAction(e_BackendAction.release_sliding,
                                    "release_sliding")
action_release_pressure = CoreAction(e_BackendAction.release_pressure,
                                     "release_pressure")
action_release_team_pressure = CoreAction(e_BackendAction.release_team_pressure,
                                          "release_team_pressure")
action_release_switch = CoreAction(e_BackendAction.release_switch,
                                   "release_switch")
action_release_sprint = CoreAction(e_BackendAction.release_sprint,
                                   "release_sprint")
action_release_dribble = CoreAction(e_BackendAction.release_dribble,
                                    "release_dribble")

# ***** Define some action sets *****
action_set_v1 = [
    action_idle, action_left, action_top_left, action_top,
    action_top_right, action_right, action_bottom_right,
    action_bottom, action_bottom_left, action_long_pass,
    action_high_pass, action_short_pass, action_shot,
    action_sprint, action_release_direction, action_release_sprint,
    action_sliding, action_dribble, action_release_dribble]

action_set_v2 = action_set_v1 + [action_builtin_ai]

# Special action set that includes all the core actions in the same order.
full_action_set = action_set_v2 + [
    action_keeper_rush, action_pressure,
    action_team_pressure, action_switch,
    action_release_long_pass, action_release_high_pass,
    action_release_short_pass, action_release_shot,
    action_release_keeper_rush, action_release_sliding,
    action_release_pressure, action_release_team_pressure,
    action_release_switch,
]

action_set_dict = {
    "default": action_set_v1,
    "v2": action_set_v2,
    # "full" action set is needed by the play_game script.
    # Don't use it for training models.
    "full": full_action_set,
}

reverse_action_mapping = {
    action_long_pass: action_release_long_pass,
    action_high_pass: action_release_high_pass,
    action_short_pass: action_release_short_pass,
    action_shot: action_release_shot,
    action_keeper_rush: action_release_keeper_rush,
    action_sliding: action_release_sliding,
    action_pressure: action_release_pressure,
    action_team_pressure: action_release_team_pressure,
    action_switch: action_release_switch,
    action_sprint: action_release_sprint,
    action_dribble: action_release_dribble,
    action_release_long_pass: action_long_pass,
    action_release_high_pass: action_high_pass,
    action_release_short_pass: action_short_pass,
    action_release_shot: action_shot,
    action_release_keeper_rush: action_keeper_rush,
    action_release_sliding: action_sliding,
    action_release_pressure: action_pressure,
    action_release_team_pressure: action_team_pressure,
    action_release_switch: action_switch,
    action_release_sprint: action_sprint,
    action_release_dribble: action_dribble
}

# Returns action set specified by the config.
def get_action_set(config):
  action_set_name = config["action_set"]
  return action_set_dict[action_set_name]

def get_sticky_actions(config):
  """Returns list of sticky actions for the currently used action set."""
  sticky_actions = []
  for a in get_action_set(config):
    if a._sticky:
      sticky_actions.append(a)
  return sticky_actions


# Converts different action representation to an action from a given action set.
def named_action_from_action_set(action_set, action):
  if (hasattr(action, "__dict__") and action_set and
      set(action.__dict__) == set(action_set[0].__dict__)):
    return action

  if (isinstance(action, numpy.int32) or isinstance(action, numpy.int64) or
      isinstance(action, int)):
    # The action can be given as a numpy.int32 which cannot be
    # serialized. First convert it to a proper python integer.
    action = int(action)
    assert action < len(action_set), "Action outside of action set"
    return action_set[action]

  assert False, "Action {} not found in action set".format(action)


def disable_action(action):
  assert set(action.__dict__) == set(action_left.__dict__)
  if action._directional:
    return action_release_direction
  return reverse_action_mapping[action]
