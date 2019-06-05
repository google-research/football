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


"""Class responsible for generating scenarios."""

import importlib
import logging
import os
import random

from absl import flags

import gfootball_engine as libgame

Player = libgame.FormationEntry
Role = libgame.e_PlayerRole
Team = libgame.e_Team

FLAGS = flags.FLAGS

def all_scenarios():
  path = os.path.abspath(__file__)
  path = os.path.join(os.path.dirname(os.path.dirname(path)), "scenarios")
  return [f[:-3] for f in os.listdir(path) if
          f.endswith(".py") and f != '__init__.py']

class Scenario(object):

  def __init__(self, config):
    # Game config controls C++ engine and is derived from the main config.
    self._scenario_cfg = libgame.ScenarioConfig()
    self._config = config
    self._active_team = Team.e_Home
    scenario = None
    try:
      scenario = importlib.import_module('gfootball.scenarios.{}'.format(config['level']))
    except ImportError as e:
      logging.warning('Loading scenario "%s" failed' % config['level'])
      logging.warning(e)
      exit(1)
    scenario.build_scenario(self)
    self.SetTeam(libgame.e_Team.e_Home)
    self._FakePlayersForEmptyTeam(self._scenario_cfg.home_team)
    self.SetTeam(libgame.e_Team.e_Away)
    self._FakePlayersForEmptyTeam(self._scenario_cfg.away_team)
    self._BuildScenarioConfig()

  def _FakePlayersForEmptyTeam(self, team):
    if len(team) == 0:
      self.AddPlayer(-1.000000, 0.420000, libgame.e_PlayerRole.e_PlayerRole_GK, True)

  def _BuildScenarioConfig(self):
    """Builds scenario config from gfootball.environment config."""
    self._scenario_cfg.real_time = self._config['real_time']
    self._scenario_cfg.home_agents = len(self._config['home_players'])
    self._scenario_cfg.away_agents = len(self._config['away_players'])
    self._scenario_cfg.offsides = self._config['offsides']
    self._scenario_cfg.render = self._config['render']
    # This is needed to record 'game_engine_random_seed' in the dump.
    if 'game_engine_random_seed' not in self._config._values:
      self._config.set_scenario_value('game_engine_random_seed',
                                      random.randint(0, 2000000000))

    if not self._config['deterministic']:
      self._scenario_cfg.game_engine_random_seed = (
          self._config['game_engine_random_seed'])

  def SetFlag(self, name, value):
    self._config.set_scenario_value(name, value)

  def SetTeam(self, team):
    self._active_team = team

  def AddPlayer(self, x, y, role, lazy=False):
    """Build player for the current scenario.

    Args:
      x: x coordinate of the player in the range [-1, 1].
      y: y coordinate of the player in the range [-0.42, 0.42].
      role: Player's role in the game (goal keeper etc.).
      lazy: Computer doesn't perform any automatic actions for lazy player.
    """
    player = Player(x, y, role, lazy)
    if self._active_team == Team.e_Home:
      self._scenario_cfg.home_team.append(player)
    else:
      self._scenario_cfg.away_team.append(player)

  def SetBallPosition(self, ball_x, ball_y):
    self._scenario_cfg.ball_position[0] = ball_x
    self._scenario_cfg.ball_position[1] = ball_y

  def EpisodeNumber(self):
    return self._config['episode_number']

  def ScenarioConfig(self):
    return self._scenario_cfg
