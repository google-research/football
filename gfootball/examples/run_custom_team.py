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

"""Example of running a scenario with custom teams."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl import app
from absl import flags
from absl import logging

import gfootball.env as football_env
from gfootball.env import custom_team
from gfootball.env import scenario_builder

FLAGS = flags.FLAGS

def main(_):
  # Define Left Team (Red Jersey, Blue Shorts)
  left_team = custom_team.Team(
      name="Red Devils",
      kit_color=[0.9, 0.1, 0.1],
      shorts_color=[0.1, 0.1, 0.9]
  )

  # Goalkeeper with high stats
  gk_stats = [0.9] * 22
  left_team.add_player(custom_team.Player(
      x=-1.0, y=0.0,
      role=scenario_builder.Role.e_PlayerRole_GK,
      stats=gk_stats,
      kit_no=1
  ))

  # Striker
  striker_stats = [0.7] * 22
  left_team.add_player(custom_team.Player(
      x=0.0, y=0.0,
      role=scenario_builder.Role.e_PlayerRole_CF,
      stats=striker_stats,
      kit_no=9
  ))

  # Define Right Team (Blue Jersey, White Shorts)
  right_team = custom_team.Team(
      name="Blue Angels",
      kit_color=[0.1, 0.1, 0.9],
      shorts_color=[0.9, 0.9, 0.9]
  )

  right_team.add_player(custom_team.Player(
      x=-1.0, y=0.0,
      role=scenario_builder.Role.e_PlayerRole_GK
  ))

  right_team.add_player(custom_team.Player(
      x=-0.5, y=0.0,
      role=scenario_builder.Role.e_PlayerRole_CB
  ))

  # Merge configs
  config_update = {}
  config_update.update(left_team.to_config(side="left"))
  config_update.update(right_team.to_config(side="right"))

  # Create Environment
  env = football_env.create_environment(
      env_name='custom',
      other_config_options=config_update,
      render=False
  )

  logging.info("Environment created successfully with custom teams.")

  env.reset()
  # Run a few steps to verify
  for _ in range(10):
      action = env.action_space.sample()
      env.step(action)

  env.close()
  logging.info("Simulation finished.")

if __name__ == '__main__':
  app.run(main)
