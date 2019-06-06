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

"""GFootball Environment."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function


from baselines.common.atari_wrappers import FrameStack
from gfootball.env import config
from gfootball.env import football_env
from gfootball.env import wrappers


def create_environment(env_name='',
                       stacked=False,
                       representation='extracted',
                       with_checkpoints=False,
                       enable_goal_videos=False,
                       enable_full_episode_videos=False,
                       render=False,
                       write_video=False,
                       dump_frequency=1,
                       logdir='',
                       data_dir=None,
                       font_file=None,
                       away_player=None):
  """Creates a Google Research Football environment.

  Args:
    env_name: a name of a scenario to run, e.g. "11_vs_11_stochastic".
      The list of scenarios can be found in directory "scenarios".
    stacked: If True, stack 4 observations, otherwise, only the last
      observation is returned by the environment.
      Stacking is only possible when representation is one of the following:
      "pixels", "pixels_gray" or "extracted".
      In that case, the stacking is done along the last (i.e. channel)
      dimension.
    representation: String to define the representation used to build
      the observation. It can be one of the following:
      'pixels': the observation is the rendered view of the football field
        downsampled to w=96, h=72. The observation size is: 72x96x3
        (or 72x96x12 when "stacked" is True).
      'pixels_gray': the observation is the rendered view of the football field
        in gray scale and downsampled to w=96, h=72. The observation size is
        72x96x1 (or 72x96x4 when stacked is True).
      'extracted': also referred to as super minimap. The observation is
        composed of 4 planes of size w=96, h=72.
        Its size is then 72x96x4 (or 72x96x16 when stacked is True).
        The first plane P holds the position of the 11 player of the home
        team, P[y,x] is one if there is a player at position (x,y), otherwise,
        its value is zero.
        The second plane holds in the same way the position of the 11 players
        of the away team.
        The third plane holds the active player of the home team.
        The last plane holds the position of the ball.
      'simple115': the observation is a vector of size 115. It holds:
         - the ball_position and the ball_direction as (x,y,z)
         - one hot encoding of who controls the ball.
           [1, 0, 0]: nobody, [0, 1, 0]: home team, [0, 0, 1]: away team.
         - one hot encoding of size 11 to indicate who is the active player
           in the home team.
         - 11 (x,y) positions for each player of the home team.
         - 11 (x,y) motion vectors for each player of the home team.
         - 11 (x,y) positions for each player of the away team.
         - 11 (x,y) motion vectors for each player of the away team.
         - one hot encoding of the game mode. Vector of size 7 with the
           following meaning:
           {NormalMode, KickOffMode, GoalKickMode, FreeKickMode,
            CornerMode, ThrowInMode, PenaltyMode}.
         Can only be used when the scenario is a flavor of normal game
         (i.e. 11 versus 11 players).
    with_checkpoints: True to add intermediate checkpoint rewards to guide
       the agent to move to the opponent goal.
       If False, only scoring provides a reward.
    enable_goal_videos: whether to dump traces up to 200 frames before goals.
    enable_full_episode_videos: whether to dump traces for every episode.
    render: whether to render game frames.
       Must be enable when rendering videos or when using pixels
       representation.
    write_video: whether to dump videos when a trace is dumped.
    dump_frequency: how often to write dumps/videos (in terms of # of episodes)
      Sub-sample the episodes for which we dump videos to save some disk space.
    logdir: directory holding the logs.
    data_dir: location of the game engine data
       Safe to leave as the default value.
    font_file: location of the game font file
       Safe to leave as the default value.
    away_player: Away player (adversary) to use in the environment.
       Reserved for future usage to provide an opponent to train against.
       (which could be used for self-play).

  Returns:
    Google Research Football environment.
  """
  assert env_name
  away_players = [away_player] if away_player else []
  c = config.Config({
      'dump_full_episodes': enable_full_episode_videos,
      'dump_scores': enable_goal_videos,
      'level': env_name,
      'render': render,
      'tracesdir': logdir,
      'write_video': write_video,
      'away_players': away_players,
  })
  if data_dir:
    c['data_dir'] = data_dir
  if font_file:
    c['font_file'] = font_file
  env = football_env.FootballEnv(c)
  if dump_frequency > 1:
    env = wrappers.PeriodicDumpWriter(env, dump_frequency)
  if with_checkpoints:
    env = wrappers.CheckpointRewardWrapper(env)
  if representation.startswith('pixels'):
    env = wrappers.PixelsStateWrapper(env, 'gray' in representation)
  elif representation == 'simple21':
    env = wrappers.Simple21StateWrapper(env)
  elif representation == 'simple115':
    env = wrappers.Simple115StateWrapper(env)
  elif representation == 'extracted':
    env = wrappers.SMMWrapper(env)
  else:
    raise ValueError('Unsupported representation: {}'.format(representation))
  if stacked:
    env = FrameStack(env, 4)
  return env
