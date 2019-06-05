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

#include "gamepad.hpp"

#include "../managers/usereventmanager.hpp"
#include "../base/utils.hpp"

#include "../main.hpp"

HIDGamepad::HIDGamepad(int gamepadID) : gamepadID(gamepadID) {

  deviceType = e_HIDeviceType_Gamepad;
  identifier = std::string(SDL_JoystickNameForIndex(gamepadID)) + " #" + int_to_str(gamepadID);

  LoadConfig();
}

HIDGamepad::~HIDGamepad() {
}

void HIDGamepad::LoadConfig() {
  for (int i = 0; i < e_ControllerButton_Size; i++) {
    controllerButtonState[i] = false;
    previousControllerButtonState[i] = false;
  }

  for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
    float min = GetConfiguration()->GetReal(("input_gamepad_" + GetIdentifier() + "_calibration_" + int_to_str(i) + "_min").c_str(), -32768);
    float max = GetConfiguration()->GetReal(("input_gamepad_" + GetIdentifier() + "_calibration_" + int_to_str(i) + "_max").c_str(), 32767);
    float rest = GetConfiguration()->GetReal(("input_gamepad_" + GetIdentifier() + "_calibration_" + int_to_str(i) + "_rest").c_str(), 0);
    UserEventManager::GetInstance().SetJoystickAxisCalibration(GetGamepadID(), i, min, max, rest);
  }

  std::string gpbuttonIDs_string[14];
  for (int i = 0; i < e_ControllerButton_Size; i++) {

    // xbox controller defaults
    int defaultButton = 0;
    if      (i == 0) defaultButton = -3;
    else if (i == 1) defaultButton = -2;
    else if (i == 2) defaultButton = -4;
    else if (i == 3) defaultButton = -1;
    else if (i == 4) defaultButton = 3;
    else if (i == 5) defaultButton = 1;
    else if (i == 6) defaultButton = 0;
    else if (i == 7) defaultButton = 2;
    else if (i == 8) defaultButton = 4;
    else if (i == 9) defaultButton = -6;
    else if (i == 10) defaultButton = 5;
    else if (i == 11) defaultButton = -5;
    else if (i == 12) defaultButton = 6;
    else if (i == 13) defaultButton = 7;

    controllerMapping[i] = GetConfiguration()->GetInt(("input_gamepad_" + GetIdentifier() + "_" + int_to_str(i)).c_str(), defaultButton);
  }

  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    int defaultMapping = 0;
    if      (i == e_ButtonFunction_Up) defaultMapping = e_ControllerButton_Up;
    else if (i == e_ButtonFunction_Right) defaultMapping = e_ControllerButton_Right;
    else if (i == e_ButtonFunction_Down) defaultMapping = e_ControllerButton_Down;
    else if (i == e_ButtonFunction_Left) defaultMapping = e_ControllerButton_Left;
    else if (i == e_ButtonFunction_LongPass) defaultMapping = e_ControllerButton_Y;
    else if (i == e_ButtonFunction_HighPass) defaultMapping = e_ControllerButton_B;
    else if (i == e_ButtonFunction_ShortPass) defaultMapping = e_ControllerButton_A;
    else if (i == e_ButtonFunction_Shot) defaultMapping = e_ControllerButton_X;
    else if (i == e_ButtonFunction_KeeperRush) defaultMapping = e_ControllerButton_Y;
    else if (i == e_ButtonFunction_Sliding) defaultMapping = e_ControllerButton_B;
    else if (i == e_ButtonFunction_Pressure) defaultMapping = e_ControllerButton_A;
    else if (i == e_ButtonFunction_TeamPressure) defaultMapping = e_ControllerButton_X;
    else if (i == e_ButtonFunction_Switch) defaultMapping = e_ControllerButton_L1;
    else if (i == e_ButtonFunction_Special) defaultMapping = e_ControllerButton_L2;
    else if (i == e_ButtonFunction_Sprint) defaultMapping = e_ControllerButton_R1;
    else if (i == e_ButtonFunction_Dribble) defaultMapping = e_ControllerButton_R2;
    else if (i == e_ButtonFunction_Start) defaultMapping = e_ControllerButton_Start;
    else if (i == e_ButtonFunction_Select) defaultMapping = e_ControllerButton_Select;

    functionMapping[i] = (e_ControllerButton)GetConfiguration()->GetInt(("input_gamepad_" + GetIdentifier() + "_mapping_" + int_to_str(i)).c_str(), defaultMapping);
  }
}

void HIDGamepad::SaveConfig() {
  for (int i = 0; i < e_ControllerButton_Size; i++) {
    GetConfiguration()->Set(("input_gamepad_" + GetIdentifier() + "_" + int_to_str(i)).c_str(), controllerMapping[i]);
  }
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    GetConfiguration()->Set(("input_gamepad_" + GetIdentifier() + "_mapping_" + int_to_str(i)).c_str(), functionMapping[i]);
  }
  GetConfiguration()->SaveFile(GetConfigFilename());
}

void HIDGamepad::Process() {
  // printf("gamepad ID #%i\n", gamepadID);
  for (int i = 0; i < e_ControllerButton_Size; i++) {
    previousControllerButtonState[i] = controllerButtonState[i];
    signed int buttonID = controllerMapping[i];
    if (buttonID >= 0) { // button
      controllerButtonState[i] = UserEventManager::GetInstance().GetJoyButtonState(gamepadID, buttonID) ? 1.0 : 0.0;
    } else { // axis
      // decode
      int axisID = -buttonID - 1;
      signed int sign = ((axisID % 2) * 2) - 1;
      axisID /= 2;
      bool deadzone = true;
      if (axisID < 2) deadzone = false;
      float value = UserEventManager::GetInstance().GetJoystickAxis(gamepadID, axisID, deadzone);
      if ((sign < 0 && value < 0) || (sign > 0 && value > 0)) controllerButtonState[i] = fabs(value); else
                                                              controllerButtonState[i] = 0;
    }
  }
}

bool HIDGamepad::GetButton(e_ButtonFunction buttonFunction) {
  return controllerButtonState[functionMapping[buttonFunction]] > 0.0f;
}

float HIDGamepad::GetButtonValue(e_ButtonFunction buttonFunction) {
  return controllerButtonState[functionMapping[buttonFunction]];
}

void HIDGamepad::SetButton(e_ButtonFunction buttonFunction, bool state) {
  controllerButtonState[functionMapping[buttonFunction]] = state;
}

bool HIDGamepad::GetPreviousButtonState(e_ButtonFunction buttonFunction) {
  return previousControllerButtonState[functionMapping[buttonFunction]];
}

Vector3 HIDGamepad::GetDirection() {
  Vector3 inputDirection;
  inputDirection.coords[0] -= GetButtonValue(e_ButtonFunction_Left);
  inputDirection.coords[0] += GetButtonValue(e_ButtonFunction_Right);
  inputDirection.coords[1] += GetButtonValue(e_ButtonFunction_Up);
  inputDirection.coords[1] -= GetButtonValue(e_ButtonFunction_Down);
  if (inputDirection.GetLength() < analogStickDeadzone) {
    inputDirection = Vector3(0);
  } else {
    inputDirection.Normalize(0);
  }
  return inputDirection;
}
