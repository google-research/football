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


"""Set of supported controller actions."""

# Definition has to mirror 1:1 football_backend actions, the purpose of having
# this class is to break dependency on the backend (C++).


class ControllerAction(object):

  def __init__(self, backend_id):
    self.backend_id = backend_id


game_idle = ControllerAction(0)
game_left = ControllerAction(1)
game_top_left = ControllerAction(2)
game_top = ControllerAction(3)
game_top_right = ControllerAction(4)
game_right = ControllerAction(5)
game_bottom_right = ControllerAction(6)
game_bottom = ControllerAction(7)
game_bottom_left = ControllerAction(8)
game_long_pass = ControllerAction(9)
game_high_pass = ControllerAction(10)
game_short_pass = ControllerAction(11)
game_shot = ControllerAction(12)
game_keeper_rush = ControllerAction(13)
game_sliding = ControllerAction(14)
game_pressure = ControllerAction(15)
game_team_pressure = ControllerAction(16)
game_switch = ControllerAction(17)
game_sprint = ControllerAction(18)
game_dribble = ControllerAction(19)
game_release_direction = ControllerAction(20)
game_release_long_pass = ControllerAction(21)
game_release_high_pass = ControllerAction(22)
game_release_short_pass = ControllerAction(23)
game_release_shot = ControllerAction(24)
game_release_keeper_rush = ControllerAction(25)
game_release_sliding = ControllerAction(26)
game_release_pressure = ControllerAction(27)
game_release_team_pressure = ControllerAction(28)
game_release_switch = ControllerAction(29)
game_release_sprint = ControllerAction(30)
game_release_dribble = ControllerAction(31)
