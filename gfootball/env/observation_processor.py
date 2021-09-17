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
import os
import shutil
import tempfile
import timeit
import traceback

from absl import logging
from gfootball.env import constants as const
from gfootball.env import football_action_set
from gfootball.scenarios import e_PlayerRole_GK
import numpy as np
from six.moves import range
from six.moves import zip
import six.moves.cPickle

# How many past frames are kept around for the dumps to make use of them.
PAST_STEPS_TRACE_SIZE = 100

WRITE_FILES = True

try:
  import cv2
except ImportError:
  import cv2


class DumpConfig(object):

  def __init__(self,
               max_count=1,
               steps_before=PAST_STEPS_TRACE_SIZE,
               steps_after=0,
               min_frequency=10):
    self._steps_before = steps_before
    self._steps_after = steps_after
    self._max_count = max_count
    # Make sure self._last_dump_time < timeit.default_timer() - min_frequency
    # holds upon startup.
    self._last_dump_time = timeit.default_timer() - 2 * min_frequency
    self._active_dump = None
    self._min_frequency = min_frequency


class TextWriter(object):

  def __init__(self, frame, x, y=0, field_coords=False, color=(255, 255, 255)):
    self._frame = frame
    if field_coords:
      x = 400 * (x + 1) - 10
      y = 695 * (y + 0.43)
    self._pos_x = int(x)
    self._pos_y = int(y) + 20
    self._color = color
    self._font = cv2.FONT_HERSHEY_SIMPLEX
    self._lineType = 2
    self._arrow_types = ('top', 'top_right', 'right', 'bottom_right', 'bottom',
                         'bottom_left', 'left', 'top_left')

  def write(self, text, scale_factor=1, color=None):
    textPos = (self._pos_x, self._pos_y)
    fontScale = 0.8 * scale_factor
    cv2.putText(self._frame, text, textPos, self._font, fontScale, color or self._color,
                self._lineType)
    self._pos_y += int(25 * scale_factor)

  def write_table(self, data, widths, scale_factor=1, offset=0):
    # data is a list of rows. Each row is a list of strings.
    fontScale = 0.5 * scale_factor
    init_x = self._pos_x
    for row in data:
      assert (len(row) == len(widths))
      self._pos_x += offset
      for col, cell in enumerate(row):
        color = self._color
        if isinstance(cell, tuple):
          assert (len(cell) == 2)
          (text, color) = cell
        else:
          assert (isinstance(cell, str))
          text = cell

        if text in self._arrow_types:
          self.write_arrow(text, scale_factor=scale_factor)
        else:
          textPos = (self._pos_x, self._pos_y)
          cv2.putText(self._frame, text, textPos, self._font, fontScale, color,
                      self._lineType)
        self._pos_x += widths[col]
      self._pos_x = init_x
      self._pos_y += int(20 * scale_factor)
    self._pos_x = init_x

  def write_arrow(self, arrow_type, scale_factor=1):
    assert (arrow_type in self._arrow_types)
    thickness = 1
    arrow_offsets = {
        'top': (12, 0, 12, -16),
        'top_right': (4, -4, 16, -16),
        'right': (0, -10, 20, -10),
        'bottom_right': (4, -16, 16, -4),
        'bottom': (10, -16, 10, 0),
        'bottom_left': (12, -12, 0, 0),
        'left': (20, -10, 0, -10),
        'top_left': (16, -4, 4, -16)
    }
    (s_x, s_y, e_x,
     e_y) = tuple(int(v * scale_factor) for v in arrow_offsets[arrow_type])
    start_point = (self._pos_x + s_x, self._pos_y + s_y)
    end_point = (self._pos_x + e_x, self._pos_y + e_y)
    image = cv2.arrowedLine(self._frame, start_point, end_point, self._color,
                            thickness)


