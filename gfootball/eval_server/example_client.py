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

"""Example football client.

It creates remote football game with given credentials and plays a few games.
"""

import random

from absl import app
from absl import flags
from absl import logging
import gfootball.env as football_env
from gfootball.env import football_action_set
import grpc
import numpy as np
import tensorflow.compat.v2 as tf

FLAGS = flags.FLAGS
flags.DEFINE_string('username', None, 'Username to use')
flags.mark_flag_as_required('username')
flags.DEFINE_string('token', None, 'Token to use.')
flags.DEFINE_integer('how_many', 1000, 'How many games to play')
flags.DEFINE_bool('render', False, 'Whether to render a game.')
flags.DEFINE_string('track', '', 'Name of the competition track.')
flags.DEFINE_string('model_name', '',
                    'A model identifier to be displayed on the leaderboard.')
flags.DEFINE_string('inference_model', '',
                    'A path to an inference model. Empty for random actions')

NUM_ACTIONS = len(football_action_set.action_set_dict['default'])


def random_actions(_):
  return random.randint(0, NUM_ACTIONS - 1)


def seed_rl_preprocessing(observation):
  observation = np.expand_dims(observation, axis=0)
  data = np.packbits(observation, axis=-1)  # This packs to uint8
  if data.shape[-1] % 2 == 1:
    data = np.pad(data, [(0, 0)] * (data.ndim - 1) + [(0, 1)], 'constant')
  return data.view(np.uint16)


def get_inference_model(inference_model):
  if not inference_model or FLAGS.username == 'random':
    return random_actions
  model = tf.saved_model.load(inference_model)
  return lambda x: model(seed_rl_preprocessing(x))[0][0].numpy()


def main(unused_argv):
  model = get_inference_model(FLAGS.inference_model)
  env = football_env.create_remote_environment(
      FLAGS.username, FLAGS.token, FLAGS.model_name, track=FLAGS.track,
      representation='extracted', stacked=True,
      include_rendering=FLAGS.render)
  for _ in range(FLAGS.how_many):
    ob = env.reset()
    cnt = 1
    done = False
    while not done:
      try:
        action = model(ob)
        ob, rew, done, _ = env.step(action)
        logging.info('Playing the game, step %d, action %d, rew %f, done %d',
                     cnt, action, rew, done)
        cnt += 1
      except grpc.RpcError as e:
        print(e)
        break
    print('=' * 50)


if __name__ == '__main__':
  app.run(main)
