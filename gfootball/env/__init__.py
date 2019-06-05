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
    env_name: a name of a scenario to run
    stacked: whether to stack observations
    representation: what kind of representation to return
    with_checkpoints: whether to add checkpoint reward
    enable_goal_videos: whether to dump video of goals
    enable_full_episode_videos: whether to dump full episode videos
    render: whether to render game frames
    write_video: whether to dump videos
    dump_frequency: how often to write dumps/videos (in terms of # of episodes)
    logdir: logdir
    data_dir: location of the game engine data
    font_file: location of the game font file
    away_player: Away player (adversary) to use in the environment.

  Returns:
    Google Research Football environment.
  """
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
  else:
    env = wrappers.SMMWrapper(env)
  if stacked:
    env = FrameStack(env, 4)
  return env
