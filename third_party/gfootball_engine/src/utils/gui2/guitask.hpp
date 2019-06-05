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

#ifndef _HPP_GUI2_GUI2TASK
#define _HPP_GUI2_GUI2TASK

#include "wrap_SDL_ttf.h"

#include "../../types/iusertask.hpp"

#include "../../scene/scene2d/scene2d.hpp"

#include "windowmanager.hpp"

namespace blunted {

  class Gui2Task : public IUserTask {

    public:
      Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin);
      virtual ~Gui2Task();

      virtual void GetPhase();
      virtual void ProcessPhase();
      virtual void PutPhase();

      virtual void ProcessEvents();

      Gui2WindowManager *GetWindowManager() { return this->windowManager; }

      void SetEventJoyButtons(int activate, int escape);

      void EnableKeyboard() { keyboard = true; }
      void DisableKeyboard() { keyboard = false; }
      void SetActiveJoystickID(int joyID) { activeJoystick = joyID; }
      int GetActiveJoystickID() const { return activeJoystick; }

      virtual std::string GetName() const { return "gui2"; }

    protected:
      boost::shared_ptr<Scene2D> scene2D;

      Gui2WindowManager *windowManager;

      std::map<SDL_Keycode, TimedKeyPress> prevKeyState;
      bool prevButtonState[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      bool prevAxisState[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];

      int joyButtonActivate = 0;
      int joyButtonEscape = 0;
      int activeJoystick = 0;
      bool keyboard = false;

  };

}

#endif
