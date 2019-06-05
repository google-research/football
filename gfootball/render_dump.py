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

"""Script allowing to render a replay video from a game dump."""

from absl import app
from absl import flags
from gfootball.env import config
from gfootball.env import observation_processor

import six.moves.cPickle

FLAGS = flags.FLAGS
flags.DEFINE_string('file', '', 'Dump file to render')


def main(_):
  cfg = config.Config()
  cfg['dump_full_episodes'] = True
  cfg['write_video'] = True
  cfg['display_game_stats'] = True
  with open(FLAGS.file, 'rb') as f:
    dump = six.moves.cPickle.load(f)
  processor = observation_processor.ObservationProcessor(cfg)
  for frame in dump:
    processor.update(frame)
  processor.write_dump('episode_done')

if __name__ == '__main__':
  app.run(main)
