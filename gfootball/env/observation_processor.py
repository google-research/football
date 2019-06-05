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
import logging
import os
import tempfile
import timeit
import traceback

from gfootball.env import config as cfg
from gfootball.env import constants
from gfootball.env import football_action_set
import numpy as np
import scipy
from six.moves import range
from six.moves import zip
import six.moves.cPickle
import tensorflow as tf

try:
  import cv2
except ImportError:
  import cv2

HIGH_RES=False  # change to true for collecting replays

class DumpConfig(object):

  def __init__(self,
               max_length=200,
               max_count=1,
               skip_visuals=False,
               snapshot_delay=0,
               min_frequency=10):
    self._max_length = max_length
    self._max_count = max_count
    self._last_dump = 0
    self._skip_visuals = skip_visuals
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
    frame = trace._trace['observation']['frame']
  else:
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
    for player_idx, player_coord in enumerate(trace['home_team']):
      writer = TextWriter(
          frame,
          player_coord[0],
          player_coord[1],
          field_coords=True,
          color=(0, 255, 0))
      letter = 'H'
      if 'active' in trace and trace['active'] == player_idx:
        letter = 'X'
      elif 'home_agent_controlled_player' in trace and player_idx in trace[
          'home_agent_controlled_player']:
        letter = 'X'
      writer.write(letter)
    for player_idx, player_coord in enumerate(trace['away_team']):
      writer = TextWriter(
          frame,
          player_coord[0],
          player_coord[1],
          field_coords=True,
          color=(0, 0, 255))
      letter = 'A'
      if 'opponent_active' in trace and trace['opponent_active'] == player_idx:
        letter = 'Y'
      elif 'away_agent_controlled_player' in trace and player_idx in trace[
          'away_agent_controlled_player']:
        letter = 'Y'
      writer.write(letter)
  return frame


def softmax(x):
  return np.exp(x) / np.sum(np.exp(x), axis=0)


@cfg.log
def write_dump(name, trace, skip_visuals=False, config={}):
  if not skip_visuals:
    fd, temp_path = tempfile.mkstemp(suffix='.avi')
    if HIGH_RES:
      frame_dim = (1280, 720)
      fcc = cv2.VideoWriter_fourcc('p', 'n', 'g', ' ')
    else:
      fcc = cv2.VideoWriter_fourcc(*'XVID')
      frame_dim = (800, 600)
    video = cv2.VideoWriter(
        temp_path, fcc,
        constants.PHYSICS_STEPS_PER_SECOND / config['physics_steps_per_frame'],
        frame_dim)
    frame_cnt = 0
    if len(trace) > 0:
      time = trace[0]._time
    for o in trace:
      frame_cnt += 1
      frame = get_frame(o)
      frame = frame[..., ::-1]
      frame = cv2.resize(frame, frame_dim, interpolation=cv2.INTER_AREA)
      if config['display_game_stats']:
        writer = TextWriter(frame, 950 if HIGH_RES else 500)
        writer.write('SCORE: %d - %d' % (o._score[0], o._score[1]))
        writer.write('BALL OWNED TEAM: %d' % (o['ball_owned_team']))
        writer.write('BALL OWNED PLAYER: %d' % (o['ball_owned_player']))
        writer.write('REWARD %.4f' % (o['reward']))
        writer.write('CUM. REWARD: %.4f' % (o['cumulative_reward']))
        writer = TextWriter(frame, 0)
        writer.write('FRAME: %d' % frame_cnt)
        writer.write('TIME: %f' % (o._time - time))
        time = o._time
        writer.write('Pressure: %d' % o._pressed_pressure)
        writer.write('Keeper pressure: %d' % o._pressed_keeper_rush)
        writer.write('Team pressure: %d' % o._pressed_team_pressure)
        writer.write('Sprint: %d' % o._pressed_sprint)
        writer.write('Dribble: %d' % o._pressed_dribble)
        writer.write('DIRECTION: %s' % ('NONE' if o._pressed_direction is None
                                        else o._pressed_direction.name))
        if 'action' in o._trace['debug']:
          writer.write('ACTION: %s' % (o['action'][0].name))
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
      tf.gfile.Copy(temp_path, name + '.avi', overwrite=True)
      os.remove(temp_path)
    except:
      logging.info(traceback.format_exc())
  to_pickle = []
  temp_frames = []
  for o in trace:
    if 'frame' in o._trace['observation']:
      temp_frames.append(o._trace['observation']['frame'])
      o._trace['observation']['frame'] = 'removed'
    to_pickle.append(o._trace)
  with tf.gfile.Open(name + '.dump', 'wb') as f:
    six.moves.cPickle.dump(to_pickle, f)
  for o in trace:
    if 'frame' in o._trace['observation']:
      o._trace['observation']['frame'] = temp_frames.pop(0)
  logging.info('Dump written to %s.dump', name)
  if not skip_visuals:
    logging.info('Video written to %s.avi', name)
  return True


def logging_write_dump(name, trace, skip_visuals=False, config={}):
  try:
    write_dump(name, trace, skip_visuals=skip_visuals, config=config)
  except Exception as e:
    logging.info(traceback.format_exc())
    raise


