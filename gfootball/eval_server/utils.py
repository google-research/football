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

"""General utils."""


import random
import string
import time
import urllib.request
from gfootball.eval_server import config
import grpc


def get_random_string(length=10, append_timestamp=True):
  characters = string.ascii_lowercase + string.ascii_uppercase + string.digits
  res = ''.join(random.choice(characters) for i in range(length))
  if append_timestamp:
    res += '_{}'.format(int(time.time()))
  return res


def get_grpc_channel(server):
  # send keepalive ping every 10 second
  # allow unlimited amount of keepalive pings
  options = (('grpc.keepalive_time_ms', 10000),
             ('grpc.http2.max_pings_without_data', 0))
  return grpc.insecure_channel(server, options=options)


def get_master_address(track='default'):
  # We add ''?' + get_random_string()' to avoid caching. Somehow even with
  # disabled server side caching on the file we were still getting the old
  # content of the file.
  response = urllib.request.urlopen(
      config.master_address_public_path + '_' + track + '?' +
      get_random_string())
  ip = response.read().decode('utf-8').strip()
  return '{}:{}'.format(ip, config.grpc_port)
