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

"""Runs football_env on OpenAI's ppo2."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import multiprocessing
import os
from absl import app
from absl import flags
from baselines import logger
from baselines.bench import monitor
from baselines.common.vec_env.subproc_vec_env import SubprocVecEnv
from baselines.ppo2 import ppo2
import gfootball.env as football_env
from gfootball.examples import models  


FLAGS = flags.FLAGS

flags.DEFINE_string('level', 'academy_empty_goal_close',
                    'Defines type of problem being solved')
flags.DEFINE_enum('state', 'extracted_stacked', ['extracted',
                                                 'extracted_stacked'],
                  'Observation to be used for training.')
flags.DEFINE_enum('reward_experiment', 'scoring',
                  ['scoring', 'scoring,checkpoints'],
                  'Reward to be used for training.')
flags.DEFINE_enum('policy', 'cnn', ['cnn', 'lstm', 'mlp', 'impala_cnn',
                                    'gfootball_impala_cnn'],
                  'Policy architecture')
flags.DEFINE_integer('num_timesteps', int(2e6),
                     'Number of timesteps to run for.')
flags.DEFINE_integer('num_envs', 8,
                     'Number of environments to run in parallel.')
flags.DEFINE_integer('nsteps', 128, 'Number of environment steps per epoch; '
                     'batch size is nsteps * nenv')
flags.DEFINE_integer('noptepochs', 4, 'Number of updates per epoch.')
flags.DEFINE_integer('nminibatches', 8,
                     'Number of minibatches to split one epoch to.')
flags.DEFINE_integer('save_interval', 100,
                     'How frequently checkpoints are saved.')
flags.DEFINE_integer('seed', 0, 'Random seed.')
flags.DEFINE_float('lr', 0.00008, 'Learning rate')
flags.DEFINE_float('ent_coef', 0.01, 'Entropy coeficient')
flags.DEFINE_float('gamma', 0.993, 'Discount factor')
flags.DEFINE_float('cliprange', 0.27, 'Clip range')
flags.DEFINE_float('max_grad_norm', 0.5, 'Max gradient norm (clipping)')
flags.DEFINE_bool('render', False, 'If True, environment rendering is enabled.')
flags.DEFINE_bool('dump_full_episodes', False,
                  'If True, trace is dumped after every episode.')
flags.DEFINE_bool('dump_scores', False,
                  'If True, sampled traces after scoring are dumped.')
flags.DEFINE_string('load_path', None, 'Path to load initial checkpoint from.')


def create_single_football_env(iprocess):
  """Creates gfootball environment."""
  env = football_env.create_environment(
      env_name=FLAGS.level, stacked=('stacked' in FLAGS.state),
      rewards=FLAGS.reward_experiment,
      logdir=logger.get_dir(),
      write_goal_dumps=FLAGS.dump_scores and (iprocess == 0),
      write_full_episode_dumps=FLAGS.dump_full_episodes and (iprocess == 0),
      render=FLAGS.render and (iprocess == 0),
      dump_frequency=50 if FLAGS.render and iprocess == 0 else 0)
  env = monitor.Monitor(env, logger.get_dir() and os.path.join(logger.get_dir(),
                                                               str(iprocess)))
  return env


def train(_):
  """Trains a PPO2 policy."""
  vec_env = SubprocVecEnv([
      (lambda _i=i: create_single_football_env(_i))
      for i in range(FLAGS.num_envs)
  ], context=None)

  # Import tensorflow after we create environments. TF is not fork sake, and
  # we could be using TF as part of environment if one of the players is
  # controlled by an already trained model.
  import tensorflow.compat.v1 as tf
  ncpu = multiprocessing.cpu_count()
  config = tf.ConfigProto(allow_soft_placement=True,
                          intra_op_parallelism_threads=ncpu,
                          inter_op_parallelism_threads=ncpu)
  config.gpu_options.allow_growth = True
  tf.Session(config=config).__enter__()

  ppo2.learn(network=FLAGS.policy,
             total_timesteps=FLAGS.num_timesteps,
             env=vec_env,
             seed=FLAGS.seed,
             nsteps=FLAGS.nsteps,
             nminibatches=FLAGS.nminibatches,
             noptepochs=FLAGS.noptepochs,
             max_grad_norm=FLAGS.max_grad_norm,
             gamma=FLAGS.gamma,
             ent_coef=FLAGS.ent_coef,
             lr=FLAGS.lr,
             log_interval=1,
             save_interval=FLAGS.save_interval,
             cliprange=FLAGS.cliprange,
             load_path=FLAGS.load_path)


if __name__ == '__main__':
  app.run(train)
