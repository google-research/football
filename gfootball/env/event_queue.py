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

import pygame

KEYBOARD_EVENTS = [pygame.KEYDOWN]
GAMEPAD_EVENTS = [pygame.JOYBUTTONDOWN, pygame.JOYAXISMOTION]

_queue = []
_controllers = []


def add_controller(controller_kind, controller_index=None):
  global _controllers
  _controllers.append((controller_kind, controller_index))


def fits(event, controller_kind, controller_index):
  if controller_kind == 'keyboard':
    return event.type in KEYBOARD_EVENTS
  if controller_kind == 'gamepad':
    return event.type in GAMEPAD_EVENTS and event.joy == controller_index
  assert False, 'Unknown controller kind!'


def get(controller_kind, controller_index=None):
  global _queue
  global _controllers
  _queue.extend(pygame.event.get())
  result = []
  new_state = []
  for event in _queue:
    if fits(event, controller_kind, controller_index):
      result.append(event)
    else:
      for controller in _controllers:
        if fits(event, *controller):
          new_state.append(event)
          break
  _queue = new_state
  return result
