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


"""Football environment E2E test."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from absl.testing import parameterized
from collections.abc import Iterable
from multiprocessing import pool
from multiprocessing import Queue
import gfootball
import os
import platform
import sys
import random
import threading
import atexit
import zlib
from absl.testing import absltest

from gfootball.env import config
from gfootball.env import football_action_set
from gfootball.env import football_env
from gfootball.env import wrappers
from gfootball.env import scenario_builder
import numpy as np
import psutil
from six.moves import range

fast_run = False


def observation_hash(observation, hash_value = 0):
  for obs in observation:
    for key, value in sorted(obs.items()):
      hash_value = zlib.adler32(key.encode(), hash_value)
      hash_value = zlib.adler32(np.ascontiguousarray(value), hash_value)
  return hash_value


def compute_hash(env, actions):
  """Computes hash of observations returned by environment for a given scenario.

  Args:
    env: environment
    actions: number of actions

  Returns:
    hash
  """
  o = env.reset()
  hash_value = observation_hash(o)
  done = False
  step = 0
  while not done:
    o, _, done, _ = env.step(step % actions)
    hash_value = observation_hash(o, hash_value)
    step += 1
    if step >= 200:
      break
  return hash_value


def run_scenario(cfg, queue, actions, render=False, validation=True):
  env = football_env.FootballEnv(cfg)
  if render:
    env.render()
  obs = env.reset()
  queue.put(obs)
  if validation:
    # Otherwise the following error is generated on Windows:
    #  OverflowError: Python int too large to convert to C long
    # The number can be also derived by
    #  import struct
    #  end = 2 ** (struct.Struct('l').size * 8 - 1) - 1
    end = 999999999999999 if platform.system() != 'Windows' else 2147483647
    env.tracker_setup(0, end)
  done = False
  step = 0
  while not done:
    if isinstance(actions, Iterable):
      if step >= len(actions):
        break
      action = actions[step]
    else:
      action = actions.get()
      if action is None:
        break
    step += 1
    if isinstance(action, Iterable):
      obs, _, done, _ = env.step(action)
    else:
      obs, _, done, _ = env.step([action, action])
    queue.put(obs)
  queue.put(None)
  env.close()


def normalize_observation(o):
  if o['ball'][0] == -0:
    o['ball'][0] = 0
  if o['ball'][1] == -0:
    o['ball'][1] = 0
  if o['ball_direction'][0] == -0:
    o['ball_direction'][0] = 0
  if o['ball_direction'][1] == -0:
    o['ball_direction'][1] = 0


class FootballEnvTest(parameterized.TestCase):

  def compare_observations(self, l1, l2):
    for o1, o2 in zip(l1, l2):
      if 'frame' in o1 and 'frame' not in o2:
        del o1['frame']
      elif 'frame' in o2 and 'frame' not in o1:
        del o2['frame']
      normalize_observation(o1)
      normalize_observation(o2)
      o1 = str(tuple(sorted(o1.items())))
      o2 = str(tuple(sorted(o2.items())))
      self.assertEqual(o1, o2)

  def test___control_all_players(self):
    """Validate MultiAgentToSingleAgent wrapper and control_all_players flag."""
    try:
      gfootball.env.create_environment(
          env_name='tests.multiagent_wrapper',
          rewards='checkpoints,scoring',
          number_of_left_players_agent_controls=2)
    except AssertionError:
      pass
    else:
      self.assertTrue(False)

    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        representation='simple115v2',
        number_of_left_players_agent_controls=11,
        number_of_right_players_agent_controls=11)
    obs = env.reset()
    self.assertLen(obs, 22)
    self.assertIn(obs, env.observation_space)

    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        number_of_left_players_agent_controls=11,
        number_of_right_players_agent_controls=0)
    obs = env.reset()
    self.assertLen(obs, 11)
    self.assertIn(obs, env.observation_space)

    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        representation='simple115v2',
        number_of_left_players_agent_controls=0,
        number_of_right_players_agent_controls=11)
    obs = env.reset()
    self.assertLen(obs, 11)
    self.assertIn(obs, env.observation_space)

    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        number_of_left_players_agent_controls=1,
        number_of_right_players_agent_controls=1)
    obs = env.reset()
    self.assertLen(obs, 2)
    self.assertIn(obs, env.observation_space)

    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        number_of_left_players_agent_controls=1)
    obs = env.reset()
    self.assertEqual(np.shape(obs), (72, 96, 4))
    self.assertIn(obs, env.observation_space)
    obs, _, _, _ = env.step([football_action_set.action_left])
    self.assertEqual(np.shape(obs), (72, 96, 4))
    env = gfootball.env.create_environment(
        env_name='tests.multiagent_wrapper',
        rewards='checkpoints,scoring',
        representation='raw',
        number_of_left_players_agent_controls=1,
        number_of_right_players_agent_controls=1)
    obs = env.reset()
    self.assertLen(obs, 2)
    self.assertEqual(obs[0]['sticky_actions'][0], 0)
    self.assertEqual(obs[1]['sticky_actions'][4], 0)
    obs, _, _, _ = env.step(
        [football_action_set.action_idle, football_action_set.action_idle])
    obs, _, _, _ = env.step(
        [football_action_set.action_left, football_action_set.action_right])
    self.assertLen(obs, 2)
    self.assertEqual(obs[0]['sticky_actions'][0], 1)
    self.assertEqual(obs[1]['sticky_actions'][4], 1)

  def test_score_empty_goal(self):
    """Score on an empty goal."""
    cfg = config.Config()

    env = football_env.FootballEnv(cfg)
    cfg['level'] = 'academy_empty_goal'
    last_o = env.reset()[0]
    for _ in range(120):
      o, reward, done, _ = env.step(football_action_set.action_right)
      o = o[0]
      if done:
        self.assertEqual(reward, 1)
        break
      self.assertFalse(done)
      self.assertGreaterEqual(o['ball'][0], last_o['ball'][0] - 0.01)
      self.assertGreaterEqual(
          o['left_team'][o['active']][0],
          last_o['left_team'][last_o['active']][0] - 0.01)
      last_o = o
    self.assertTrue(done)
    env.close()

  def test_second_half(self):
    """Test second half feature."""
    cfg = config.Config()
    cfg['level'] = 'tests.second_half'
    env = football_env.FootballEnv(cfg)
    for _ in range(5):
      o, _, done, _ = env.step(football_action_set.action_idle)
      self.assertFalse(done)
      self.assertAlmostEqual(o[0]['left_team'][o[0]['active']][0], 0, delta=0.1)
    for _ in range(6):
      self.assertFalse(done)
      o, _, done, _ = env.step(football_action_set.action_idle)
      self.assertAlmostEqual(
          o[0]['left_team'][o[0]['active']][0], -0.5, delta=0.1)
    self.assertTrue(done)
    env.close()

  def test___render(self):
    """Make sure rendering is not broken."""
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Rendering is not supported.
      return
    cfg = config.Config({
        'level': 'tests.11_vs_11_hard_deterministic',
    })
    env = football_env.FootballEnv(cfg)
    env.render()
    o = env.reset()
    hash_value = observation_hash(o)
    for _ in range(10):
      o, _, _, _ = env.step(football_action_set.action_right)
      hash_value = observation_hash(o, hash_value)
    # Linux
    expected_hash_value = 4000732293
    if platform.system() == 'Windows':
      # On Windows we may have to check four possible values:
      #  for each of architectures (32 or 64 bits)
      #    what SDK was used (latest or from Visual Studio 2017)
      #  We will check only two values once we start using windows-latest VM again
      if sys.maxsize > 2 ** 32:  # x64
        expected_hash_value = 683941870 if hash_value != 3676773624 else 3676773624
      else:  # x86
        expected_hash_value = 1490754124 if hash_value != 2808421794 else 2808421794
    elif platform.system() == 'Darwin':
      expected_hash_value = 1865563121
    self.assertEqual(hash_value, expected_hash_value)
    env.close()

  def test_dynamic_render(self):
    """Verifies dynamic render support."""
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Rendering is not supported.
      return
    cfg = config.Config({
        'level': 'tests.11_vs_11_hard_deterministic',
    })
    env = football_env.FootballEnv(cfg)
    o = env.reset()
    for _ in range(10):
      o, _, _, _ = env.step(football_action_set.action_right)
      self.assertNotIn('frame', o[0])
      env.render()
      self.assertIn('frame', env.observation()[0])
      self.compare_observations(o, env.observation())
      o, _, _, _ = env.step(football_action_set.action_right)
      self.assertIn('frame', env.observation()[0])
      env.disable_render()
      self.compare_observations(o, env.observation())
    env.close()

  def test_different_action_formats(self):
    """Verify different action formats are accepted."""
    cfg = config.Config()
    env = football_env.FootballEnv(cfg)
    env.reset()
    env.step(football_action_set.action_right)
    env.step([football_action_set.action_right])
    env.step(np.array([football_action_set.action_right]))
    env.step(np.array(football_action_set.action_right))
    env.close()

  def test_multi_instance(self):
    """Validates that two instances of the env can run in the same thread."""

    def run_episode():
      if 'UNITTEST_IN_DOCKER' in os.environ:
        return 0
      cfg = config.Config({
          'level': 'tests.11_vs_11_hard_deterministic'
      })
      env = football_env.FootballEnv(cfg)
      actions = len(football_action_set.get_action_set(cfg))
      hash_value = compute_hash(env, actions)
      env.close()
      return hash_value

    tpool = pool.ThreadPool(processes=2)
    atexit.register(tpool.close)
    run1 = tpool.apply_async(run_episode)
    run2 = tpool.apply_async(run_episode)
    self.assertEqual(run1.get(), run2.get())

  def test_multi_render(self):
    """Only one rendering instance allowed at a time."""
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Rendering is not supported.
      return
    cfg = config.Config({})
    env1 = football_env.FootballEnv(cfg)
    env1.render()
    env1.reset()

    env2 = football_env.FootballEnv(cfg)
    try:
      env2.render()
    except AssertionError:
      env1.close()
      env2.close()
      # It is still possible to render.
      env3 = football_env.FootballEnv(cfg)
      env3.reset()
      env3.close()
      return
    assert False, 'Exception expected'

  def test_scenarios_are_at_least_loading(self):
    cfg = config.Config()
    for l in scenario_builder.all_scenarios():
      cfg['level'] = l
      unused_game_cfg = cfg.ScenarioConfig()

  def memory_usage(self):
    process = psutil.Process(os.getpid())
    return process.memory_info().rss

  def test__memory_usage(self):
    """Make sure memory usage is low when not recording videos."""
    # This test has to go first, so that memory usage is not affected.
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Forge doesn't support rendering.
      return
    cfg = config.Config({'write_video': False})
    env = football_env.FootballEnv(cfg)
    env.render()
    env.reset()
    initial_memory = self.memory_usage()
    for _ in range(100):
      _, _, _, _ = env.step(football_action_set.action_right)
    memory_usage = self.memory_usage() - initial_memory
    env.close()
    self.assertGreaterEqual(10000000, memory_usage)

  def test_player_order_invariant(self):
    """Checks that environment behaves the same regardless of players order."""
    players = ['agent:right_players=1', 'lazy:left_players=11']
    cfg = config.Config({
        'level': 'tests.11_vs_11_hard_deterministic',
        'players': players
    })
    env = football_env.FootballEnv(cfg)
    actions = len(football_action_set.get_action_set(cfg))
    hash_value1 = compute_hash(env, actions)
    players = [players[1], players[0]]
    cfg = config.Config({
        'level': 'tests.11_vs_11_hard_deterministic',
        'players': players
    })
    env = football_env.FootballEnv(cfg)
    hash_value2 = compute_hash(env, actions)
    self.assertEqual(hash_value1, hash_value2)
    env.close()

  @parameterized.parameters(range(1))
  def test_setstate(self, seed):
    """Checks setState functionality."""
    cfg1 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed,
        'reverse_team_processing': False
    })
    cfg2 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed + 10,
        'reverse_team_processing': False
    })
    env1 = football_env.FootballEnv(cfg1)
    env2 = football_env.FootballEnv(cfg2)
    initial_obs = env1.reset()
    env2.reset()
    initial_state = env1.get_state()
    env2.set_state(initial_state)
    random.seed(seed)
    actions = len(football_action_set.get_action_set(cfg1))
    first_action = random.randint(0, actions - 1)
    first_obs, _, _, _ = env1.step(first_action)
    _, _, _, _ = env2.step(first_action)
    step = 0
    limit = 10 if fast_run else 3000
    while step < limit:
      step += 1
      action = random.randint(0, actions - 1)
      if step % 10 == 0:
        env2.set_state(initial_state)
        self.compare_observations(initial_obs, env2.observation())
        env2.step(first_action)
        self.compare_observations(first_obs, env2.observation())
        env2.set_state(env1.get_state())
      self.compare_observations(env1.observation(), env2.observation())
      _, _, done1, _ = env1.step(action)
      _, _, done2, _ = env2.step(action)
      self.assertEqual(done1, done2)
      if done1:
        break
    env1.close()
    env2.close()

  @parameterized.parameters(range(1))
  def test_symmetry(self, seed):
    """Checks game symmetry."""
    processes = []
    cfg1 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed,
        'players': ['agent:left_players=1,right_players=1'],
        'reverse_team_processing': False,
    })
    cfg2 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed,
        'players': ['agent:left_players=1,right_players=1'],
        'reverse_team_processing': True,
    })
    random.seed(seed)
    action_cnt = len(football_action_set.get_action_set(cfg1))
    actions = [random.randint(0, action_cnt - 1) for _ in range(10 if fast_run else 3000)]
    queue1 = Queue()
    thread1 = threading.Thread(
        target=run_scenario, args=(cfg1, queue1, actions))
    thread1.start()
    queue2 = Queue()
    thread2 = threading.Thread(
        target=run_scenario, args=(cfg2, queue2, actions))
    thread2.start()
    while True:
      o1 = queue1.get()
      o2 = queue2.get()
      if not o1 or not o2:
        self.assertEqual(o1, o2)
        break
      self.compare_observations(o1[:1], o2[1:])
      self.compare_observations(o2[:1], o1[1:])
    thread1.join()
    thread2.join()

  @parameterized.parameters((1, 'left', True), (0, 'right', True),
                            (1, 'left', False), (0, 'right', False))
  def test_offside(self, episode, team2, reverse):
    cfg = config.Config({
        'level': 'tests.offside_test',
        'players': ['agent:{}_players=1'.format(team2)],
        'episode_number': episode,
        'reverse_team_processing': reverse,
    })
    env = football_env.FootballEnv(cfg)
    env.reset()
    o, _, done, _ = env.step(football_action_set.action_long_pass)
    done = False
    while not done and o[0]['right_team'][1][0] == 0:
      o, _, done, _ = env.step(football_action_set.action_idle)
    self.assertAlmostEqual(o[0]['ball'][0], 0.6, delta=0.4)
    self.assertAlmostEqual(o[0]['right_team'][0][0], 0.6, delta=0.4)
    self.assertAlmostEqual(o[0]['right_team'][1][0], 0.6, delta=0.4)
    self.assertAlmostEqual(o[0]['left_team'][0][0], -0.6, delta=0.4)
    self.assertAlmostEqual(o[0]['left_team'][1][0], -0.6, delta=0.4)
    env.close()

  @parameterized.parameters((0, 1, True), (1, -1, True), (0, 1, False),
                            (1, -1, False))
  def test_corner(self, episode, factor, reverse):
    cfg = config.Config({
        'level': 'tests.corner_test',
        'players': ['agent:left_players=1,right_players=1'],
        'episode_number': episode,
        'reverse_team_processing': reverse,
    })
    env = football_env.FootballEnv(cfg)
    o = env.reset()
    done = False
    while not done:
      o, _, done, _ = env.step([football_action_set.action_left,
                                football_action_set.action_left])
    self.assertAlmostEqual(o[0]['ball'][0], -0.95 * factor, delta=0.1)
    self.assertAlmostEqual(o[0]['ball'][1], 0.4 * factor, delta=0.1)
    self.assertAlmostEqual(o[0]['right_team'][0][0], 1, delta=0.1)
    self.assertAlmostEqual(o[0]['right_team'][1][0], -0.95 * factor, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][0][0], -0.95, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][1][0], -0.9 * factor, delta=0.2)
    env.close()

  def test_penalty(self):
    cfg = config.Config({
        'level': 'tests.penalty',
        'players': ['agent:left_players=1'],
    })
    env = football_env.FootballEnv(cfg)
    o = env.reset()
    done = False
    while not done:
      o, _, done, _ = env.step([football_action_set.action_sliding])
    self.assertAlmostEqual(o[0]['ball'][0], -0.809, delta=0.01)
    self.assertAlmostEqual(o[0]['ball'][1], 0.0, delta=0.01)
    self.assertAlmostEqual(o[0]['right_team'][0][0], 1, delta=0.1)
    self.assertAlmostEqual(o[0]['right_team'][1][0], -0.75, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][0][0], -0.95, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][1][0], -0.70, delta=0.1)
    env.close()

  @parameterized.parameters((0, -1, True), (1, 1, True), (0, -1, False),
                            (1, 1, False))
  def test_keeper_ball(self, episode, factor, reverse):
    cfg = config.Config({
        'level': 'tests.keeper_test',
        'players': ['agent:left_players=1,right_players=1'],
        'episode_number': episode,
        'reverse_team_processing': reverse,
    })
    env = football_env.FootballEnv(cfg)
    o = env.reset()
    done = False
    while not done:
      o, _, done, _ = env.step([football_action_set.action_right,
                                football_action_set.action_right])
    self.assertAlmostEqual(o[0]['ball'][0], -1.0 * factor, delta=0.1)
    self.assertAlmostEqual(o[0]['ball'][1], 0.0, delta=0.1)
    self.assertAlmostEqual(o[0]['right_team'][0][0], 1, delta=0.1)
    self.assertAlmostEqual(o[0]['right_team'][1][0], 0.4, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][0][0], -0.9, delta=0.1)
    self.assertAlmostEqual(o[0]['left_team'][1][0], -0.33, delta=0.1)
    env.close()

  @parameterized.parameters((0, True), (1, True), (0, False), (1, False))
  def test_goal(self, episode, reverse):
    cfg = config.Config({
        'level': 'tests.goal_test',
        'players': ['agent:left_players=1,right_players=1'],
        'episode_number': episode,
        'reverse_team_processing': reverse,
    })
    env = football_env.FootballEnv(cfg)
    o = env.reset()
    done = False
    while not done:
      o, _, done, _ = env.step(
          [football_action_set.action_right, football_action_set.action_right])
    self.assertAlmostEqual(o[0]['ball'][0], 0.0, delta=0.1)
    self.assertEqual(o[0]['score'][episode], 1)
    self.assertEqual(o[0]['score'][1 - episode], 0)
    env.close()

  @parameterized.parameters(range(1))
  def test_render_state_equals_norender(self, seed):
    """Checks that rendering game state is the same as non-rendering."""
    if 'UNITTEST_IN_DOCKER' in os.environ:
      # Forge doesn't support rendering.
      return
    cfg1 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed,
        'players': ['agent:left_players=1,right_players=1'],
        'reverse_team_processing': False,
    })
    cfg2 = config.Config({
        'level': 'tests.symmetric',
        'game_engine_random_seed': seed,
        'players': ['agent:left_players=1,right_players=1'],
        'reverse_team_processing': False,
    })
    random.seed(seed)
    action_cnt = len(football_action_set.get_action_set(cfg1))
    actions = [random.randint(0, action_cnt - 1) for _ in range(50)]
    queue1 = Queue()
    thread1 = threading.Thread(
        target=run_scenario, args=(cfg1, queue1, actions, False, False))
    thread1.start()
    queue2 = Queue()
    thread2 = threading.Thread(
        target=run_scenario, args=(cfg2, queue2, actions, True, False))
    thread2.start()
    while True:
      o1 = queue1.get()
      o2 = queue2.get()
      if not o1 or not o2:
        self.assertEqual(o1, o2)
        break
      self.compare_observations(o1, o2)
    thread1.join()
    thread2.join()

  def test_get_state_wrapper(self):
    env = gfootball.env.create_environment(
        stacked=True,
        env_name='academy_empty_goal',
        rewards='checkpoints,scoring')
    o = env.reset()
    state = env.get_state()
    reward1 = 0
    hash1 = 0
    while reward1 < 0.9:
      o, r, _, _ = env.step(football_action_set.action_right)
      reward1 += r
      hash1 = zlib.adler32(o, hash1)
    self.assertAlmostEqual(reward1, 0.9, delta=0.01)
    env.set_state(state)
    hash2 = 0
    reward2 = 0
    while reward2 < 0.9:
      o, r, _, _ = env.step(football_action_set.action_right)
      reward2 += r
      hash2 = zlib.adler32(o, hash2)
    self.assertAlmostEqual(reward2, 0.9, delta=0.01)
    self.assertEqual(hash1, hash2)

  def test_restore_after_reset(self):
    cfg = config.Config({
        'level': '11_vs_11_competition',
    })
    env = football_env.FootballEnv(cfg)
    obs = env.reset()
    state = env.get_state()
    env.reset()
    env.set_state(state)
    obs_ = env.observation()
    state_ = env.get_state()
    env.step(0)  # Test if can take step
    self.compare_observations(obs, obs_)
    self.assertEqual(state, state_)

  def test_restore_after_done(self):
    cfg = config.Config({
        'level': 'academy_empty_goal_close',
    })
    env = football_env.FootballEnv(cfg)
    env.reset()
    state = env.get_state()
    # Go right until reaching the goal.
    done = False
    while not done:
      _, _, done, _ = env.step(5)
    env.set_state(state)
    env.step(0)  # Test if can take step


if __name__ == '__main__':
  absltest.main(failfast=True)
