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


"""Important environment constants."""

from __future__ import print_function

# How many physics steps game engine does per second.
PHYSICS_STEPS_PER_SECOND = 100

# List of observations exposed by the environment.
EXPOSED_OBSERVATIONS = {
    'ball', 'ball_direction', 'ball_rotation', 'ball_owned_team',
    'ball_owned_player', 'home_team', 'home_team_direction',
    'home_team_tired_factor', 'home_team_yellow_card', 'home_team_active',
    'home_team_roles', 'away_team', 'away_team_direction',
    'away_team_tired_factor', 'away_team_yellow_card', 'away_team_active',
    'away_team_roles', 'active', 'sticky_actions', 'score', 'steps_left',
    'game_mode'
}
