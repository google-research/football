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

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "keyboard.hpp"

#include "../managers/usereventmanager.hpp"

#include "../main.hpp"

HIDKeyboard::HIDKeyboard() {

  deviceType = e_HIDeviceType_Keyboard;
  identifier = "keyboard";

  LoadConfig();
}

HIDKeyboard::~HIDKeyboard() {
}

void HIDKeyboard::LoadConfig() {
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    functionButtonState[i] = false;
    previousFunctionButtonState[i] = false;

    functionMapping[i] = (SDL_Keycode)GetConfiguration()->GetInt(("input_keyboard_" + int_to_str(i)).c_str(), (SDL_Keycode)defaultKeyIDs[i]);
  }
}

void HIDKeyboard::SaveConfig() {
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    GetConfiguration()->SetInt(("input_keyboard_" + int_to_str(i)).c_str(), functionMapping[i]);
  }
  GetConfiguration()->SaveFile(GetConfigFilename());
}

void HIDKeyboard::Process() {
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    previousFunctionButtonState[i] = functionButtonState[i];
    functionButtonState[i] = UserEventManager::GetInstance().GetKeyboardState(functionMapping[i]);
  }
}

bool HIDKeyboard::GetButton(e_ButtonFunction buttonFunction) {
  return functionButtonState[buttonFunction];
}

float HIDKeyboard::GetButtonValue(e_ButtonFunction buttonFunction) {
  if (functionButtonState[buttonFunction]) return 1.0; else return 0.0;
}

void HIDKeyboard::SetButton(e_ButtonFunction buttonFunction, bool state) {
  functionButtonState[buttonFunction] = state;
}

bool HIDKeyboard::GetPreviousButtonState(e_ButtonFunction buttonFunction) {
  return previousFunctionButtonState[buttonFunction];
}

Vector3 HIDKeyboard::GetDirection() {
  Vector3 inputDirection;
  if (GetButton(e_ButtonFunction_Left))  inputDirection.coords[0] -= 1;
  if (GetButton(e_ButtonFunction_Right)) inputDirection.coords[0] += 1;
  if (GetButton(e_ButtonFunction_Up))    inputDirection.coords[1] += 1;
  if (GetButton(e_ButtonFunction_Down))  inputDirection.coords[1] -= 1;
  inputDirection.Normalize(0);
  return inputDirection;
}