def write_players_state(writer, players_info):
  table_text = [["PLAYER", "SPRINT", "DRIBBLE", "DIRECTION", "ACTION"]]
  widths = [65, 65, 70, 85, 85]

  # Sort the players according to the order they appear in observations
  for _, player_info in sorted(players_info.items()):
    table_text.append([
      (player_info['id'], player_info['color']),
      str(player_info.get("sprint", "-")),
      str(player_info.get("dribble", "-")),
      player_info.get("DIRECTION", "O"),
      player_info.get("ACTION", "-")])
  writer.write_table(table_text, widths, scale_factor=1.0, offset=10)


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
      color=(248, 244, 236))
  writer.write('B')
  for player_idx, player_coord in enumerate(trace['left_team']):
    writer = TextWriter(
        frame,
        player_coord[0],
        player_coord[1],
        field_coords=True,
        color=(238, 68, 47))
    letter = str(player_idx)
    if trace['left_team_roles'][player_idx] == e_PlayerRole_GK:
      letter = 'G'
    writer.write(letter)
  for player_idx, player_coord in enumerate(trace['right_team']):
    writer = TextWriter(
        frame,
        player_coord[0],
        player_coord[1],
        field_coords=True,
        color=(99, 172, 190))
    letter = str(player_idx)
    if trace['right_team_roles'][player_idx] == e_PlayerRole_GK:
      letter = 'G'
    writer.write(letter)
  return frame


def softmax(x):
  return np.exp(x) / np.sum(np.exp(x), axis=0)


