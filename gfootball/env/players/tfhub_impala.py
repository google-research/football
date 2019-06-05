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

"""Player from Impala tfhub module."""

import json
import os
from gfootball.env import football_action_set
from gfootball.env import observation_preprocessing
from gfootball.env import player_base
import numpy as np
import tensorflow as tf
import tensorflow_hub as hub

nest = tf.contrib.framework.nest


def get_flat_dict_from_structure(structure):
  flattened = nest.flatten_with_joined_string_paths(structure)
  return {k: v for k, v in flattened}


def get_placeholders_dict(input_info_dict):
  res = {}
  for k, v in input_info_dict.items():
    res[k] = tf.placeholder(dtype=v.dtype, shape=v.get_shape())
  return res


OBSERVATION_KINDS = ['extracted', 'extracted_stacked']



class SimpleObservationProcessor(object):
  """Observation processor for PlayerTfhubImpala."""

  def __init__(self, observation_kind):
    assert observation_kind in OBSERVATION_KINDS
    self._observation_kind = observation_kind
    self._stacked_size = 4 if 'stacked' in observation_kind else 1
    self._data = []

  def add_raw_observation(self, observation):
    if 'extracted' in self._observation_kind:
      observation = observation_preprocessing.generate_smm(observation)
    else:
      assert False, 'Unsupported observation kind!'
    if self._data:
      self._data = self._data + [observation]
      self._data = self._data[-self._stacked_size:]
    else:
      self._data = [observation] * self._stacked_size

  def get_observation(self):
    return np.expand_dims(np.concatenate(self._data, axis=-1), axis=0)


class Player(player_base.PlayerBase):
  """Impala agent loaded from TF Hub module."""

  def __init__(self, config):
    module_path = config['param']
    self._module = hub.Module(module_path)
    self._placeholders = get_placeholders_dict(
        self._module.get_input_info_dict())

    # Load additional info
    self._recurrent_state_size = 0
    with tf.gfile.Open(os.path.join(module_path, 'info.json'), 'r') as f:
      data = json.load(f)
      recurrent = data['recurrent_policy']
      if recurrent:
        self._recurrent_state_size = data['lstm_num_units']
      # Name it so that it is not confused with LSTM state.
      self._observation_type = data['state']
      self._action_set = data['action_set']
      assert (config['action_set'] == data['action_set'] or
              config['action_set'] == 'full')

    self._state = None
    self._last_action = 0
    self._observation_processor = None
    self.reset()

    self._output = self._module(self._placeholders, as_dict=True)
    self._sess = tf.Session()
    self._sess.run([tf.global_variables_initializer()])

  def get_all_zeros_input(self):
    res = {}
    for _, v in self._placeholders.items():
      if v.dtype == 'string':
        res[v] = ['_']
      else:
        res[v] = np.zeros(dtype=v.dtype.as_numpy_dtype, shape=v.shape)
    return res

  def take_action(self, observations):
    self._observation_processor.add_raw_observation(observations)
    obs = self._observation_processor.get_observation()
    score_reward = ((observations['score'][0] - observations['score'][0]) -
                    (self._score[0] - self._score[1]))
    self._score = observations['score']

    feed_dict = self.get_all_zeros_input()
    # Feed "essential" inputs here (since Impala has several more)

    feed_dict[self._placeholders['0/0']] = [self._last_action]
    feed_dict[self._placeholders['0/1/observation/0']] = obs
    sticky_actions = ([observations['sticky_actions']])
    feed_dict[self._placeholders['0/1/observation/2']] = sticky_actions
    feed_dict[self._placeholders['0/1/observation/3']] = np.array(
        [score_reward], dtype=np.int32)
    if self._recurrent_state_size:
      feed_dict[self._placeholders['1/c']] = self._state[0]
      feed_dict[self._placeholders['1/h']] = self._state[1]

    out = self._sess.run(self._output, feed_dict=feed_dict)
    action_num = out['0/action'][0]
    self._last_action = action_num
    action = football_action_set.action_set_dict[self._action_set][action_num]
    if self._recurrent_state_size:
      self._state = (out['1/c'], out['1/h'])

    return action

  def reset(self):
    self._observation_processor = SimpleObservationProcessor(
        self._observation_type)
    self._score = [0, 0]
    if self._recurrent_state_size:
      self._state = (np.zeros((1, self._recurrent_state_size)),
                     np.zeros((1, self._recurrent_state_size)))
