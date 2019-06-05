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


"""Config loader."""

from __future__ import print_function

import copy
import logging
import os
import sys
import traceback

from absl import flags

import gfootball_engine as libgame

FLAGS = flags.FLAGS

logging.basicConfig(level=logging.INFO, format='%(asctime)s: %(message)s')


def log(func):
  # Change to True to enable tracing / debugging.
  debug = False
  if not debug:
    return func
  else:
    def wrapper(*args, **kwargs):
      try:
        logging.error('ENTER %s:%s', func.__module__, func.__name__)
        res = func(*args, **kwargs)
        logging.error('EXIT %s:%s', func.__module__, func.__name__)
        return res
      except Exception as e:
        logging.error('Exception: %s', traceback.format_exc())
        sys.exit(1)

    return wrapper


class Config(object):

  def __init__(self, values=None):
    self._values = {
        'action_set': 'default',
        'away_players': [],
        'data_dir':
            os.path.abspath(os.path.join(os.path.dirname(libgame.__file__),
                                         'data')),
        'font_file':
            os.path.abspath(os.path.join(os.path.dirname(libgame.__file__),
                                         'fonts', 'AlegreyaSansSC-ExtraBold.ttf')),
        'display_game_stats': True,
        'dump_full_episodes': False,
        'dump_scores': False,
        'game_difficulty': 0.6,
        'home_players': ['agent'],
        'level': '11_vs_11_stochastic',
        'physics_steps_per_frame': 10,
        'real_time': False,
        'render': False,
        'tracesdir': '/tmp/dumps',
        'write_video': False
    }
    if 'GFOOTBALL_DATA_DIR' in os.environ:
      self._values['data_dir'] = os.environ['GFOOTBALL_DATA_DIR']
    if 'GFOOTBALL_FONT' in os.environ:
      self._values['font_file'] = os.environ['GFOOTBALL_FONT']
    if values:
      self._values.update(values)
    self.NewScenario()

  def __eq__(self, other):
    assert isinstance(other, self.__class__)
    return self._values == other._values and self._scenario_values == other._scenario_values

  def __ne__(self, other):
    return not self.__eq__(other)

  def __getitem__(self, key):
    if key in self._scenario_values:
      return self._scenario_values[key]
    return self._values[key]

  def __setitem__(self, key, value):
    self._values[key] = value

  def get_dictionary(self):
    cfg = copy.deepcopy(self._values)
    cfg.update(self._scenario_values)
    return cfg

  def set_scenario_value(self, key, value):
    """Override value of specific config key for a single episode."""
    self._scenario_values[key] = value

  def serialize(self):
    return self._values

  def update(self, config):
    self._values.update(config)

  def GameConfig(self):
    cfg = libgame.GameConfig()
    cfg.render_mode = libgame.e_RenderingMode.e_Onscreen if self[
        'render'] else libgame.e_RenderingMode.e_Disabled
    cfg.high_quality = self['render']
    cfg.data_dir = self['data_dir']
    cfg.font_file = self['font_file']
    cfg.game_difficulty = self['game_difficulty']
    cfg.physics_steps_per_frame = self['physics_steps_per_frame']
    return cfg

  def ScenarioConfig(self):
    return self._scenario_cfg

  def NewScenario(self):
    if 'episode_number' not in self._values:
      self._values['episode_number'] = 0
    self._values['episode_number'] += 1
    self._scenario_values = {
        'deterministic': False,
        'end_episode_on_score': False,
        'end_episode_on_possession_change': False,
        'end_episode_on_out_of_play': False,
        'game_duration': 3000,
        'offsides': True,
    }
    from gfootball.env import scenario_builder
    self._scenario_cfg = scenario_builder.Scenario(self).ScenarioConfig()
