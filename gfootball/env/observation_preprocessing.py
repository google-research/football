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

SMM_LAYERS = ['home_team', 'away_team', 'active', 'ball']

# Normalized minimap coordinates
MINIMAP_NORM_X_MIN = -1.0
MINIMAP_NORM_X_MAX = 1.0
MINIMAP_NORM_Y_MIN = -1.0 / 2.25
MINIMAP_NORM_Y_MAX = 1.0 / 2.25


def generate_smm(observation):
  """Returns minimap observation given the raw features.

  Args:
    observation: raw features from the environment
  """
  frame = np.zeros((SMM_HEIGHT, SMM_WIDTH, len(SMM_LAYERS)), dtype=np.uint8)
  for index, layer in enumerate(SMM_LAYERS):
    if layer not in observation:
      continue
    points = np.array(observation[layer]).reshape(-1)
    for p in range(len(points) // 2):
      x = int(
          (points[p * 2] - MINIMAP_NORM_X_MIN) /
          (MINIMAP_NORM_X_MAX - MINIMAP_NORM_X_MIN) * SMM_WIDTH)
      y = int(
          (points[p * 2 + 1] - MINIMAP_NORM_Y_MIN) /
          (MINIMAP_NORM_Y_MAX - MINIMAP_NORM_Y_MIN) * SMM_HEIGHT)
      x = max(0, min(SMM_WIDTH - 1, x))
      y = max(0, min(SMM_HEIGHT - 1, y))
      frame[y, x, index] = 255
  return frame
