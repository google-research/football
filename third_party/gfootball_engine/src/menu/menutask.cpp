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

#include "menutask.hpp"

#include "../onthepitch/match.hpp"

#include "pagefactory.hpp"

#include "../gametask.hpp"

#include "../main.hpp"

#include "../framework/scheduler.hpp"
#include "../managers/resourcemanagerpool.hpp"

using namespace blunted;

MenuTask::MenuTask(float aspectRatio, float margin, TTF_Font *defaultFont,
                   TTF_Font *defaultOutlineFont, const Properties *config)
    : Gui2Task(GetScene2D(), aspectRatio, margin) {
  Gui2Style *style = windowManager->GetStyle();

  style->SetFont(e_TextType_Default, defaultFont);
  style->SetFont(e_TextType_DefaultOutline, defaultOutlineFont);
  style->SetFont(e_TextType_Caption, defaultFont);
  style->SetFont(e_TextType_Title, defaultFont);
  style->SetFont(e_TextType_ToolTip, defaultFont);

  // huisstijl:
  // blauw: 0, 100, 220
  // orange: 240, 100, 0
  style->SetColor(e_DecorationType_Dark1, Vector3(20, 35, 55));
  style->SetColor(e_DecorationType_Dark2, Vector3(60, 35, 20));
  style->SetColor(e_DecorationType_Bright1, Vector3(150, 180, 220));
  style->SetColor(e_DecorationType_Bright2, Vector3(240, 150, 100));
  style->SetColor(e_DecorationType_Toggled, Vector3(240, 60, 60));

  windowManager->SetTimeStep_ms(10);

  Gui2Root *root = windowManager->GetRoot();
  root->Show();

  PageFactory *pageFactory = new PageFactory();
  windowManager->SetPageFactory(pageFactory);

  int size = GetControllers().size();

  for (int i = 0; i < size; i++) {
    SideSelection side;
    side.controllerID = i;
    // Everybody plays in the same team.
    side.side = -1;
//      if ((size > 1 && i == 1) || (size == 1 && i == 0)) {
//        side.side = -1;
//      } else {
//        side.side = 0;
//      }
    queuedFixture.sides.push_back(side);
  }

  // 1 == ajax
  // 2 == arsenal
  // 3 == barcelona
  // 4 == bayern
  // 5 == borussia
  // 6 == man utd
  // 7 == psv
  // 8 == real madrid
  queuedFixture.teamID1 = "3";
  queuedFixture.teamID2 = "8";
  queuedFixture.team1KitNum = 2;
  queuedFixture.team2KitNum = 2;
  menuAction = e_MenuAction_Menu;
}

MenuTask::~MenuTask() {
  delete windowManager->GetPageFactory();
}

void MenuTask::ProcessPhase() {

  Gui2Task::ProcessPhase();

  if (menuAction == e_MenuAction_Menu) {

    windowManager->GetPagePath()->Clear();

    GetGameTask()->Action(e_GameTaskMessage_StopMatch);
    GetGameTask()->Action(e_GameTaskMessage_StartMenuScene);

    Properties properties;
    windowManager->GetPageFactory()->CreatePage((int)e_PageID_LoadingMatch, properties, 0);
  } else if (menuAction == e_MenuAction_Game) {

    GetGameTask()->Action(e_GameTaskMessage_StopMenuScene);
    GetGameTask()->Action(e_GameTaskMessage_StartMatch);

  }

  menuAction = e_MenuAction_None;
}

