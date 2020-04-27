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


"""Remote football environment."""
import pickle
import time
from absl import logging
import cv2
from gfootball.env import football_action_set
from gfootball.eval_server import config
from gfootball.eval_server import utils
from gfootball.eval_server.proto import game_server_pb2
from gfootball.eval_server.proto import game_server_pb2_grpc
from gfootball.eval_server.proto import master_pb2
from gfootball.eval_server.proto import master_pb2_grpc
import grpc
import gym
import numpy as np


CONNECTION_TRIALS = 20


# This config is needed in order to run various env wrappers.
class FakeConfig(object):
  """An immitation of Config with the set of fields necessary to run wrappers.
  """

  def __init__(self, num_players):
    self._num_players = num_players
    self._values = {
    }

  def number_of_players_agent_controls(self):
    return self._num_players

  def __getitem__(self, key):
    return self._values[key]


class RemoteFootballEnv(gym.Env):

  def __init__(self, username, token, model_name='', track='default',
               include_rendering=False):
    self._config = FakeConfig(
        config.track_to_num_controlled_players.get(track, 1))
    self._num_actions = len(football_action_set.action_set_dict['default'])
    self._track = track
    self._include_rendering = include_rendering

    self._username = username
    self._token = token
    self._model_name = model_name

    self._game_id = None
    self._channel = None

    self._update_master()

  def _update_master(self):
    while True:
      try:
        master_address = utils.get_master_address(self._track)
        logging.info('Connecting to %s', master_address)
        self._master_channel = utils.get_grpc_channel(master_address)
        grpc.channel_ready_future(self._master_channel).result(timeout=10)
        break
      except grpc.FutureTimeoutError:
        logging.info('Failed to connect to master')
      except BaseException as e:
        logging.info('Error %s, sleeping 10 secs', e)
        time.sleep(10)
    logging.info('Connection successful')

  @property
  def action_space(self):
    return gym.spaces.Discrete(self._num_actions)

  def step(self, action):
    if self._game_id is None:
      raise RuntimeError('Environment should be reset!')
    if np.isscalar(action):
      action = [int(action)]
    request = game_server_pb2.StepRequest(
        game_version=config.game_version, game_id=self._game_id,
        username=self._username, token=self._token, action=-1,
        model_name=self._model_name, action_list=action)
    return self._get_env_result(request, 'Step')

  def reset(self):
    if self._channel is not None:
      # Client surrenders in the current game and starts next one.

      self._channel.close()
      self._channel = None

    # Get game server address and side id from master.
    start_game_request = master_pb2.StartGameRequest(
        game_version=config.game_version, username=self._username,
        token=self._token, model_name=self._model_name,
        include_rendering=self._include_rendering)
    response = self._reset_with_retries(start_game_request)
    self._game_id = response.game_id
    self._channel = utils.get_grpc_channel(response.game_server_address)
    grpc.channel_ready_future(self._channel).result()
    get_env_result_request = game_server_pb2.GetEnvResultRequest(
        game_version=config.game_version, game_id=self._game_id,
        username=self._username, token=self._token, model_name=self._model_name)
    return self._get_env_result(get_env_result_request, 'GetEnvResult')[0]

  def _reset_with_retries(self, request):
    time_to_sleep = 1
    while True:
      try:
        stub = master_pb2_grpc.MasterStub(self._master_channel)
        return stub.StartGame(request, timeout=10*60)
      except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.DEADLINE_EXCEEDED:
          time_to_sleep = 1
          continue
        logging.warning('Exception during request: %s', e)
        logging.warning('Sleeping for %d seconds', time_to_sleep)
        time.sleep(time_to_sleep)
        if time_to_sleep < 30:
          time_to_sleep *= 2
        self._update_master()
      except BaseException as e:
        logging.warning('Exception during request: %s', e)
        logging.warning('Sleeping for %d seconds', time_to_sleep)
        time.sleep(time_to_sleep)
        if time_to_sleep < 30:
          time_to_sleep *= 2
        self._update_master()
    raise RuntimeError('Connection problems!')

  def _get_env_result(self, request, rpc_name):
    assert rpc_name in ['GetEnvResult', 'Step']

    response = None
    for _ in range(CONNECTION_TRIALS):
      time_to_sleep = 1
      try:
        stub = game_server_pb2_grpc.GameServerStub(self._channel)
        response = getattr(stub, rpc_name)(request)
        break
      except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.INVALID_ARGUMENT:
          raise e
        if e.code() == grpc.StatusCode.FAILED_PRECONDITION:
          raise e
        logging.warning('Exception during request: %s', e)
        time.sleep(time_to_sleep)
        if time_to_sleep < 30:
          time_to_sleep *= 2
      except BaseException as e:
        logging.warning('Exception during request: %s', e)
        time.sleep(time_to_sleep)
        if time_to_sleep < 30:
          time_to_sleep *= 2
    if response is None:
      raise RuntimeError('Connection problems!')

    env_result = pickle.loads(response.env_result)
    self._process_env_result(env_result)
    return env_result[0], env_result[1], env_result[2], env_result[3]

  def _process_env_result(self, env_result):
    ob, rew, done, info = env_result
    if self._include_rendering and 'frame' in info:
      cv2.imshow('GRF League', info['frame'])
      cv2.waitKey(1)
    if done:
      self._game_id = None
      self._channel.close()
      self._channel = None
