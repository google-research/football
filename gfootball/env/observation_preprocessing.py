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

from gfootball.env import football_action_set
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

_MARKER_VALUE = 255


def get_smm_layers(config):
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
    frame[y, x] = _MARKER_VALUE


def generate_smm(observation, config=None,
                 channel_dimensions=(SMM_WIDTH, SMM_HEIGHT)):
  """Returns a list of minimap observations given the raw features for each
  active player.

  Args:
    observation: raw features from the environment
    config: environment config
    channel_dimensions: resolution of SMM to generate

  Returns:
    (N, H, W, C) - shaped np array representing SMM. N stands for the number of
    players we are controlling.
  """
  frame = np.zeros((len(observation), channel_dimensions[1],
                    channel_dimensions[0], len(get_smm_layers(config))),
                   dtype=np.uint8)

  for o_i, o in enumerate(observation):
    for index, layer in enumerate(get_smm_layers(config)):
      assert layer in o
      if layer == 'active':
        if o[layer] == -1:
          continue
        mark_points(frame[o_i, :, :, index],
                    np.array(o['left_team'][o[layer]]).reshape(-1))
      else:
        mark_points(frame[o_i, :, :, index], np.array(o[layer]).reshape(-1))
  return frame
