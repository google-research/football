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


"""Conversion functions for observations.
"""


from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import numpy as np
from six.moves import range


SMM_WIDTH = 96
SMM_HEIGHT = 72

SMM_LAYERS = ['left_team', 'right_team', 'ball', 'active']

# Normalized minimap coordinates
MINIMAP_NORM_X_MIN = -1.0
MINIMAP_NORM_X_MAX = 1.0
MINIMAP_NORM_Y_MIN = -1.0 / 2.25
MINIMAP_NORM_Y_MAX = 1.0 / 2.25


def get_smm_layers(config):
  if config and config['enable_sides_swap']:
    return [] + SMM_LAYERS + ['is_active_left']
  return SMM_LAYERS


def mark_points(frame, points):
  """Draw dots corresponding to 'points'.

  Args:
    frame: 2-d matrix representing one SMM channel ([y, x])
    points: a list of (x, y) coordinates to be marked
  """
  for p in range(len(points) // 2):
    x = int((points[p * 2] - MINIMAP_NORM_X_MIN) /
            (MINIMAP_NORM_X_MAX - MINIMAP_NORM_X_MIN) * frame.shape[1])
    y = int((points[p * 2 + 1] - MINIMAP_NORM_Y_MIN) /
            (MINIMAP_NORM_Y_MAX - MINIMAP_NORM_Y_MIN) * frame.shape[0])
    x = max(0, min(frame.shape[1] - 1, x))
    y = max(0, min(frame.shape[0] - 1, y))
    frame[y, x] = 255


def generate_smm(observation, config=None,
                 channel_dimensions=(SMM_WIDTH, SMM_HEIGHT)):
  """Returns a list of minimap observations given the raw features for each
  active player.

  Args:
    observation: raw features from the environment
    channel_dimensions: resolution of SMM to generate
    config: environment config

  Returns:
    (N, H, W, C) - shaped np array representing SMM. N stands for the number of
    players we are controlling.
  """
  active = observation['active']
  frame = np.zeros((len(active), channel_dimensions[1], channel_dimensions[0],
                    len(get_smm_layers(config))), dtype=np.uint8)
  for a_index, a in enumerate(active):
    for index, layer in enumerate(get_smm_layers(config)):
      if layer not in observation:
        continue
      if layer == 'active':
        if a == -1:
          continue
        team = ('right_team' if ('is_active_left' in observation and
                                 not observation['is_active_left'])
                else 'left_team')
        mark_points(frame[a_index, :, :, index],
                    np.array(observation[team][a]).reshape(-1))
      elif layer == 'is_active_left':
        frame[a_index, :, :, index] = 1 if observation[layer] else 0
      else:
        mark_points(frame[a_index, :, :, index],
                    np.array(observation[layer]).reshape(-1))
  return frame
