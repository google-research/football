// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ai_keyboard.hpp"

AIControlledKeyboard::AIControlledKeyboard() {
  DO_VALIDATION;
  Reset();
}

bool AIControlledKeyboard::GetButton(e_ButtonFunction buttonFunction) {
  return buttons_pressed_[buttonFunction];
}

void AIControlledKeyboard::ResetNotSticky() {
  DO_VALIDATION;
  buttons_pressed_[e_ButtonFunction_LongPass] = false;
  buttons_pressed_[e_ButtonFunction_HighPass] = false;
  buttons_pressed_[e_ButtonFunction_ShortPass] = false;
  buttons_pressed_[e_ButtonFunction_Shot] = false;
  buttons_pressed_[e_ButtonFunction_Sliding] = false;
  buttons_pressed_[e_ButtonFunction_Switch] = false;
}

void AIControlledKeyboard::SetButton(e_ButtonFunction buttonFunction,
                                     bool state) {
  DO_VALIDATION;
  buttons_pressed_[buttonFunction] = state;
}

bool AIControlledKeyboard::GetPreviousButtonState(
    e_ButtonFunction buttonFunction) {
  DO_VALIDATION;
  return false;
}

Vector3 AIControlledKeyboard::GetDirection() {
  DO_VALIDATION;
  return direction_ * mirror;
}

Vector3 AIControlledKeyboard::GetOriginalDirection() { return direction_; }

void AIControlledKeyboard::SetDirection(const Vector3& new_direction) {
  direction_ = new_direction;
}

void AIControlledKeyboard::Reset() {
  DO_VALIDATION;
  direction_ = Vector3(0, 0, 0);
  memset(buttons_pressed_, 0, sizeof(buttons_pressed_));
}

void AIControlledKeyboard::ProcessState(EnvState* state) {
  DO_VALIDATION;
  state->setValidate(false);
  state->process(mirror);
  state->process(direction_);
  state->setValidate(true);
  state->process((void*)buttons_pressed_, sizeof(buttons_pressed_));
}

void AIControlledKeyboard::Mirror(float mirror) {
  DO_VALIDATION;
  this->mirror = mirror;
}