class ActiveDump(object):

  def __init__(self,
               name,
               finish_step,
               config):
    self._name = name
    self._finish_step = finish_step
    self._config = config
    self._video_fd = None
    self._video_tmp = None
    self._video_writer = None
    self._frame_dim = None
    self._step_cnt = 0
    self._dump_file = None
    if config['write_video']:
      video_format = config['video_format']
      assert video_format in ['avi', 'webm']
      self._video_suffix = '.%s' % video_format
      self._video_fd, self._video_tmp = tempfile.mkstemp(
          suffix=self._video_suffix)
      self._frame_dim = (
          config['render_resolution_x'], config['render_resolution_y'])
      if config['video_quality_level'] not in [1, 2]:
        # Reduce resolution to (800, 450).
        self._frame_dim = min(self._frame_dim, (800, 450))
      if video_format == 'avi':
        if config['video_quality_level'] == 2:
          fcc = cv2.VideoWriter_fourcc('p', 'n', 'g', ' ')
        elif config['video_quality_level'] == 1:
          fcc = cv2.VideoWriter_fourcc(*'MJPG')
        else:
          fcc = cv2.VideoWriter_fourcc(*'XVID')
      else:
        fcc = cv2.VideoWriter_fourcc(*'vp80')

      self._video_writer = cv2.VideoWriter(
          self._video_tmp, fcc,
          const.PHYSICS_STEPS_PER_SECOND / config['physics_steps_per_frame'],
          self._frame_dim)
    if WRITE_FILES:
      self._dump_file = open(name + '.dump', 'wb')

  def __del__(self):
    self.finalize()

  def add_frame(self, frame):
    if self._video_writer:
      frame = frame[..., ::-1]
      frame = cv2.resize(frame, self._frame_dim, interpolation=cv2.INTER_AREA)
      self._video_writer.write(frame)

  def add_step(self, o):
    # Write video if requested.
    if self._video_writer:
      frame = get_frame(o)
      frame = frame[..., ::-1]
      frame = cv2.resize(frame, self._frame_dim, interpolation=cv2.INTER_AREA)
      writer = TextWriter(frame, self._frame_dim[0] - 300)
      if self._config['custom_display_stats']:
        for line in self._config['custom_display_stats']:
          writer.write(line)
      if self._config['display_game_stats']:
        writer.write('SCORE: %d - %d' % (o['score'][0], o['score'][1]))
        if o['ball_owned_team'] == 0:
          player = 'G' if o['left_team_roles'][
              o['ball_owned_player']] == e_PlayerRole_GK else o[
                  'ball_owned_player']
          writer.write('BALL OWNED: %s' % player, color=(47, 68, 238))
        elif o['ball_owned_team'] == 1:
          player = 'G' if o['right_team_roles'][
              o['ball_owned_player']] == e_PlayerRole_GK else o[
                  'ball_owned_player']
          writer.write('BALL OWNED: %s' % player, color=(190, 172, 99))
        else:
          writer.write('BALL OWNED: ---')
        writer = TextWriter(frame, 0)
        writer.write('STEP: %d' % self._step_cnt)
        sticky_actions = football_action_set.get_sticky_actions(self._config)

        players_info = {}
        for team in ['left', 'right']:
          player_info = {}
          sticky_actions_field = '%s_agent_sticky_actions' % team
          for player in range(len(o[sticky_actions_field])):
            assert len(sticky_actions) == len(o[sticky_actions_field][player])
            player_idx = o['%s_agent_controlled_player' % team][player]
            player_info = {}
            player_info['color'] = (
                47, 68, 238) if team == 'left' else (190, 172, 99)
            player_info['id'] = 'G' if o[
                '%s_team_roles' %
                team][player_idx] == e_PlayerRole_GK else str(player_idx)
            active_direction = None
            for i in range(len(sticky_actions)):
              if sticky_actions[i]._directional:
                if o[sticky_actions_field][player][i]:
                  active_direction = sticky_actions[i]
              else:
                player_info[sticky_actions[i]._name] = \
                    o[sticky_actions_field][player][i]

            # Info about direction
            player_info['DIRECTION'] = \
                'O' if active_direction is None else active_direction._name
            if 'action' in o._trace['debug']:
              # Info about action
              player_info['ACTION'] = \
                  o['action'][len(players_info)]._name
            players_info[(team, player_idx)] = player_info

        write_players_state(writer, players_info)

        if 'baseline' in o._trace['debug']:
          writer.write('BASELINE: %.5f' % o._trace['debug']['baseline'])
        if 'logits' in o._trace['debug']:
          probs = softmax(o._trace['debug']['logits'])
          action_set = football_action_set.get_action_set(self._config)
          for action, prob in zip(action_set, probs):
            writer.write('%s: %.5f' % (action.name, prob), scale_factor=0.5)
        for d in o._debugs:
          writer.write(d)
      self._video_writer.write(frame)
    # Write the dump.
    temp_frame = None
    if 'frame' in o._trace['observation']:
      temp_frame = o._trace['observation']['frame']
      del o._trace['observation']['frame']

    # Add config to the first frame for our replay tools to use.
    if self._step_cnt == 0:
      o['debug']['config'] = self._config.get_dictionary()

    six.moves.cPickle.dump(o._trace, self._dump_file)
    if temp_frame is not None:
      o._trace['observation']['frame'] = temp_frame
    self._step_cnt += 1

  def finalize(self):
    dump_info = {}
    if self._video_writer:
      self._video_writer.release()
      self._video_writer = None
      os.close(self._video_fd)
      try:
        # For some reason sometimes the file is missing, so the code fails.
        if WRITE_FILES:
          shutil.move(self._video_tmp, self._name + self._video_suffix)
        dump_info['video'] = '%s%s' % (self._name, self._video_suffix)
        logging.info('Video written to %s%s', self._name, self._video_suffix)
      except:
        logging.error(traceback.format_exc())
    if self._dump_file:
      self._dump_file.close()
      self._dump_file = None
      if self._step_cnt == 0:
        logging.warning('No data to write to the dump.')
      else:
        dump_info['dump'] = '%s.dump' % self._name
        logging.info('Dump written to %s.dump', self._name)
    return dump_info


