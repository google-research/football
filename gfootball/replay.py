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

"""Script allowing to replay a given trace file.
   Example usage:
   python replay.py --trace_file=/tmp/dumps/shutdown_20190521-165136974075.dump
"""


from gfootball.env import football_action_set
from gfootball.env import football_env
from gfootball.env import config

from absl import app
from absl import flags
import copy
import six.moves.cPickle
import tempfile
import tensorflow as tf
import os

FLAGS = flags.FLAGS

flags.DEFINE_string('trace_file', '', 'Trace file to replay')
flags.DEFINE_integer('fps', 50, 'How many frames per second to render')


def modify_trace(replay):
  """Adopt replay to the new framerate and add additional steps at the end."""
  trace = []
  min_fps = replay[0]['debug']['config']['physics_steps_per_frame']
  assert FLAGS.fps % min_fps == 0, (
      'Trace has to be rendered in framerate being multiple of {}'.formmat(
          min_fps))
  assert FLAGS.fps <= 100, ('Framerate of up to 100 is supported')
  empty_steps = int(FLAGS.fps / min_fps) - 1
  for f in replay:
    trace.append(f)
    idle_step = copy.deepcopy(f)
    idle_step['debug']['action'] = [football_action_set.core_action_idle
                                   ] * len(f['debug']['action'])
    for _ in range(empty_steps):
      trace.append(idle_step)
  # Add some empty steps at the end, so that we can record videos.
  for _ in range(10):
    trace.append(idle_step)
  return trace


def main(_):
  with open(FLAGS.trace_file, 'rb') as f:
    replay = six.moves.cPickle.load(f)
  trace = modify_trace(replay)
  fd, temp_path = tempfile.mkstemp(suffix='.dump')
  with tf.gfile.Open(temp_path, 'wb') as f:
    six.moves.cPickle.dump(trace, f)
  assert replay[0]['debug']['frame_cnt'] == 1, (
      'Trace does not start from the beginning of the episode, can not replay')
  cfg = config.Config(replay[0]['debug']['config'])
  player_type = 'replay={}'.format(temp_path)
  cfg['home_players'] = [player_type] * len(cfg['home_players'])
  cfg['away_players'] = [player_type] * len(cfg['away_players'])
  cfg.update({
      'physics_steps_per_frame': int(100 / FLAGS.fps),
      'real_time': False,
      'render': True,
      'tracesdir': '/tmp/dumps',
      'write_video': True
  })
  env = football_env.FootballEnv(cfg)
  env.reset(cfg)
  done = False
  try:
    while not done:
      _, _, done, _ = env.step(None)
  except KeyboardInterrupt:
    env.write_dump('shutdown')
    exit(1)
  os.close(fd)


if __name__ == '__main__':
  app.run(main)
