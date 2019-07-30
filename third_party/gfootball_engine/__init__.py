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


from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import logging
import os
import sys



game_path = os.path.dirname(os.path.abspath(__file__))
if game_path not in sys.path:
  sys.path.append(game_path)

gfootball_dir = os.path.dirname(os.path.abspath(__file__))
font_file = os.path.join(gfootball_dir, 'fonts/AlegreyaSansSC-ExtraBold.ttf')
if 'GFOOTBALL_FONT' not in os.environ:
  os.environ['GFOOTBALL_FONT'] = font_file
data_dir = os.path.join(gfootball_dir, 'data')
if 'GFOOTBALL_DATA_DIR' not in os.environ:
  os.environ['GFOOTBALL_DATA_DIR'] = data_dir

try:
  from _gameplayfootball import *
except:
  if not (os.path.isfile(os.path.join(game_path, 'libgame.so')) and
          os.path.isfile(os.path.join(game_path, '_gameplayfootball.so'))):
    logging.warning('Looks like game engine is not compiled, please run:')
    engine_path = os.path.abspath(os.path.dirname(__file__))
    logging.warning(
        '  pushd {} && cmake . && make -j && popd'.format(game_path))
    logging.warning('  pushd {} && ln -s libgame.so '
                    '_gameplayfootball.so && popd'.format(engine_path))
  raise
