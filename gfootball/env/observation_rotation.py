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


"""Rotate observation by 180 degrees.

Context: Agents are trained to play left to right.
If one needs the same agent play right to left, a simple way
is to rotate the observation by 180 degrees and pass this representation
to the agent.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from gfootball.env import football_action_set

import numpy as np


def rotate_3d_point(point):
  """Rotate 3d point around the center of the field.

  Args:
    points:  [x, y, z] point.

  Returns:
    The rotated points.
  """
  # This assumes the center of the field is the origin: (0, 0)
  return np.array([-point[0], -point[1], point[2]])


def rotate_points(points):
  """Rotate the points around the center of the field.

  Args:
    points:  Numpy array holding one or several points.

  Returns:
    The rotated points.
  """
  # This assumes the center of the field is the origin: (0, 0)
  return -points


def rotate_sticky_actions(sticky_actions):
  """Rotate the sticky bits of directional actions.

  This is used to make a policy believe it is playing from left to right
  although it is actually playing from right to left.

  Args:
    sticky_actions: Array of bits corresponding to the active actions.

  Returns:
    Array of bits corresponding to the same active actions for a player
    who would play from the opposite side.
  """
  # Re-order the sticky bits of the directional actions.
  # !!! Note !!!: This depends of the order chosen in controller_provider
  #   0: controller_actions.game_left,
  #   1: controller_actions.game_top_left,
  #   2: controller_actions.game_top,
  #   3: controller_actions.game_top_right,
  #   4: controller_actions.game_right,
  #   5: controller_actions.game_bottom_right,
  #   6: controller_actions.game_bottom,
  #   7: controller_actions.game_bottom_left,
  rotated_sticky_actions = sticky_actions[:]
  sa = rotated_sticky_actions  # Short name alias.
  sa[0], sa[1], sa[2], sa[3], sa[4], sa[5], sa[6], sa[7] = (
      sa[4], sa[5], sa[6], sa[7], sa[0], sa[1], sa[2], sa[3])
  return sa


def flip_observation(observation):
  """Observation corresponding to the field rotated by 180 degrees."""
  flipped_observation = {}
  flipped_observation['ball'] = rotate_3d_point(observation['ball'])
  flipped_observation['ball_direction'] = rotate_3d_point(
      observation['ball_direction'])
  flipped_observation['ball_rotation'] = rotate_3d_point(
      observation['ball_rotation'])
  flipped_observation['ball_owned_team'] = 1 - observation[
      'ball_owned_team'] if observation['ball_owned_team'] > -1 else -1
  flipped_observation['ball_owned_player'] = observation['ball_owned_player']
  flipped_observation['home_team'] = rotate_points(observation['away_team'])
  flipped_observation['home_team_direction'] = rotate_points(
      observation['away_team_direction'])
  flipped_observation['home_team_tired_factor'] = observation[
      'away_team_tired_factor']
  flipped_observation['home_team_active'] = observation[
      'away_team_active']
  flipped_observation['home_team_yellow_card'] = observation[
      'away_team_yellow_card']
  flipped_observation['home_team_roles'] = observation['away_team_roles']
  flipped_observation['away_team'] = rotate_points(observation['home_team'])
  flipped_observation['away_team_direction'] = rotate_points(
      observation['home_team_direction'])
  flipped_observation['away_team_tired_factor'] = observation[
      'home_team_tired_factor']
  flipped_observation['away_team_active'] = observation[
      'home_team_active']
  flipped_observation['away_team_yellow_card'] = observation[
      'home_team_yellow_card']
  flipped_observation['away_team_roles'] = observation['home_team_roles']
  flipped_observation['score'] = [
      observation['score'][1], observation['score'][0]
  ]
  flipped_observation['game_mode'] = observation['game_mode']
  flipped_observation['steps_left'] = observation['steps_left']
  flipped_observation['active'] = observation['opponent_active']
  flipped_observation['sticky_actions'] = rotate_sticky_actions(
      observation['opponent_sticky_actions'])
  return flipped_observation


def flip_action(action):
  """Actions corresponding to the field rotated by 180 degrees."""
  if action == football_action_set.core_action_left:
    return football_action_set.core_action_right
  if action == football_action_set.core_action_top_left:
    return football_action_set.core_action_bottom_right
  if action == football_action_set.core_action_top:
    return football_action_set.core_action_bottom
  if action == football_action_set.core_action_top_right:
    return football_action_set.core_action_bottom_left
  if action == football_action_set.core_action_right:
    return football_action_set.core_action_left
  if action == football_action_set.core_action_bottom_right:
    return football_action_set.core_action_top_left
  if action == football_action_set.core_action_bottom:
    return football_action_set.core_action_top
  if action == football_action_set.core_action_bottom_left:
    return football_action_set.core_action_top_right

  return action
