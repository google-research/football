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

#ifndef _HPP_GUI2_EVENTS
#define _HPP_GUI2_EVENTS

#include "../../base/math/vector3.hpp"
#include "../../managers/usereventmanager.hpp"
#include "wrap_SDL.h"
#include "wrap_SDL_keyboard.h"
#include <set>

namespace blunted {

  enum e_Gui2EventType {
    e_Gui2EventType_Windowing = 0,
    e_Gui2EventType_Mouse = 1,
    e_Gui2EventType_Keyboard = 2,
    e_Gui2EventType_Joystick = 3,
    e_Gui2EventType_User = 100
  };

  class Gui2Event {

    public:
      Gui2Event(e_Gui2EventType eventType);
      virtual ~Gui2Event();

      e_Gui2EventType GetType() const;

      void Accept() { accepted = true; }
      void Ignore() { accepted = false; }

      bool IsAccepted() const { return accepted; }

    protected:
      e_Gui2EventType eventType;

      bool accepted = false;

  };

  class WindowingEvent : public Gui2Event {

    public:
      WindowingEvent();
      virtual ~WindowingEvent();

      bool IsActivate() { return activate; }
      bool IsEscape() { return escape; }
      Vector3 GetDirection() { return direction; }

      void SetActivate() { activate = true; }
      void SetEscape() { escape = true; }
      void SetDirection(const Vector3 &direction) { this->direction = direction; }

    protected:
      bool activate = false;
      bool escape = false;
      Vector3 direction;

  };

  class KeyboardEvent : public Gui2Event {

    public:
      KeyboardEvent();
      virtual ~KeyboardEvent();

      const std::set<SDL_Keycode> &GetKeyOnce() const { return keyOnce; }
      bool GetKeyOnce(SDL_Keycode id) const { return keyOnce.count(id); }
      void SetKeyOnce(SDL_Keycode id) { keyOnce.insert(id); }
      bool GetKeyContinuous(SDL_Keycode id) const { return keyContinuous.count(id); }
      void SetKeyContinuous(SDL_Keycode id) { keyContinuous.insert(id); }
      bool GetKeyRepeated(SDL_Keycode id) const { return keyRepeated.count(id); }
      void SetKeyRepeated(SDL_Keycode id) { keyRepeated.insert(id); }
      std::set<SDL_Keycode> &GetKeysymOnce() { return keysymOnce; }
      std::set<SDL_Keycode> &GetKeysymContinuous() { return keysymContinuous; }
      std::set<SDL_Keycode> &GetKeysymRepeated() { return keysymRepeated; }

    protected:
      std::set<SDL_Keycode> keysymOnce;
      std::set<SDL_Keycode> keysymContinuous;
      std::set<SDL_Keycode> keysymRepeated;

      std::set<SDL_Keycode> keyOnce;
      std::set<SDL_Keycode> keyContinuous;
      std::set<SDL_Keycode> keyRepeated;

  };

  class JoystickEvent : public Gui2Event {

    public:
      JoystickEvent();
      virtual ~JoystickEvent();

      bool GetButton(int joyID, int id) const { return button[joyID][id]; }
      void SetButton(int joyID, int id) { button[joyID][id] = true; }
      float GetAxis(int joyID, int id) const { return axes[joyID][id]; }
      void SetAxis(int joyID, int id, float value) { this->axes[joyID][id] = value; }

    protected:

      bool button[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      float axes[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];

  };

}

#endif
