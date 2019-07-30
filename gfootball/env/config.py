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


def parse_player_definition(definition):
  """Parses player definition.

  An example of player definition is: "agent:players=4" or "replay:path=...".

  Args:
    definition: a string defining a player

  Returns:
    A tuple (name, dict).
  """
  name = definition
  d = {'left_players': 0,
       'right_players': 0}
  if ':' in definition:
    (name, params) = definition.split(':')
    for param in params.split(','):
      (key, value) = param.split('=')
      d[key] = value
  if d['left_players'] == 0 and d['right_players'] == 0:
    d['left_players'] = 1
  return name, d


def count_players(definition):
  """Returns a number of players given a definition."""
  _, player_definition = parse_player_definition(definition)
  return (int(player_definition['left_players']) +
          int(player_definition['right_players']))


def count_left_players(definition):
  """Returns a number of left players given a definition."""
  return int(parse_player_definition(definition)[1]['left_players'])


def count_right_players(definition):
  """Returns a number of players given a definition."""
  return int(parse_player_definition(definition)[1]['right_players'])


def get_agent_number_of_players(players):
  """Returns a total number of players controlled by an agent."""
  return sum([count_players(player) for player in players
              if player.startswith('agent')])


class Config(object):

  def __init__(self, values=None):
    self._values = {
        'action_set': 'default',
        'enable_sides_swap': False,
        'display_game_stats': True,
        'dump_full_episodes': False,
        'dump_scores': False,
        'game_difficulty': 0.6,
        'players': ['agent:left_players=1'],
        'level': '11_vs_11_stochastic',
        'physics_steps_per_frame': 10,
        'real_time': False,
        'render': False,
        'tracesdir': '/tmp/dumps',
        'write_video': False
    }
    if values:
      self._values.update(values)
    self.NewScenario()

  def number_of_left_players(self):
    return sum([count_left_players(player)
                for player in self._values['players']])

  def number_of_right_players(self):
    return sum([count_right_players(player)
                for player in self._values['players']])

  def number_of_players_agent_controls(self):
    return get_agent_number_of_players(self._values['players'])

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