class ObservationState(object):

  def __init__(self, trace):
    # Observations
    self._trace = trace
    self._additional_frames = []
    self._debugs = []

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
    self._frame = 0
    self._dump_config = {}
    self._dump_config['score'] = DumpConfig(
        steps_before=PAST_STEPS_TRACE_SIZE,
        max_count=(100000 if config['dump_scores'] else 0),
        min_frequency=600,
        steps_after=1)
    self._dump_config['lost_score'] = DumpConfig(
        steps_before=PAST_STEPS_TRACE_SIZE,
        max_count=(100000 if config['dump_scores'] else 0),
        min_frequency=600,
        steps_after=1)
    self._dump_config['episode_done'] = DumpConfig(
        steps_before=0,
        # Record entire episode.
        steps_after=10000,
        max_count=(100000 if config['dump_full_episodes'] else 0))
    self._dump_config['shutdown'] = DumpConfig(steps_before=PAST_STEPS_TRACE_SIZE)
    self._dump_directory = None
    self._config = config
    self.clear_state()

  def clear_state(self):
    self._frame = 0
    self._state = None
    self._trace = collections.deque([], PAST_STEPS_TRACE_SIZE)

  def reset(self):
    self.clear_state()

  def len(self):
    return len(self._trace)

  def __getitem__(self, key):
    return self._trace[key]

  def add_frame(self, frame):
    if len(self._trace) > 0 and self._config['write_video']:
      self._trace[-1].add_frame(frame)
      for dump in self.pending_dumps():
        dump.add_frame(frame)

  def update(self, trace):
    self._frame += 1
    frame = trace.get('frame', None)
    if not self._config['write_video'] and 'frame' in trace['observation']:
      # Don't record frame in the trace if we don't write video to save memory.
      no_video_trace = trace
      no_video_trace['observation'] = trace['observation'].copy()
      del no_video_trace['observation']['frame']
      self._state = ObservationState(no_video_trace)
      frame = None
    else:
      self._state = ObservationState(trace)
    self._trace.append(self._state)
    for dump in self.pending_dumps():
      dump.add_step(self._state)

  def get_last_frame(self):
    if not self._state:
      return []
    return get_frame(self._state)

  def write_dump(self, name):
    if not name in self._dump_config:
      self._dump_config[name] = DumpConfig()
    config = self._dump_config[name]
    if config._active_dump:
      logging.debug('Dump "%s": already pending', name)
      return
    if config._max_count <= 0:
      logging.debug('Dump "%s": count limit reached / disabled', name)
      return
    if config._last_dump_time > timeit.default_timer() - config._min_frequency:
      logging.debug('Dump "%s": too frequent', name)
      return
    config._max_count -= 1
    config._last_dump_time = timeit.default_timer()
    if self._dump_directory is None:
      self._dump_directory = self._config['tracesdir']
      if WRITE_FILES:
        if not os.path.exists(self._dump_directory):
          os.makedirs(self._dump_directory)
    dump_name = '{2}{3}{0}_{1}'.format(name,
        datetime.datetime.now().strftime('%Y%m%d-%H%M%S%f'),
        self._dump_directory, os.sep)
    config._active_dump = ActiveDump(dump_name,
        self._frame + config._steps_after, self._config)
    for step in list(self._trace)[-config._steps_before:]:
      config._active_dump.add_step(step)
      for frame in step._additional_frames:
        config._active_dump.add_frame(frame)
    if config._steps_after == 0:
      # Synchronously finalize dump, so that crash dump is recorded.
      config._active_dump.finalize()
      config._active_dump = None
    return dump_name

  def pending_dumps(self):
    dumps = []
    for config in self._dump_config.values():
      if config._active_dump:
        dumps.append(config._active_dump)
    return dumps

  def process_pending_dumps(self, episode_done=False):
    dumps = []
    for name in self._dump_config:
      config = self._dump_config[name]
      if config._active_dump and (
          episode_done or config._active_dump._finish_step <= self._frame):
        dump_info = config._active_dump.finalize()
        dump_info['name'] = name
        dumps.append(dump_info)
        config._active_dump = None
    return dumps
