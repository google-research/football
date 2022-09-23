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






from . import *


def build_scenario(builder):
  builder.config().game_duration = 50
  builder.config().deterministic = True
  if builder.EpisodeNumber() % 2 == 0:
    first_team = Team.e_Right
    second_team = Team.e_Left
  else:
    first_team = Team.e_Left
    second_team = Team.e_Right
  builder.SetTeam(first_team)
  builder.AddPlayer(-0.050000, 0.000000, e_PlayerRole_GK)
  builder.AddPlayer(0.8000000, 0.000000, e_PlayerRole_RM)
  builder.SetTeam(second_team)
  builder.AddPlayer(0.000000, 0.400000, e_PlayerRole_GK, True)
  builder.AddPlayer(0.000000, -0.400000, e_PlayerRole_RM, True)
