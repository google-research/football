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
#include "../hid/ihidevice.hpp"
#include <set>


class AIControlledKeyboard : public IHIDevice {

  public:
    AIControlledKeyboard();
    virtual ~AIControlledKeyboard() {}

    virtual void LoadConfig() {}
    virtual void SaveConfig() {}

    virtual void Process();

    virtual bool GetButton(e_ButtonFunction buttonFunction);
    virtual void SetButton(e_ButtonFunction buttonFunction, bool state);
    virtual bool GetPreviousButtonState(e_ButtonFunction buttonFunction);
    virtual Vector3 GetDirection();

    // Methods for remote controlling.
    void SetDirection(const Vector3& new_direction);
    virtual void Reset();

  private:
    Vector3 direction_;
    std::set<e_ButtonFunction> buttons_pressed_;
};

#endif
