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

"""Helper classes for designing custom teams."""

from gfootball.env import scenario_builder

class Player:
    def __init__(self, x, y, role, lazy=False, controllable=True, kit_no=0, stats=None):
        self.data = {
            'x': x,
            'y': y,
            'role': role,
            'lazy': lazy,
            'controllable': controllable,
            'kit_no': kit_no,
            'player_stats': stats
        }

class Team:
    def __init__(self, name, kit_color, shorts_color=None):
        self.name = name
        self.kit_color = kit_color
        self.shorts_color = shorts_color
        self.players = []

    def add_player(self, player):
        self.players.append(player)

    def to_config(self, side="left"):
        cfg = {
            f"{side}_team_name": self.name,
            f"{side}_team_color": self.kit_color,
            f"{side}_team_players": [p.data for p in self.players]
        }
        if self.shorts_color:
            cfg[f"{side}_team_color2"] = self.shorts_color
        return cfg
