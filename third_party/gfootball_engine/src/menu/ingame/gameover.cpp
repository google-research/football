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

#include "gameover.hpp"

#include <cmath>

#include "../../main.hpp"

#include "../pagefactory.hpp"

using namespace blunted;

GameOverPage::GameOverPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  match = GetGameTask()->GetMatch();
  match->Pause(true);

  Gui2Image *bg1 = new Gui2Image(windowManager, "image_gameover_bg", 10, 10, 80, 80);
  this->AddView(bg1);
  bg1->LoadImage("media/menu/backgrounds/black.png");
  bg1->Show();

  std::string scoreStr = match->GetTeam(0)->GetTeamData()->GetName() + " " + int_to_str(match->GetMatchData()->GetGoalCount(0)) + " - " + int_to_str(match->GetMatchData()->GetGoalCount(1)) + " " + match->GetTeam(1)->GetTeamData()->GetName();
  Gui2Caption *header = new Gui2Caption(windowManager, "caption_gameover_header", 0, 15, 80, 4, scoreStr);
  header->SetPosition(50 - header->GetTextWidthPercent() / 2, 15);
  this->AddView(header);
  header->Show();

  buttonOkay = new Gui2Button(windowManager, "button_gameover_ok", 40, 82, 20, 3, "well then");
  this->AddView(buttonOkay);
  buttonOkay->Show();
  buttonOkay->sig_OnClick.connect(boost::bind(&GameOverPage::GoMainMenu, this));

  float possession1 = match->GetMatchData()->GetPossessionTime_ms(0);
  float possession2 = match->GetMatchData()->GetPossessionTime_ms(1);
  int shots1 = match->GetMatchData()->GetShots(0);
  int shots2 = match->GetMatchData()->GetShots(1);

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_gameover_stats", 15, 25, 70, 50);

  grid->AddView(
      new Gui2Caption(windowManager, "caption_possession_t1", 0, 0, 25, 3,
                      int_to_str(std::round(
                          possession1 / (possession1 + possession2) * 100)) +
                          "%"),
      0, 0);
  grid->AddView(new Gui2Caption(windowManager, "caption_possession_header", 0, 0, 35, 3, "possession"), 0, 1);
  grid->AddView(
      new Gui2Caption(windowManager, "caption_possession_t2", 0, 0, 10, 3,
                      int_to_str(std::round(
                          possession2 / (possession1 + possession2) * 100)) +
                          "%"),
      0, 2);

  grid->AddView(new Gui2Caption(windowManager, "caption_shots_t1", 0, 0, 25, 3, int_to_str(shots1)), 1, 0);
  grid->AddView(new Gui2Caption(windowManager, "caption_shots_header", 0, 0, 35, 3, "shots"), 1, 1);
  grid->AddView(new Gui2Caption(windowManager, "caption_shots_t2", 0, 0, 10, 3, int_to_str(shots2)), 1, 2);

  grid->UpdateLayout(0.5);

  this->AddView(grid);
  grid->Show();


  buttonOkay->SetFocus();

  this->Show();
}

GameOverPage::~GameOverPage() {
}

void GameOverPage::GoMainMenu() {
  this->Exit();
  GetMenuTask()->SetMenuAction(e_MenuAction_Menu);
  delete this;
}
