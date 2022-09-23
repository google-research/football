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


"""Script helpers test."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import glob
import os
import zlib
import tempfile
from absl.testing import absltest

from gfootball.env import config
from gfootball.env import football_action_set
from gfootball.env import football_env
from gfootball.env import script_helpers

test_tmpdir = os.path.join(tempfile.gettempdir(), 'gfootball_test')


class ScriptHelpersTest(absltest.TestCase):

  def generate_replay(self):
    """Generates replay of an episode."""
    cfg = config.Config()
    left_players = 2
    cfg.update({
        'action_set': 'full',
        'level': 'tests.corner_test',
        'dump_full_episodes': True,
        'players': ['agent:left_players={}'.format(left_players),
                    'bot:right_players=1', 'lazy:right_players=1'],
        'tracesdir': test_tmpdir
    })
    env = football_env.FootballEnv(cfg)
    env.reset()
    actions_cnt = len(football_action_set.get_action_set(cfg))
    done = False
    step = 0
    while not done:
      step += 1
      actions = [(step + x) % actions_cnt for x in range(left_players)]
      _, _, done, _ = env.step(actions)
    env.close()

  def compute_hash(self, trace_file):
    replay = script_helpers.ScriptHelpers().load_dump(trace_file)
    hash_value = 0
    for frame in replay:
      del frame['debug']
      hash_value = zlib.adler32(str(tuple(sorted(frame.items()))).encode(),
                                hash_value)
    return hash_value

  def current_dumps(self):
    return set(glob.glob(os.path.join(test_tmpdir, 'episode_done*dump')))

  def test__replay(self):
    """Has to run first, as it generates dumps for other tests."""
    dumps_before = self.current_dumps()
    self.generate_replay()
    dumps_after = self.current_dumps()
    dump1 = dumps_after - dumps_before
    assert len(dump1) == 1, dump1
    hash1 = self.compute_hash(list(dump1)[0])
    cfg = {
        'dump_full_episodes': True,
    }
    script_helpers.ScriptHelpers().replay(list(dump1)[0],
                                          directory=test_tmpdir,
                                          config_update=cfg,
                                          render=False)
    dump2 = self.current_dumps() - dumps_after
    assert len(dump2) == 1, dump2
    hash2 = self.compute_hash(list(dump2)[0])
    self.assertEqual(hash1, hash2)

  def test_dump_to_txt(self):
    dump = list(self.current_dumps())[0]
    out_file = dump.replace('dump', 'txt')
    script_helpers.ScriptHelpers().dump_to_txt(dump, out_file, True)
    self.assertTrue(os.path.isfile(out_file))

  def test_dump_to_video(self):
    dump = list(self.current_dumps())[0]
    script_helpers.ScriptHelpers().dump_to_video(dump)


if __name__ == '__main__':
  absltest.main()
