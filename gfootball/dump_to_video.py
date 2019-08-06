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

"""Script allowing to render a replay video from a game dump."""

from absl import app
from absl import flags
from gfootball.env import script_helpers

FLAGS = flags.FLAGS
flags.DEFINE_string('trace_file', None, 'Trace file to render')
flags.mark_flag_as_required('trace_file')


def main(_):
  script_helpers.ScriptHelpers().dump_to_video(FLAGS.trace_file)


if __name__ == '__main__':
  app.run(main)
