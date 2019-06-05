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

#include "guitask.hpp"

#include "../../managers/usereventmanager.hpp"

namespace blunted {

  Gui2Task::Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin) : scene2D(scene2D) {
    windowManager = new Gui2WindowManager(scene2D, aspectRatio, margin);
    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
        prevButtonState[j][i] = false;
      }
    }
    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
        prevAxisState[j][i] = false;
      }
    }

    joyButtonActivate = 1;
    joyButtonEscape = 1;
    activeJoystick = 0;

    keyboard = false;
  }

  Gui2Task::~Gui2Task() {
    windowManager->Exit();
    delete windowManager;
  }

  void Gui2Task::GetPhase() {
  }

  void Gui2Task::ProcessPhase() {
    windowManager->GetRoot()->SetRecursiveZPriority(0);
    ProcessEvents();
    windowManager->Process();
  }

  void Gui2Task::PutPhase() {
  }

  void Gui2Task::ProcessEvents() {



    if (!windowManager->GetFocus()) return;
    Gui2View *currentFocus = windowManager->GetFocus();

    // some buttons need to be sent through a windowing event (escape, enter, directional controls)
    // if a view accepts a keyboard/joystick event, don't send windowing event => *disabled, now uses focus check
    bool needsWindowingEvent = false;
    WindowingEvent *wEvent = new WindowingEvent();


    // keyboard events

    KeyboardEvent *event = new KeyboardEvent();
    bool needsKeyboardEvent = false;

    auto current = UserEventManager::GetInstance().GetKeyboardState();
    prevKeyState = current;

    if (needsKeyboardEvent) {
      if (windowManager->GetFocus() == currentFocus) windowManager->GetFocus()->ProcessEvent(event);
    }
    delete event;


    // joystick events

    if (needsWindowingEvent)
      if (windowManager->GetFocus() == currentFocus) windowManager->GetFocus()->ProcessEvent(wEvent);

    delete wEvent;

  }

  void Gui2Task::SetEventJoyButtons(int activate, int escape) {
    this->joyButtonActivate = activate;
    this->joyButtonEscape = escape;
  }

}
