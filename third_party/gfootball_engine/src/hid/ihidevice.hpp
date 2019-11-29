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

#ifndef _HPP_HIDEVICE
#define _HPP_HIDEVICE

#include "../base/math/vector3.hpp"
#include "../defines.hpp"

using namespace blunted;

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

class IHIDevice {

  public:
    virtual ~IHIDevice() { DO_VALIDATION;}

    virtual void Reset() { DO_VALIDATION;};
    virtual void ResetNotSticky() = 0;
    virtual bool GetButton(e_ButtonFunction buttonFunction) = 0;
    virtual bool GetPreviousButtonState(e_ButtonFunction buttonFunction) = 0;
    virtual Vector3 GetDirection() = 0;
    virtual Vector3 GetOriginalDirection() = 0;
    virtual void ProcessState(EnvState* state) = 0;
    virtual void Mirror(float mirror) = 0;
};

#endif
