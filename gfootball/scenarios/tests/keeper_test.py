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
  builder.config().game_duration = 30
  builder.config().deterministic = False
  if builder.EpisodeNumber() % 2 == 0:
    builder.SetBallPosition(0.9, 0.3)
  else:
    builder.SetBallPosition(-0.9, -0.3)
  builder.SetTeam(Team.e_Left)
  builder.AddPlayer(-1.00, 0.00, e_PlayerRole_GK, True)
  builder.AddPlayer(0.85, 0.30, e_PlayerRole_RM, True)
  builder.AddPlayer(0.00, 0.00, e_PlayerRole_RM, True)
  builder.SetTeam(Team.e_Right)
  builder.AddPlayer(-1.00, 0.00, e_PlayerRole_GK, True)
  builder.AddPlayer(0.85, 0.30, e_PlayerRole_RM, True)
