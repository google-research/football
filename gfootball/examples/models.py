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

"""Additional models, not available in OpenAI baselines.

gfootball_impala_cnn is architecture used in the paper
(https://arxiv.org/pdf/1907.11180.pdf).
It is illustrated in the appendix. It is similar to Large architecture
from IMPALA paper; we use 4 big blocks instead of 3 though.
"""
from baselines.common.models import register
import sonnet as snt
import tensorflow.compat.v1 as tf


@register('gfootball_impala_cnn')
def gfootball_impala_cnn():
  def network_fn(frame):
    # Convert to floats.
    frame = tf.to_float(frame)
    frame /= 255
    with tf.variable_scope('convnet'):
      conv_out = frame
      conv_layers = [(16, 2), (32, 2), (32, 2), (32, 2)]
      for i, (num_ch, num_blocks) in enumerate(conv_layers):
        # Downscale.
        conv_out = snt.Conv2D(num_ch, 3, stride=1, padding='SAME')(conv_out)
        conv_out = tf.nn.pool(
            conv_out,
            window_shape=[3, 3],
            pooling_type='MAX',
            padding='SAME',
            strides=[2, 2])

        # Residual block(s).
        for j in range(num_blocks):
          with tf.variable_scope('residual_%d_%d' % (i, j)):
            block_input = conv_out
            conv_out = tf.nn.relu(conv_out)
            conv_out = snt.Conv2D(num_ch, 3, stride=1, padding='SAME')(conv_out)
            conv_out = tf.nn.relu(conv_out)
            conv_out = snt.Conv2D(num_ch, 3, stride=1, padding='SAME')(conv_out)
            conv_out += block_input

    conv_out = tf.nn.relu(conv_out)
    conv_out = snt.BatchFlatten()(conv_out)

    conv_out = snt.Linear(256)(conv_out)
    conv_out = tf.nn.relu(conv_out)

    return conv_out

  return network_fn
