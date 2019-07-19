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
  deviceType = e_HIDeviceType_Keyboard;
  identifier = "AIkeyboard";
}

void AIControlledKeyboard::Process() {

}

bool AIControlledKeyboard::GetButton(e_ButtonFunction buttonFunction) {
  return buttons_pressed_.find(buttonFunction) != buttons_pressed_.end();
}

void AIControlledKeyboard::SetButton(e_ButtonFunction buttonFunction, bool state) {
  if (state) {
    buttons_pressed_.insert(buttonFunction);
  } else {
    buttons_pressed_.erase(buttonFunction);
  }
}

bool AIControlledKeyboard::GetPreviousButtonState(e_ButtonFunction buttonFunction) {
  return false;
}

Vector3 AIControlledKeyboard::GetDirection() {
  return direction_;
}

void AIControlledKeyboard::SetDirection(const Vector3& new_direction) {
  direction_ = new_direction;
}

void AIControlledKeyboard::Reset() {
  direction_ = Vector3(0, 0, 0);
  buttons_pressed_.clear();
}

