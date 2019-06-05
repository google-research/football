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

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "capturekey.hpp"

#include "../windowmanager.hpp"

namespace blunted {


  // keyboard

  Gui2CaptureKey::Gui2CaptureKey(Gui2WindowManager *windowManager, const std::string &name) : Gui2View(windowManager, name, 0, 0, 0, 0) {
    isSelectable = true;
    keyID = -1;
  }

  Gui2CaptureKey::~Gui2CaptureKey() {
  }

  int Gui2CaptureKey::GetKeyID() {
    return keyID;
  }

  void Gui2CaptureKey::ProcessKeyboardEvent(KeyboardEvent *event) {
    if (event->GetKeyOnce(SDLK_ESCAPE) == true) {
      event->Ignore();
    } else {
      for (auto ev : event->GetKeyOnce()) {
        keyID = ev;
        sig_OnKey(this);
        break;
      }
    }
  }


  // joystick

  Gui2CaptureJoy::Gui2CaptureJoy(Gui2WindowManager *windowManager, const std::string &name, int controllerID) : Gui2View(windowManager, name, 0, 0, 0, 0), controllerID(controllerID) {
    isSelectable = true;

    inputType = e_JoystickInputType_Button;
    buttonID = -1;
    axisID = -1;
    axisSign = 0;
    tmpAxisID = -1;
    tmpAxisSign = 0;
  }

  Gui2CaptureJoy::~Gui2CaptureJoy() {
  }

  int Gui2CaptureJoy::GetButtonID() {
    return buttonID;
  }

  int Gui2CaptureJoy::GetAxisID() {
    return axisID;
  }

  signed int Gui2CaptureJoy::GetAxisSign() {
    return axisSign;
  }

  void Gui2CaptureJoy::ProcessKeyboardEvent(KeyboardEvent *event) {
    if (event->GetKeyOnce(SDLK_ESCAPE) == true) {
      sig_OnJoy(this); // cancel!
    }
  }

  void Gui2CaptureJoy::ProcessJoystickEvent(JoystickEvent *event) {
    for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
      if (event->GetButton(controllerID, i) == true) {
        inputType = e_JoystickInputType_Button;
        buttonID = i;
        sig_OnJoy(this);
        return;
      }
    }
    if (tmpAxisID == -1) {
      for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
        if (fabs(event->GetAxis(controllerID, i)) > 0.7) {
          tmpAxisID = i;
          tmpAxisSign = event->GetAxis(controllerID, i) < 0.0 ? -1 : 1;
          break;
        }
      }
    } else if (event->GetAxis(controllerID, tmpAxisID) == 0.0) {
      inputType = e_JoystickInputType_Axis;
      axisID = tmpAxisID;
      axisSign = tmpAxisSign;
      sig_OnJoy(this);
      return;
    }

  }

}
