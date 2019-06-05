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

#include "gamepage.hpp"

#include "../pagefactory.hpp"

#include "../../main.hpp"

#include "phasemenu.hpp"
#include "gameover.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

GamePage::GamePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData), match(0) {

  // Gui2Caption *betaSign = new Gui2Caption(windowManager, "caption_betasign", 0, 0, 0, 2, "Google Research Football alpha v0.7");
  // betaSign->SetColor(Vector3(180, 180, 180));
  // betaSign->SetTransparency(0.3f);
  // this->AddView(betaSign);
  // float w = betaSign->GetTextWidthPercent();
  // betaSign->SetPosition(50 - w * 0.5f, 97.0f);
  // betaSign->Show();

  this->Show();

  this->SetFocus();
}

GamePage::~GamePage() {



  if (match) {
    //GetGameTask()->matchLifetimeMutex.lock();
    //if (GetGameTask()->GetMatch() != 0) {

      if (Verbose()) printf("disconnecting signals\n");

      match->sig_OnMatchPhaseChange.disconnect(boost::bind(&GamePage::GoMatchPhasePage, this));
      match->sig_OnExtendedReplayMoment.disconnect(boost::bind(&GamePage::GoExtendedReplayPage, this));
      match->sig_OnGameOver.disconnect(boost::bind(&GamePage::GoGameOverPage, this));

    //}
    //GetGameTask()->matchLifetimeMutex.unlock();
  }

}

void GamePage::Process() {

  if (!match) {
    //GetGameTask()->matchLifetimeMutex.lock();
    if (GetGameTask()->GetMatch() != 0) {

      match = GetGameTask()->GetMatch();

      if (Verbose()) printf("connecting signals\n");

      match->sig_OnMatchPhaseChange.connect(boost::bind(&GamePage::GoMatchPhasePage, this));
      match->sig_OnExtendedReplayMoment.connect(boost::bind(&GamePage::GoExtendedReplayPage, this));
      match->sig_OnGameOver.connect(boost::bind(&GamePage::GoGameOverPage, this));

    }
    //GetGameTask()->matchLifetimeMutex.unlock();
  }

}

void GamePage::GoExtendedReplayPage() {
  Properties properties;
  int replayHistoryOffset_ms = match->GetReplaySize_ms();
  bool stayInReplay = true;
}

void GamePage::GoMatchPhasePage() {

  e_MatchPhase nextPhase = match->GetMatchPhase();

  Properties properties;
  properties.Set("nextphase", (int)nextPhase);
  CreatePage((int)e_PageID_MatchPhase, properties);

}

void GamePage::GoGameOverPage() {
  CreatePage((int)e_PageID_GameOver);
}

void GamePage::ProcessWindowingEvent(WindowingEvent *event) {
  if (Verbose()) if (event->IsEscape()) printf("escape!\n");
  event->Ignore();
}

void GamePage::ProcessKeyboardEvent(KeyboardEvent *event) {

  if (event->GetKeyOnce(SDLK_ESCAPE)) {

    // check which team the keyboard belongs to
    int controllerID = 0;
    const std::vector<IHIDevice*> &controllers = GetControllers();
    for (unsigned int c = 0; c < controllers.size(); c++) {
      if (controllers.at(c)->GetDeviceType() == e_HIDeviceType_Keyboard) {
        controllerID = c;
        break;
      }
    }

    if (Verbose()) printf("controller index %i (keyboard) pressed start\n", controllerID);

    int teamID = 0;
    const std::vector<SideSelection> sides = GetMenuTask()->GetControllerSetup();
    for (unsigned int s = 0; s < sides.size(); s++) {
      if (sides.at(s).controllerID == (signed int)controllerID) {
        teamID = int(round(sides.at(s).side * 0.5 + 0.5));
        break;
      }
    }

    if (Verbose()) printf ("team belonging to this controller seems to be %i\n", teamID);

    Properties properties;
    properties.Set("teamID", teamID);
    CreatePage((int)e_PageID_Ingame, properties);
    return;
  }

  event->Ignore();
}

void GamePage::ProcessJoystickEvent(JoystickEvent *event) {

  // oof, the problem is that we are using a GUI joystick event to find out what ingame HID controller pressed <start>.
  // I think we need a new system in which the game doesn't use its own input system, but uses the GUI one.
  // for now, this hax will have to do

  const std::vector<IHIDevice*> &controllers = GetControllers();
  for (unsigned int c = 0; c < controllers.size(); c++) {

    if (controllers.at(c)->GetDeviceType() == e_HIDeviceType_Gamepad) {
      HIDGamepad *gamepad = static_cast<HIDGamepad*>(controllers.at(c));
      int joyID = gamepad->GetGamepadID(); // these should be the same IDs the GUI system uses as joyID

      if (event->GetButton(joyID, gamepad->GetControllerMapping(gamepad->GetFunctionMapping(e_ButtonFunction_Start)))) {

        if (Verbose()) printf("controller index %i, gamepad/joy ID %i pressed start\n", c, joyID);

        // check which team this controller belongs to
        int teamID = 0;
        const std::vector<SideSelection> sides = GetMenuTask()->GetControllerSetup();
        for (unsigned int s = 0; s < sides.size(); s++) {
          if (sides.at(s).controllerID == (signed int)c) {
            teamID = int(round(sides.at(s).side * 0.5 + 0.5));
            break;
          }
        }

        if (Verbose()) printf ("team belonging to this controller seems to be %i\n", teamID);

        Properties properties;
        properties.Set("teamID", teamID);
        CreatePage((int)e_PageID_Ingame, properties);
        return;
      }

    }

  }

  event->Ignore();
}
