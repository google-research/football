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
  builder.config().game_duration = 3000
  builder.config().right_team_difficulty = 0.05
  builder.config().left_team_difficulty = 0.05
  builder.config().deterministic = False
  if builder.EpisodeNumber() % 2 == 0:
    first_team = Team.e_Left
    second_team = Team.e_Right
  else:
    first_team = Team.e_Right
    second_team = Team.e_Left
  builder.SetTeam(first_team)
  builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK, controllable=False)
  builder.AddPlayer(0.000000,  0.020000, e_PlayerRole_RM)
  builder.AddPlayer(0.000000, -0.020000, e_PlayerRole_CF)
  builder.AddPlayer(-0.1, -0.1, e_PlayerRole_LB)
  builder.AddPlayer(-0.1,  0.1, e_PlayerRole_CB)
  builder.SetTeam(second_team)
  builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK, controllable=False)
  builder.AddPlayer(-0.04,  0.040000, e_PlayerRole_RM)
  builder.AddPlayer(-0.04, -0.040000, e_PlayerRole_CF)
  builder.AddPlayer(-0.1, -0.1, e_PlayerRole_LB)
  builder.AddPlayer(-0.1,  0.1, e_PlayerRole_CB)
