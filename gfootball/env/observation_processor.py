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


"""Observation processor, providing multiple support methods for analyzing observations."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import collections
import datetime
from absl import logging
import os
import shutil
import tempfile
import timeit
import traceback

from gfootball.env import config as cfg
from gfootball.env import constants
from gfootball.env import football_action_set
import numpy as np
from six.moves import range
from six.moves import zip
import six.moves.cPickle

WRITE_FILES = True

try:
  import cv2
except ImportError:
  import cv2


class DumpConfig(object):

  def __init__(self,
               max_length=200,
               max_count=1,
               snapshot_delay=0,
               min_frequency=10):
    self._max_length = max_length
    self._max_count = max_count
    self._last_dump = 0
    self._snapshot_delay = snapshot_delay
    self._file_name = None
    self._result = None
    self._trigger_step = 0
    self._min_frequency = min_frequency


class TextWriter(object):

  def __init__(self, frame, x, y=0, field_coords=False, color=(255, 255, 255)):
    self._frame = frame
    if field_coords:
      x = 400 * (x + 1) - 5
      y = 695 * (y + 0.43)
    self._pos_x = int(x)
    self._pos_y = int(y) + 20
    self._color = color

  def write(self, text, scale_factor=1):
    font = cv2.FONT_HERSHEY_SIMPLEX
    textPos = (self._pos_x, self._pos_y)
    fontScale = 0.5 * scale_factor
    lineType = 1
    cv2.putText(self._frame, text, textPos, font, fontScale, self._color,
                lineType)
    self._pos_y += int(20 * scale_factor)


def get_frame(trace):
  if 'frame' in trace._trace['observation']:
    return trace._trace['observation']['frame']
  frame = np.uint8(np.zeros((600, 800, 3)))
  corner1 = (0, 0)
  corner2 = (799, 0)
  corner3 = (799, 599)
  corner4 = (0, 599)
  line_color = (0, 255, 255)
  cv2.line(frame, corner1, corner2, line_color)
  cv2.line(frame, corner2, corner3, line_color)
  cv2.line(frame, corner3, corner4, line_color)
  cv2.line(frame, corner4, corner1, line_color)
  cv2.line(frame, (399, 0), (399, 799), line_color)
  writer = TextWriter(
      frame,
      trace['ball'][0],
      trace['ball'][1],
      field_coords=True,
      color=(255, 0, 0))
  writer.write('B')
  for player_idx, player_coord in enumerate(trace['left_team']):
    writer = TextWriter(
        frame,
        player_coord[0],
        player_coord[1],
        field_coords=True,
        color=(0, 255, 0))
    letter = 'H'
    if 'active' in trace and player_idx in trace['active']:
      letter = 'X'
    elif 'left_agent_controlled_player' in trace and player_idx in trace[
        'left_agent_controlled_player']:
      letter = 'X'
    writer.write(letter)
  for player_idx, player_coord in enumerate(trace['right_team']):
    writer = TextWriter(
        frame,
        player_coord[0],
        player_coord[1],
        field_coords=True,
        color=(255, 255, 0))
    letter = 'A'
    if 'opponent_active' in trace and player_idx in trace['opponent_active']:
      letter = 'Y'
    elif 'right_agent_controlled_player' in trace and player_idx in trace[
        'right_agent_controlled_player']:
      letter = 'Y'
    writer.write(letter)
  return frame


def softmax(x):
  return np.exp(x) / np.sum(np.exp(x), axis=0)


def write_dump(name, trace, config):
  if len(trace) == 0:
    logging.warning('No data to write to the dump.')
    return False
  if config['write_video']:
    fd, temp_path = tempfile.mkstemp(suffix='.avi')
    if config['video_quality_level'] == 2:
      frame_dim = (1280, 720)
      fcc = cv2.VideoWriter_fourcc('p', 'n', 'g', ' ')
    elif config['video_quality_level'] == 1:
      fcc = cv2.VideoWriter_fourcc(*'MJPG')
      frame_dim = (1280, 720)
    else:
      fcc = cv2.VideoWriter_fourcc(*'XVID')
      frame_dim = (800, 450)
    video = cv2.VideoWriter(
        temp_path, fcc,
        constants.PHYSICS_STEPS_PER_SECOND / config['physics_steps_per_frame'],
        frame_dim)
    frame_cnt = 0
    time = trace[0]._time
    for o in trace:
      frame_cnt += 1
      frame = get_frame(o)
      frame = frame[..., ::-1]
      frame = cv2.resize(frame, frame_dim, interpolation=cv2.INTER_AREA)
      writer = TextWriter(frame, frame_dim[0] - 300)
      if config['custom_display_stats']:
        for line in config['custom_display_stats']:
          writer.write(line)
      if config['display_game_stats']:
        writer.write('SCORE: %d - %d' % (o['score'][0], o['score'][1]))
        writer.write('BALL OWNED TEAM: %d' % (o['ball_owned_team']))
        writer.write('BALL OWNED PLAYER: %d' % (o['ball_owned_player']))
        writer.write('REWARD %.4f' % (o['reward']))
        writer.write('CUM. REWARD: %.4f' % (o['cumulative_reward']))
        writer = TextWriter(frame, 0)
        writer.write('FRAME: %d' % frame_cnt)
        writer.write('TIME: %f' % (o._time - time))
        sticky_actions = football_action_set.get_sticky_actions(config)
        sticky_actions_field = 'left_agent_sticky_actions'
        if len(o[sticky_actions_field]) == 0:
          sticky_actions_field = 'right_agent_sticky_actions'
        assert len(sticky_actions) == len(o[sticky_actions_field][0])
        active_direction = None
        for i in range(len(sticky_actions)):
          if sticky_actions[i]._directional:
            if o[sticky_actions_field][0][i]:
              active_direction = sticky_actions[i]
          else:
            writer.write('%s: %d' % (sticky_actions[i]._name,
                                     o[sticky_actions_field][0][i]))
        writer.write('DIRECTION: %s' % ('NONE' if active_direction is None
                                        else active_direction._name))
        if 'action' in o._trace['debug']:
          writer.write('ACTION: %s' % (o['action'][0]._name))
        if 'baseline' in o._trace['debug']:
          writer.write('BASELINE: %.5f' % o._trace['debug']['baseline'])
        if 'logits' in o._trace['debug']:
          probs = softmax(o._trace['debug']['logits'])
          action_set = football_action_set.get_action_set(config)
          for action, prob in zip(action_set, probs):
            writer.write('%s: %.5f' % (action.name, prob), scale_factor=0.5)
        for d in o._debugs:
          writer.write(d)
      video.write(frame)
      for frame in o._additional_frames:
        frame = frame[..., ::-1]
        frame = cv2.resize(frame, frame_dim, interpolation=cv2.INTER_AREA)
        video.write(frame)
    video.release()
    os.close(fd)
    try:
      # For some reason sometimes the file is missing, so the code fails.
      if WRITE_FILES:
        shutil.copy2(temp_path, name + '.avi')
      logging.info('Video written to %s.avi', name)
      os.remove(temp_path)
    except:
      logging.error(traceback.format_exc())
  to_pickle = []
  temp_frames = []
  for o in trace:
    if 'frame' in o._trace['observation']:
      temp_frames.append(o._trace['observation']['frame'])
      del o._trace['observation']['frame']
    to_pickle.append(o._trace)
  assert len(temp_frames) == 0 or len(temp_frames) == len(trace)
  # Add config to the first frame for our replay tools to use.
  to_pickle[0]['debug']['config'] = config.get_dictionary()
  if WRITE_FILES:
    with open(name + '.dump', 'wb') as f:
      six.moves.cPickle.dump(to_pickle, f)
  if len(temp_frames):
    for o in trace:
      o._trace['observation']['frame'] = temp_frames.pop(0)
  logging.info('Dump written to %s.dump', name)
  return True


class ObservationState(object):

  def __init__(self, trace):
    # Observations
    self._trace = trace
    self._additional_frames = []
    self._debugs = []
    self._time = timeit.default_timer()

  def __getitem__(self, key):
    if key in self._trace:
      return self._trace[key]
    if key in self._trace['observation']:
      return self._trace['observation'][key]
    return self._trace['debug'][key]

  def __contains__(self, key):
    if key in self._trace:
      return True
    if key in self._trace['observation']:
      return True
    return key in self._trace['debug']

  def _distance(self, o1, o2):
    # We add 'z' dimension if not present, as ball has 3 dimensions, while
    # players have only 2.
    if len(o1) == 2:
      o1 = np.array([o1[0], o1[1], 0])
    if len(o2) == 2:
      o2 = np.array([o2[0], o2[1], 0])
    return np.linalg.norm(o1 - o2)

  def add_debug(self, text):
    self._debugs.append(text)

  def add_frame(self, frame):
    self._additional_frames.append(frame)


class ObservationProcessor(object):

  def __init__(self, config):
    # Const. configuration
    self._ball_takeover_epsilon = 0.03
    self._ball_lost_epsilon = 0.05
    self._trace_length = 10000 if config['dump_full_episodes'] else 200
    self._frame = 0
    self._dump_config = {}
    self._dump_config['score'] = DumpConfig(
        max_length=200,
        max_count=(100000 if config['dump_scores'] else 0),
        min_frequency=600,
        snapshot_delay=10)
    self._dump_config['lost_score'] = DumpConfig(
        max_length=200,
        max_count=(100000 if config['dump_scores'] else 0),
        min_frequency=600,
        snapshot_delay=10)
    self._dump_config['episode_done'] = DumpConfig(
        max_length=10000,
        max_count=(100000 if config['dump_full_episodes'] else 0))
    self._dump_config['shutdown'] = DumpConfig(max_length=10000)
    self._dump_directory = None
    self._config = config
    self.clear_state()

  def clear_state(self):
    self._frame = 0
    self._state = None
    self._trace = collections.deque([], self._trace_length)

  def __del__(self):
    self.process_pending_dumps(True)

  def reset(self):
    self.process_pending_dumps(True)
    self.clear_state()

  def len(self):
    return len(self._trace)

  def __getitem__(self, key):
    return self._trace[key]

  def add_frame(self, frame):
    if len(self._trace) > 0 and self._config['write_video']:
      self._trace[-1].add_frame(frame)

  def update(self, trace):
    self._frame += 1
    if not self._config['write_video'] and 'frame' in trace['observation']:
      # Don't record frame in the trace if we don't write video - full episode
      # consumes over 8G.
      no_video_trace = trace
      no_video_trace['observation'] = trace['observation'].copy()
      del no_video_trace['observation']['frame']
      self._state = ObservationState(no_video_trace)
    else:
      self._state = ObservationState(trace)
    self._trace.append(self._state)
    self.process_pending_dumps(False)
    return self._state

  def get_last_frame(self):
    if not self._state:
      return []
    return get_frame(self._state)

  def write_dump(self, name):
    if not name in self._dump_config:
      self._dump_config[name] = DumpConfig()
    config = self._dump_config[name]
    if config._file_name:
      logging.debug('Dump "%s": already pending', name)
      return
    if config._max_count <= 0:
      logging.debug('Dump "%s": count limit reached / disabled', name)
      return
    if config._last_dump >= timeit.default_timer() - config._min_frequency:
      logging.debug('Dump "%s": too frequent', name)
      return
    config._max_count -= 1
    config._last_dump = timeit.default_timer()
    if self._dump_directory is None:
      self._dump_directory = self._config['tracesdir']
      if WRITE_FILES:
        if not os.path.exists(self._dump_directory):
          os.makedirs(self._dump_directory)
    config._file_name = '{2}/{0}_{1}'.format(
        name,
        datetime.datetime.now().strftime('%Y%m%d-%H%M%S%f'),
        self._dump_directory)
    config._trigger_step = self._frame + config._snapshot_delay
    self.process_pending_dumps(True)
    return config._file_name

  def process_pending_dumps(self, finish):
    for name in self._dump_config:
      config = self._dump_config[name]
      if config._file_name:
        if finish or config._trigger_step <= self._frame:
          logging.debug('Start dump %s', name)
          trace = list(self._trace)[-config._max_length:]
          write_dump(config._file_name, trace, self._config)
          config._file_name = None
      if config._result:
        assert not config._file_name
        if config._result.ready() or finish:
          config._result.get()
          config._result = None
