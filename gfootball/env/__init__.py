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

from gfootball.env import config
from gfootball.env import football_env
from gfootball.env import observation_preprocessing
from gfootball.env import wrappers


def create_environment(env_name='',
                       stacked=False,
                       representation='extracted',
                       rewards='scoring',
                       enable_goal_videos=False,
                       enable_full_episode_videos=False,
                       render=False,
                       write_video=False,
                       dump_frequency=1,
                       logdir='',
                       extra_players=None,
                       number_of_left_players_agent_controls=1,
                       number_of_right_players_agent_controls=0,
                       enable_sides_swap=False,
                       channel_dimensions=(
                           observation_preprocessing.SMM_WIDTH,
                           observation_preprocessing.SMM_HEIGHT)):
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
        downsampled to 'channel_dimensions'. The observation size is:
        'channel_dimensions'x3 (or 'channel_dimensions'x12 when "stacked" is
        True).
      'pixels_gray': the observation is the rendered view of the football field
        in gray scale and downsampled to 'channel_dimensions'. The observation
        size is 'channel_dimensions'x1 (or 'channel_dimensions'x4 when stacked
        is True).
      'extracted': also referred to as super minimap. The observation is
        composed of 4 planes of size 'channel_dimensions'.
        Its size is then 'channel_dimensions'x4 (or 'channel_dimensions'x16 when
        stacked is True).
        The first plane P holds the position of the 11 player of the left
        team, P[y,x] is one if there is a player at position (x,y), otherwise,
        its value is zero.
        The second plane holds in the same way the position of the 11 players
        of the right team.
        The third plane holds the active player of the left team.
        The last plane holds the position of the ball.
      'simple115': the observation is a vector of size 115. It holds:
         - the ball_position and the ball_direction as (x,y,z)
         - one hot encoding of who controls the ball.
           [1, 0, 0]: nobody, [0, 1, 0]: left team, [0, 0, 1]: right team.
         - one hot encoding of size 11 to indicate who is the active player
           in the left team.
         - 11 (x,y) positions for each player of the left team.
         - 11 (x,y) motion vectors for each player of the left team.
         - 11 (x,y) positions for each player of the right team.
         - 11 (x,y) motion vectors for each player of the right team.
         - one hot encoding of the game mode. Vector of size 7 with the
           following meaning:
           {NormalMode, KickOffMode, GoalKickMode, FreeKickMode,
            CornerMode, ThrowInMode, PenaltyMode}.
         Can only be used when the scenario is a flavor of normal game
         (i.e. 11 versus 11 players).
    rewards: Comma separated list of rewards to be added.
       Currently supported rewards are 'scoring' and 'checkpoints'.
    enable_goal_videos: whether to dump traces up to 200 frames before goals.
    enable_full_episode_videos: whether to dump traces for every episode.
    render: whether to render game frames.
       Must be enable when rendering videos or when using pixels
       representation.
    write_video: whether to dump videos when a trace is dumped.
    dump_frequency: how often to write dumps/videos (in terms of # of episodes)
      Sub-sample the episodes for which we dump videos to save some disk space.
    logdir: directory holding the logs.
    extra_players: A list of extra players to use in the environment.
        Each player is defined by a string like:
        "$player_name:left_players=?,right_players=?,$param1=?,$param2=?...."
    number_of_left_players_agent_controls: Number of left players an agent
        controls.
    number_of_right_players_agent_controls: Number of right players an agent
        controls.
    enable_sides_swap: Whether to randomly pick a field side at the beginning of
       each episode for the team that the agent controls.
    channel_dimensions: (width, height) tuple that represents the dimensions of
       SMM or pixels representation.
  Returns:
    Google Research Football environment.
  """
  assert env_name
  players = [('agent:left_players=%d,right_players=%d' % (
      number_of_left_players_agent_controls,
      number_of_right_players_agent_controls))]
  if extra_players is not None:
    players.extend(extra_players)
  c = config.Config({
      'enable_sides_swap': enable_sides_swap,
      'dump_full_episodes': enable_full_episode_videos,
      'dump_scores': enable_goal_videos,
      'players': players,
      'level': env_name,
      'render': render,
      'tracesdir': logdir,
      'write_video': write_video,
  })
  env = football_env.FootballEnv(c)
  if dump_frequency > 1:
    env = wrappers.PeriodicDumpWriter(env, dump_frequency)
  assert 'scoring' in rewards.split(',')
  if 'checkpoints' in rewards.split(','):
    env = wrappers.CheckpointRewardWrapper(env)
  if representation.startswith('pixels'):
    env = wrappers.PixelsStateWrapper(env, 'gray' in representation,
                                      channel_dimensions)
  elif representation == 'simple115':
    env = wrappers.Simple115StateWrapper(env)
  elif representation == 'extracted':
    env = wrappers.SMMWrapper(env, channel_dimensions)
  else:
    raise ValueError('Unsupported representation: {}'.format(representation))
  if (number_of_left_players_agent_controls +
      number_of_right_players_agent_controls == 1):
    env = wrappers.SingleAgentObservationWrapper(env)
    env = wrappers.SingleAgentRewardWrapper(env)
  if stacked:
    # Import FrameStack here to avoid unconditional dependence on baselines.
    from baselines.common.atari_wrappers import FrameStack
    env = FrameStack(env, 4)

  return env
