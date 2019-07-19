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

from absl import app
from absl import flags


from gfootball.env import config
from gfootball.env import football_env

FLAGS = flags.FLAGS

flags.DEFINE_string(
    'left_players', 'keyboard',
    'Comma separated list of left players, single keyboard player by default')
flags.DEFINE_string('right_players', '', 'List of right players')
flags.DEFINE_string('level', '', 'Level to play')
flags.DEFINE_enum('action_set', 'full', ['default', 'full'], 'Action set')
flags.DEFINE_bool('real_time', True,
                  'If true, environment will slow down so humans can play.')


def main(_):
  left_players = FLAGS.left_players.split(',') if FLAGS.left_players else ''
  right_players = FLAGS.right_players.split(',') if FLAGS.right_players else ''
  assert not (
      'agent' in left_players or 'agent' in right_players
  ), 'Player type \'agent\' can not be used with play_game. Use tfhub player.'
  cfg = config.Config({
      'action_set': FLAGS.action_set,
      'right_players': right_players,
      'dump_full_episodes': True,
      'left_players': left_players,
      'real_time': FLAGS.real_time,
      'render': True
  })
  if FLAGS.level:
    cfg['level'] = FLAGS.level
  env = football_env.FootballEnv(cfg)
  env.reset()
  try:
    while True:
      _, _, done, _ = env.step(None)
      if done:
        env.reset()
  except KeyboardInterrupt:
    env.write_dump('shutdown')
    exit(1)


if __name__ == '__main__':
  app.run(main)
