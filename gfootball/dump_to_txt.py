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


"""Script converting dump file to human readable format.

   Example usage:
   python dump_to_txt.py --trace_file=/tmp/input.dump --output=/tmp/output.txt

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl import app
from absl import flags
from gfootball.env import script_helpers

FLAGS = flags.FLAGS

flags.DEFINE_string('trace_file', None, 'Trace file to convert')
flags.DEFINE_string('output', None, 'Output txt file')
flags.DEFINE_bool('include_debug', True,
                  'Include debug information for each step')
flags.mark_flag_as_required('trace_file')
flags.mark_flag_as_required('output')


def main(_):
  script_helpers.ScriptHelpers().dump_to_txt(FLAGS.trace_file, FLAGS.output,
                                             FLAGS.include_debug)


if __name__ == '__main__':
  app.run(main)
