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


"""Set of functions used by command line scripts."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from gfootball.env import config
from gfootball.env import football_action_set
from gfootball.env import football_env
from gfootball.env import observation_processor

import copy
import six.moves.cPickle
import os
import tempfile


class ScriptHelpers(object):
  """Set of methods used by command line scripts."""

  def __init__(self):
    pass

  def __modify_trace(self, replay, fps):
    """Adopt replay to the new framerate and add additional steps at the end."""
    trace = []
    min_fps = replay[0]['debug']['config']['physics_steps_per_frame']
    assert fps % min_fps == 0, (
        'Trace has to be rendered in framerate being multiple of {}'.format(
            min_fps))
    assert fps <= 100, ('Framerate of up to 100 is supported')
    empty_steps = int(fps / min_fps) - 1
    for f in replay:
      trace.append(f)
      idle_step = copy.deepcopy(f)
      idle_step['debug']['action'] = [football_action_set.action_idle
                                     ] * len(f['debug']['action'])
      for _ in range(empty_steps):
        trace.append(idle_step)
    # Add some empty steps at the end, so that we can record videos.
    for _ in range(10):
      trace.append(idle_step)
    return trace

  def __build_players(self, dump_file, spec):
    players = []
    for player in spec:
      players.extend(['replay:path={},left_players=1'.format(
          dump_file)] * config.count_left_players(player))
      players.extend(['replay:path={},right_players=1'.format(
          dump_file)] * config.count_right_players(player))
    return players

  def load_dump(self, dump_file):
    dump = []
    with open(dump_file, 'rb') as in_fd:
      while True:
        try:
          step = six.moves.cPickle.load(in_fd)
        except EOFError:
          return dump
        dump.append(step)

  def dump_to_txt(self, dump_file, output, include_debug):
    with open(output, 'w') as out_fd:
      dump = self.load_dump(dump_file)
    if not include_debug:
      for s in dump:
        if 'debug' in s:
          del s['debug']
    with open(output, 'w') as f:
      f.write(str(dump))

  def dump_to_video(self, dump_file):
    dump = self.load_dump(dump_file)
    cfg = config.Config(dump[0]['debug']['config'])
    cfg['dump_full_episodes'] = True
    cfg['write_video'] = True
    cfg['display_game_stats'] = True
    processor = observation_processor.ObservationProcessor(cfg)
    processor.write_dump('episode_done')
    for frame in dump:
      processor.update(frame)

  def replay(self, dump, fps=10, config_update={}, directory=None, render=True):
    replay = self.load_dump(dump)
    trace = self.__modify_trace(replay, fps)
    fd, temp_path = tempfile.mkstemp(suffix='.dump')
    with open(temp_path, 'wb') as f:
      for step in trace:
        six.moves.cPickle.dump(step, f)
    assert replay[0]['debug']['frame_cnt'] == 0, (
        'Trace does not start from the beginning of the episode, can not replay')
    cfg = config.Config(replay[0]['debug']['config'])
    cfg['players'] = self.__build_players(temp_path, cfg['players'])
    config_update['physics_steps_per_frame'] = int(100 / fps)
    config_update['real_time'] = False
    if directory:
      config_update['tracesdir'] = directory
    config_update['write_video'] = True
    cfg.update(config_update)
    env = football_env.FootballEnv(cfg)
    if render:
      env.render()
    env.reset()
    done = False
    try:
      while not done:
        _, _, done, _ = env.step([])
    except KeyboardInterrupt:
      env.write_dump('shutdown')
      exit(1)
    os.close(fd)
