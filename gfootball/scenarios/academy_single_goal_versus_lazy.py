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

episode = 0


def build_scenario(builder):
  global episode
  episode += 1
  builder.config().game_duration = 3000
  builder.config().deterministic = False
  builder.config().offsides = False
  builder.config().end_episode_on_score = True
  builder.config().end_episode_on_out_of_play = True
  builder.config().end_episode_on_possession_change = True
  builder.SetBallPosition(-0.48, -0.06356)

  builder.SetTeam(Team.e_Left)
  builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK)
  builder.AddPlayer(0.000000, 0.020000, e_PlayerRole_RM)
  builder.AddPlayer(0.000000, -0.020000, e_PlayerRole_CF)
  builder.AddPlayer(-0.422000, -0.19576, e_PlayerRole_LB)
  builder.AddPlayer(-0.500000, -0.06356, e_PlayerRole_CB)
  builder.AddPlayer(-0.500000, 0.063559, e_PlayerRole_CB)
  builder.AddPlayer(-0.422000, 0.195760, e_PlayerRole_RB)
  builder.AddPlayer(-0.184212, -0.10568, e_PlayerRole_CM)
  builder.AddPlayer(-0.267574, 0.000000, e_PlayerRole_CM)
  builder.AddPlayer(-0.184212, 0.105680, e_PlayerRole_CM)
  builder.AddPlayer(-0.010000, -0.21610, e_PlayerRole_LM)

  # All right players are lazy (i.e., they don't move, except the keeper)
  builder.SetTeam(Team.e_Right)
  builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK)
  builder.AddPlayer(-0.050000, 0.000000, e_PlayerRole_RM, True)
  builder.AddPlayer(-0.010000, 0.216102, e_PlayerRole_CF, True)
  builder.AddPlayer(-0.422000, -0.19576, e_PlayerRole_LB, True)
  builder.AddPlayer(-0.500000, -0.06356, e_PlayerRole_CB, True)
  builder.AddPlayer(-0.500000, 0.063559, e_PlayerRole_CB, True)
  builder.AddPlayer(-0.422000, 0.195760, e_PlayerRole_RB, True)
  builder.AddPlayer(-0.184212, -0.10568, e_PlayerRole_CM, True)
  builder.AddPlayer(-0.267574, 0.000000, e_PlayerRole_CM, True)
  builder.AddPlayer(-0.184212, 0.105680, e_PlayerRole_CM, True)
  builder.AddPlayer(-0.010000, -0.21610, e_PlayerRole_LM, True)
