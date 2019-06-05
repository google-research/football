// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GFOOTBALL_ACTIONS_H
#define GFOOTBALL_ACTIONS_H

enum Action {
  game_idle = 0,
  game_left = 1,
  game_top_left = 2,
  game_top = 3,
  game_top_right = 4,
  game_right = 5,
  game_bottom_right = 6,
  game_bottom = 7,
  game_bottom_left = 8,
  game_long_pass = 9,
  game_high_pass = 10,
  game_short_pass = 11,
  game_shot = 12,
  game_keeper_rush = 13,
  game_sliding = 14,
  game_pressure = 15,
  game_team_pressure = 16,
  game_switch = 17,
  game_sprint = 18,
  game_dribble = 19,
  game_release_direction = 20,
  game_release_long_pass = 21,
  game_release_high_pass = 22,
  game_release_short_pass = 23,
  game_release_shot = 24,
  game_release_keeper_rush = 25,
  game_release_sliding = 26,
  game_release_pressure = 27,
  game_release_team_pressure = 28,
  game_release_switch = 29,
  game_release_sprint = 30,
  game_release_dribble = 31
};

#endif
