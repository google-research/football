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

#ifndef _AI_KEYBOARD
#define _AI_KEYBOARD

#include "../base/math/vector3.hpp"
#include "../defines.hpp"
#include "../hid/ihidevice.hpp"
#include <set>


enum e_ButtonFunction {
  e_ButtonFunction_LongPass,
  e_ButtonFunction_HighPass,
  e_ButtonFunction_ShortPass,
  e_ButtonFunction_Shot,
  e_ButtonFunction_KeeperRush,
  e_ButtonFunction_Sliding,
  e_ButtonFunction_Pressure,
  e_ButtonFunction_TeamPressure,
  e_ButtonFunction_Switch,
  e_ButtonFunction_Sprint,
  e_ButtonFunction_Dribble,
  e_ButtonFunction_Size
};

class AIControlledKeyboard {

  public:
    AIControlledKeyboard(e_PlayerColor color);
    bool GetButton(e_ButtonFunction buttonFunction);
    void ResetNotSticky();
    void SetButton(e_ButtonFunction buttonFunction, bool state);
    bool GetPreviousButtonState(e_ButtonFunction buttonFunction);
    blunted::Vector3 GetDirection();
    blunted::Vector3 GetOriginalDirection();

    // Methods for remote controlling.
    void SetDirection(const blunted::Vector3& new_direction);
    bool Disabled() { return disabled_;}
    void SetDisabled(bool disabled);
    void Reset();
    void ProcessState(EnvState* state);
    void Mirror(float mirror);
    e_PlayerColor GetPlayerColor() const { return playerColor; }

  private:
    blunted::Vector3 direction_;
    float mirror = 1.0f;
    bool disabled_ = false;
    bool buttons_pressed_[e_ButtonFunction_Size];
    const e_PlayerColor playerColor;
};

#endif
