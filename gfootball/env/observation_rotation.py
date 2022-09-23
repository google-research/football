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


def rotate_sticky_actions(sticky_actions_state, config):
  """Rotate the sticky bits of directional actions.

  This is used to make a policy believe it is playing from left to right
  although it is actually playing from right to left.

  Args:
    sticky_actions_state: Array of bits corresponding to the active actions.
    config: config used by the environment

  Returns:
    Array of bits corresponding to the same active actions for a player
    who would play from the opposite side.
  """
  sticky_actions = football_action_set.get_sticky_actions(config)
  assert len(sticky_actions) == len(sticky_actions_state), len(sticky_actions)
  action_to_state = {}
  for i in range(len(sticky_actions)):
    action_to_state[sticky_actions[i]] = sticky_actions_state[i]
  rotated_sticky_actions = []
  for i in range(len(sticky_actions)):
    rotated_sticky_actions.append(action_to_state[flip_single_action(
        sticky_actions[i], config)])
  return rotated_sticky_actions


def flip_team_observation(observation, result, config, from_team, to_team):
  """Rotates team-specific observations."""
  result['{}_team'.format(to_team)] = rotate_points(
      observation['{}_team'.format(from_team)])
  result['{}_team_direction'.format(to_team)] = rotate_points(
      observation['{}_team_direction'.format(from_team)])
  result['{}_team_tired_factor'.format(to_team)] = observation[
      '{}_team_tired_factor'.format(from_team)]
  result['{}_team_active'.format(to_team)] = observation[
      '{}_team_active'.format(from_team)]
  result['{}_team_yellow_card'.format(to_team)] = observation[
      '{}_team_yellow_card'.format(from_team)]
  result['{}_team_roles'.format(to_team)] = observation['{}_team_roles'.format(
      from_team)]
  result['{}_team_active'.format(to_team)] = observation[
      '{}_team_active'.format(from_team)]
  result['{}_team_designated_player'.format(to_team)] = observation[
      '{}_team_designated_player'.format(from_team)]
  if '{}_agent_controlled_player'.format(from_team) in observation:
    result['{}_agent_controlled_player'.format(to_team)] = observation[
        '{}_agent_controlled_player'.format(from_team)]
  if '{}_agent_sticky_actions'.format(from_team) in observation:
    result['{}_agent_sticky_actions'.format(to_team)] = [
        rotate_sticky_actions(sticky, config)
        for sticky in observation['{}_agent_sticky_actions'.format(from_team)]
    ]


def flip_observation(observation, config):
  """Observation corresponding to the field rotated by 180 degrees."""
  flipped_observation = {}
  flipped_observation['ball'] = rotate_3d_point(observation['ball'])
  flipped_observation['ball_direction'] = rotate_3d_point(
      observation['ball_direction'])
  flipped_observation['ball_rotation'] = observation['ball_rotation']
  flipped_observation['ball_owned_team'] = 1 - observation[
      'ball_owned_team'] if observation['ball_owned_team'] > -1 else -1
  flipped_observation['ball_owned_player'] = observation['ball_owned_player']
  flipped_observation['score'] = [
      observation['score'][1], observation['score'][0]
  ]
  flipped_observation['game_mode'] = observation['game_mode']
  flipped_observation['steps_left'] = observation['steps_left']
  flip_team_observation(observation, flipped_observation, config, 'left',
                        'right')
  flip_team_observation(observation, flipped_observation, config, 'right',
                        'left')
  return flipped_observation


def flip_single_action(action, config):
  """Actions corresponding to the field rotated by 180 degrees."""
  action = football_action_set.named_action_from_action_set(
      football_action_set.get_action_set(config), action)
  if action == football_action_set.action_left:
    return football_action_set.action_right
  if action == football_action_set.action_top_left:
    return football_action_set.action_bottom_right
  if action == football_action_set.action_top:
    return football_action_set.action_bottom
  if action == football_action_set.action_top_right:
    return football_action_set.action_bottom_left
  if action == football_action_set.action_right:
    return football_action_set.action_left
  if action == football_action_set.action_bottom_right:
    return football_action_set.action_top_left
  if action == football_action_set.action_bottom:
    return football_action_set.action_top
  if action == football_action_set.action_bottom_left:
    return football_action_set.action_top_right
  return action


def flip_action(action, config):
  if isinstance(action, np.ndarray) or isinstance(action, list):
    return [flip_single_action(a, config) for a in action]
  return flip_single_action(action, config)
