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

"""Script allowing to play the game by multiple players."""


from gfootball.env import football_env
from gfootball.env import config

from absl import app
from absl import flags

FLAGS = flags.FLAGS

# For Impala tfhub modules, use tfhub_impala=[module_dir], like this:
# --home_players=tfhub_impala=/usr/local/google/home/michaz/model_ine
flags.DEFINE_string(
    'home_players', 'keyboard',
    'Comma separated list of home players, single keyboard player by default')
flags.DEFINE_string('away_players', '', 'List of away players')
flags.DEFINE_string('level', '', 'Level to play')
flags.DEFINE_enum('action_set', 'full', ['default', 'full'], 'Action set')
flags.DEFINE_bool('real_time', True,
                  'If true, environment will slow down so humans can play.')


def main(_):
  cfg = config.Config({
      'action_set': FLAGS.action_set,
      'away_players':
          FLAGS.away_players.split(',') if FLAGS.away_players else '',
      'dump_full_episodes': True,
      'home_players':
          FLAGS.home_players.split(',') if FLAGS.home_players else '',
      'real_time': FLAGS.real_time,
      'render': True
  })
  if FLAGS.level:
    cfg['level'] = FLAGS.level
  env = football_env.FootballEnv(cfg)
  env.reset(cfg)
  try:
    while True:
      _, _, done, _ = env.step(None)
      if done:
        env.reset(cfg)
  except KeyboardInterrupt:
    env.write_dump('shutdown')
    exit(1)


if __name__ == '__main__':
  app.run(main)