class ObservationState(object):

  def __init__(self, trace):
    # Observations
    self._trace = trace
    self._additional_frames = []
    self._debugs = []
    self._time = timeit.default_timer()
    self._away_defence_max_x = -10
    self._score = [0, 0]
    self._pressed_direction = None
    self._pressed_X = False
    self._pressed_Y = False
    self._pressed_A = False
    self._pressed_B = False
    self._pressed_LB = False
    self._pressed_RB = False
    self._pressed_LT = False
    self._pressed_RT = False

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

  def debug(self):
    logging.info('SCORE: ', self._score)

  def _get_coordinates(self, o):
    """Retrieves coordinates of an object in question.

    Args:
      o: Object name for which to retrieve coordinates from observations.

    Returns:
      Objects's coordinates.
    """
    if isinstance(o, str):
      if o == 'active':
        o = self['home_team'][self['active']]
      else:
        o = self[o]
    if isinstance(o, list):
      o = np.array(o)
    return o

  def _distance(self, o1, o2):
    # We add 'z' dimension if not present, as ball has 3 dimensions, while
    # players have only 2.
    if len(o1) == 2:
      o1 = np.array([o1[0], o1[1], 0])
    if len(o2) == 2:
      o2 = np.array([o2[0], o2[1], 0])
    return np.linalg.norm(o1 - o2)

  def object_distance(self, object1, object2):
    o1 = self._get_coordinates(object1)
    o2 = self._get_coordinates(object2)
    if o1.ndim == 1 and o2.ndim == 1:
      return self._distance(o1, o2)
    if o1.ndim == 2 and o2.ndim == 1:
      return min([self._distance(o, o2) for o in o1])
    if o1.ndim == 1 and o2.ndim == 2:
      return min([self._distance(o, o1) for o in o2])
    assert 'Not supported operation'

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
        snapshot_delay=10,
        skip_visuals=not config['write_video'])
    self._dump_config['lost_score'] = DumpConfig(
        max_length=200,
        max_count=(100000 if config['dump_scores'] else 0),
        min_frequency=600,
        snapshot_delay=10,
        skip_visuals=not config['write_video'])
    self._dump_config['episode_done'] = DumpConfig(
        max_length=(200 if HIGH_RES else 10000),
        max_count=(100000 if config['dump_full_episodes'] else 0),
        skip_visuals=not config['write_video'])
    self._dump_config['shutdown'] = DumpConfig(
        max_length=(200 if HIGH_RES else 10000),
        skip_visuals=not config['write_video'])
    self._thread_pool = None
    self._dump_directory = None
    self._config = config
    self.clear_state()

  def clear_state(self):
    self._frame = 0
    self._state = None
    self._trace = collections.deque([], self._trace_length)

  def __del__(self):
    self.process_pending_dumps(True)
    if self._thread_pool:
      self._thread_pool.close()

  def reset(self):
    self.process_pending_dumps(True)
    self.clear_state()

  def len(self):
    return len(self._trace)

  def __getitem__(self, key):
    return self._trace[key]

  def add_frame(self, frame):
    if len(self._trace) > 0:
      self._trace[-1].add_frame(frame)

  @cfg.log
  def update(self, trace):
    self._frame += 1
    if not self._config['write_video'] and 'frame' in trace:
      # Don't record frame in the trace if we don't write video - full episode
      # consumes over 8G.
      no_video_trace = trace
      no_video_trace['observation'] = trace['observation'].copy()
      del no_video_trace['observation']['frame']
      self._state = ObservationState(no_video_trace)
    else:
      self._state = ObservationState(trace)
    self._trace.append(self._state)
    state = self._state
    sticky = self._state['home_agent_sticky_actions'][0]
    if sticky[0]:
      state._pressed_direction = football_action_set.core_action_left
    if sticky[1]:
      state._pressed_direction = football_action_set.core_action_top_left
    if sticky[2]:
      state._pressed_direction = football_action_set.core_action_top
    if sticky[3]:
      state._pressed_direction = football_action_set.core_action_top_right
    if sticky[4]:
      state._pressed_direction = football_action_set.core_action_right
    if sticky[5]:
      state._pressed_direction = football_action_set.core_action_bottom_right
    if sticky[6]:
      state._pressed_direction = football_action_set.core_action_bottom
    if sticky[7]:
      state._pressed_direction = football_action_set.core_action_bottom_left
    state._pressed_keeper_rush = sticky[8]
    state._pressed_pressure = sticky[9]
    state._pressed_team_pressure = sticky[10]
    state._pressed_sprint = sticky[11]
    state._pressed_dribble = sticky[12]
    self.process_pending_dumps(False)
    return self._state

  @cfg.log
  def write_dump(self, name):
    if not name in self._dump_config:
      self._dump_config[name] = DumpConfig()
    config = self._dump_config[name]
    if config._file_name:
      logging.info('Dump "%s": already pending', name)
      return
    if config._max_count <= 0:
      logging.info('Dump "%s": count limit reached / disabled', name)
      return
    if config._last_dump >= timeit.default_timer() - config._min_frequency:
      logging.info('Dump "%s": too frequent', name)
      return
    config._max_count -= 1
    config._last_dump = timeit.default_timer()
    if self._dump_directory is None:
      self._dump_directory = self._config['tracesdir']
      tf.gfile.MakeDirs(self._dump_directory)
    config._file_name = '{2}/{0}_{1}'.format(
        name,
        datetime.datetime.now().strftime('%Y%m%d-%H%M%S%f'),
        self._dump_directory)
    config._trigger_step = self._frame + config._snapshot_delay
    self.process_pending_dumps(True)
    return config._file_name

  @cfg.log
  def process_pending_dumps(self, finish):
    for name in self._dump_config:
      config = self._dump_config[name]
      if config._file_name:
        if finish or config._trigger_step <= self._frame:
          logging.info('Start dump %s', name)
          trace = list(self._trace)[-config._max_length:]
          write_dump(config._file_name, trace, config._skip_visuals,
                     self._config)
          config._file_name = None
      if config._result:
        assert not config._file_name
        if config._result.ready() or finish:
          config._result.get()
          config._result = None
