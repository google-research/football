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

#ifndef _HPP_MANAGERS_USEREVENT
#define _HPP_MANAGERS_USEREVENT

#include "../defines.hpp"

#include "../types/singleton.hpp"

#include "../base/math/vector3.hpp"

#include "wrap_SDL.h"
#include <map>

namespace blunted {

  const int _JOYSTICK_MAX = 8;
  const int _JOYSTICK_MAXBUTTONS = 32;
  const int _JOYSTICK_MAXAXES = 8;

  struct TimedKeyPress {
    unsigned long pressTime_ms = 0;
  };

  class UserEventManager : public Singleton<UserEventManager> {

    friend class Singleton<UserEventManager>;

    public:
      UserEventManager();
      virtual ~UserEventManager();

      virtual void Exit();

      virtual void InputSDLEvent(const SDL_Event &event);

      bool GetKeyboardState(SDL_Keycode code) const;
      std::map<SDL_Keycode, TimedKeyPress> GetKeyboardState() const;
      void SetKeyboardState(SDL_Keycode key, bool newState);
      unsigned long GetLastKeyPressDiff_ms(SDL_Keycode key);

      int GetJoystickCount() { return SDL_NumJoysticks(); }
      bool GetJoyButtonState(int joyID, int sdlJoyButtonID) const;
      void SetJoyButtonState(int joyID, int sdlJoyButtonID, bool newState);

      float GetJoystickAxis(int joyID, int axisID, bool deadzone = true) const;
      float GetJoystickAxisRaw(int joyID, int axisID) const;
      void SetJoystickAxisCalibration(int joyID, int axisID, float min,
                                      float max, float rest);

     protected:
      // may need to switch to the vectors below in the future. why? SDL_keysym also takes into account unicode and modifier stuff, and contains the SDL_Keycodes.
      // so basically, it's the parent structure, and we are going to need that info at some point, and we can't make an array with the max size (probably) since
      // i guess it has a dynamic size.
      std::map<SDL_Keycode, TimedKeyPress> keyPressed;
      unsigned long lastKeyTime_ms = 0;

      bool mousePressed[8];

      SDL_Joystick *joystick[_JOYSTICK_MAX];
      bool joyButtonPressed[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      float joyAxis[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];
      float joyAxisCalibration[_JOYSTICK_MAX][_JOYSTICK_MAXAXES][3]; // min, max, rest

      mutable boost::mutex keyPressedMutex;
      mutable boost::mutex mousePressedMutex;
      mutable boost::mutex joyButtonPressedMutex;

  };

}

#endif
