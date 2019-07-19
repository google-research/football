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
   python dump_to_txt.py --dump=/tmp/input.dump --output=/tmp/output.txt

"""



from absl import app
from absl import flags

import six.moves.cPickle

FLAGS = flags.FLAGS

flags.DEFINE_string('dump', '', 'Trace file to convert')
flags.DEFINE_string('output', '', 'Output txt file')
flags.DEFINE_bool('include_debug', True,
                  'Include debug information for each step')


def main(_):
  with open(FLAGS.dump, 'rb') as f:
    replay = six.moves.cPickle.load(f)
  if not FLAGS.include_debug:
    for s in replay:
      if 'debug' in s:
        del s['debug']
  with open(FLAGS.output, 'w') as f:
    f.write(str(replay))


if __name__ == '__main__':
  app.run(main)
