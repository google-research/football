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


"""Environment that can be used with OpenAI Baselines."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import collections
import cv2
from gfootball.env import football_action_set
from gfootball.env import observation_preprocessing
import gym
import numpy as np


class PartyObservationWrapper(gym.ObservationWrapper):
  """A wrapper that converts an observation to 115-features state."""

  def __init__(self, env, fixed_positions=False):
    """Initializes the wrapper.

    Args:
      env: an envorinment to wrap
      fixed_positions: whether to fix observation indexes corresponding to teams
    Note: simple115v2 enables fixed_positions option.
    """
    gym.ObservationWrapper.__init__(self, env)
    action_shape = np.shape(self.env.action_space)
    shape = (action_shape[0] if len(action_shape) else 1, 115)
    self.observation_space = gym.spaces.Box(
        low=-np.inf, high=np.inf, shape=shape, dtype=np.float32)
    self._fixed_positions = fixed_positions

  def observation(self, observation):
    """Converts an observation into simple115 (or simple115v2) format."""
    return OurObservationWrapper.convert_observation(observation, self._fixed_positions)

  @staticmethod
  def convert_observation(observation, fixed_positions):
    """Converts an observation into simple115 (or simple115v2) format.

    Args:
      observation: observation that the environment returns
      fixed_positions: Players and positions are always occupying 88 fields
                       (even if the game is played 1v1).
                       If True, the position of the player will be the same - no
                       matter how many players are on the field:
                       (so first 11 pairs will belong to the first team, even
                       if it has less players).
                       If False, then the position of players from team2
                       will depend on number of players in team1).

    Returns:
      (N, 655) shaped representation, where N stands for the number of players
      being controlled.
      *   22 - (x,y) coordinates of left team players
      *   22 - (x,y) direction of left team players
      *   22 - (x,y) coordinates of right team players
      *   22 - (x, y) direction of right team players
      *   3 - (x, y and z) - ball position
      *   3 - ball direction
      *   3 - one hot encoding of ball ownership (none, left, right)
      *   11 - one hot encoding of which player is active
      *   7 - one hot encoding of `game_mode`
      *   11 - number of yellow cards for each player of the left team
      *   11 - number of yellow cards for each player of the right team
      *   11 - tired factor for each player of the left team
      *   11 - ntired factor for each player of the right team
      *   2 - relative pose between active player and ball
      *   10 - vector 1 or 0 dependung on which action is active
      *   242 - (x,y) relative position between teamates
      *   242 - (x,y) relative position between opponents
    """

    def do_flatten(obj):
      """Run flatten on either python list or numpy array."""
      if type(obj) == list:
        return np.array(obj).flatten()
      return obj.flatten()

    final_obs = []
    for obs in observation:
      o = []
      if fixed_positions:
        for i, name in enumerate(['left_team', 'left_team_direction',
                                  'right_team', 'right_team_direction']):
          o.extend(do_flatten(obs[name]))
          # If there were less than 11vs11 players we backfill missing values
          # with -1.
          if len(o) < (i + 1) * 22:
            o.extend([-1] * ((i + 1) * 22 - len(o)))
      else:
        o.extend(do_flatten(obs['left_team']))
        o.extend(do_flatten(obs['left_team_direction']))
        o.extend(do_flatten(obs['right_team']))
        o.extend(do_flatten(obs['right_team_direction']))

      # If there were less than 11vs11 players we backfill missing values with
      # -1.
      # 88 = 11 (players) * 2 (teams) * 2 (positions & directions) * 2 (x & y)
      if len(o) < 88:
        o.extend([-1] * (88 - len(o)))

      # ball position
      o.extend(obs['ball'])
      # ball direction
      o.extend(obs['ball_direction'])
      # one hot encoding of which team owns the ball
      if obs['ball_owned_team'] == -1:
        o.extend([1, 0, 0])
      if obs['ball_owned_team'] == 0:
        o.extend([0, 1, 0])
      if obs['ball_owned_team'] == 1:
        o.extend([0, 0, 1])

      active = [0] * 11
      if obs['active'] != -1:
        active[obs['active']] = 1
      o.extend(active)

      game_mode = [0] * 7
      game_mode[obs['game_mode']] = 1
      o.extend(game_mode)


      yellow_cards_left = obs['left_team_yellow_card']
      o.extend(yellow_cards_left)

      yellow_cards_right = obs['right_team_yellow_card']
      o.extend(yellow_cards_right)

      left_team_tired_factor = obs['left_team_tired_factor']
      o.extend(left_team_tired_factor)

      right_team_tired_factor = obs['right_team_tired_factor']
      o.extend(right_team_tired_factor)

      if obs['active'] != -1:
        active_player_pose = np.array(obs['left_team'][obs['active']])
        ball_pose =np.array(obs['ball'][:-1])
        rel_position = active_player_pose - ball_pose
        o.extend(rel_position)


      rel_pos_teamates = []
      for i in range(11):
        for j in range(11):
          rel_pos = None
          if i == j: 
            rel_pos = [0 , 0] # Not sure how to encode this 
          elif not obs['left_team_active'][i] or not obs['left_team_active'][j]:
            rel_pos = [0, 0] # Not sure how to encode this
          else:
            rel_pos = np.array(obs['left_team'][i]) - np.array(obs['left_team'][j])
          rel_pos_teamates.append(rel_pos)

      o.extend(do_flatten(rel_pos_teamates))

      rel_pos_opponents = []
      for i in range(11):
        for j in range(11):
          rel_pos = None
          if not obs['left_team_active'][i] or not obs['right_team_active'][j]:
            rel_pos = [0, 0] # Not sure how to encode this
          else:
            rel_pos = np.array(obs['left_team'][i]) - np.array(obs['right_team'][j])
          rel_pos_opponents.append(rel_pos)

      o.extend(do_flatten(rel_pos_opponents))      

      sticky = obs['sticky_actions']
      o.extend(sticky)

      final_obs.append(o)
    return np.array(final_obs, dtype=np.float32)


