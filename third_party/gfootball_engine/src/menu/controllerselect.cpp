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

#include "controllerselect.hpp"

#include "../main.hpp"

#include "mainmenu.hpp"

#include "startmatch/teamselect.hpp"

#include "pagefactory.hpp"

using namespace blunted;

ControllerSelectPage::ControllerSelectPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  inGame = pageData.properties->GetBool("isInGame");

  Gui2Image *bg1 = new Gui2Image(windowManager, "image_gameover_bg", 10, 15, 80, 70);
  this->AddView(bg1);
  bg1->LoadImage("media/menu/backgrounds/black.png");
  bg1->Show();

  Gui2Caption *t1 = new Gui2Caption(windowManager, "caption_controllerselect_t1", 0, 0, 28, 3, "Team 1");
  Gui2Caption *t2 = new Gui2Caption(windowManager, "caption_controllerselect_t2", 0, 0, 28, 3, "Team 2");

  t1->SetPosition(25 - t1->GetTextWidthPercent() * 0.5, 10);
  t2->SetPosition(75 - t2->GetTextWidthPercent() * 0.5, 10);

  this->AddView(t1);
  t1->Show();
  this->AddView(t2);
  t2->Show();

  this->SetFocus();

  const std::vector<IHIDevice*> &controllers = GetControllers();
  if (inGame) {
    sides = GetMenuTask()->GetControllerSetup();
    assert(sides.size() == controllers.size());
  }
  for (unsigned int i = 0; i < controllers.size(); i++) {
    SideSelection side;
    side.controllerID = i;
    if (inGame) {
      side.side = sides.at(i).side;
    } else {
      side.side = 0;
      if (i == 0 && controllers.size() < 2) side.side = -1; // autoselect 1st player == team 0 (side -1)
      else if (i == 1) side.side = -1; // if more than 1 controller, we're likely to have a gamepad on id > 0, so pick this one as auto p1 instead
    }
    side.controllerImage = new Gui2Image(windowManager, "image_controller" + int_to_str(i), 0, 0, 14, 10);
    this->AddView(side.controllerImage);
    if (controllers.at(i)->GetDeviceType() == e_HIDeviceType_Gamepad) {
      side.controllerImage->LoadImage("media/menu/controller/controller_small.png");
    } else {
      side.controllerImage->LoadImage("media/menu/controller/keyboard_small.png");
    }
    side.controllerImage->Show();
    if (!inGame) sides.push_back(side); else sides.at(i) = side;
    delay.push_back(0);
  }

  SetImagePositions();

  this->Show();
}

ControllerSelectPage::~ControllerSelectPage() {
}

void ControllerSelectPage::SetImagePositions() {
  for (unsigned int i = 0; i < sides.size(); i++) {
    int x = 43 + sides.at(i).side * 25;
    sides.at(i).controllerImage->SetPosition(x, 20 + i * 15);
  }
}

void ControllerSelectPage::Process() {
  Gui2View::Process();
}

void ControllerSelectPage::ProcessKeyboardEvent(KeyboardEvent *event) {
  HIDKeyboard *keyboard = static_cast<HIDKeyboard*>(GetControllers().at(0));
  if (event->GetKeyOnce(keyboard->GetFunctionMapping(e_ButtonFunction_Left))) {
    sides.at(0).side -= 1;
  }
  if (event->GetKeyOnce(keyboard->GetFunctionMapping(e_ButtonFunction_Right))) {
    sides.at(0).side += 1;
  }
  sides.at(0).side = clamp(sides.at(0).side, -1, 1);

  SetImagePositions();
}

void ControllerSelectPage::ProcessJoystickEvent(JoystickEvent *event) {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  for (unsigned int i = 1; i < controllers.size(); i++) {
    if (delay.at(i) < EnvironmentManager::GetInstance().GetTime_ms() - 250) {
      HIDGamepad *gamepad = static_cast<HIDGamepad*>(controllers.at(i));
      if (gamepad->GetButtonValue(e_ButtonFunction_Left) > 0.5) {
        sides.at(i).side -= 1;
        delay.at(i) = EnvironmentManager::GetInstance().GetTime_ms();
      }
      if (gamepad->GetButtonValue(e_ButtonFunction_Right) > 0.5) {
        sides.at(i).side += 1;
        delay.at(i) = EnvironmentManager::GetInstance().GetTime_ms();
      }
      sides.at(i).side = clamp(sides.at(i).side, -1, 1);
    }
  }

  SetImagePositions();
}

void ControllerSelectPage::ProcessWindowingEvent(WindowingEvent *event) {
  if (event->IsActivate()) {
    if (!inGame) {
      GetMenuTask()->SetControllerSetup(sides);

      CreatePage(e_PageID_TeamSelect);
      return;
    }
    Gui2Page::ProcessWindowingEvent(event);
  }
  if (event->IsEscape()) {
    if (inGame) {
      GetMenuTask()->SetControllerSetup(sides);
      GetGameTask()->GetMatch()->UpdateControllerSetup();
    }
    Gui2Page::ProcessWindowingEvent(event);
  }
}
