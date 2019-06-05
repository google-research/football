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

#include "scoreboard.hpp"

#include "../../utils/gui2/windowmanager.hpp"

#include "wrap_SDL.h"

#include "../../onthepitch/match.hpp"

using namespace blunted;

Gui2ScoreBoard::Gui2ScoreBoard(Gui2WindowManager *windowManager, Match *match)
    : Gui2View(windowManager, "scoreboard", 2, 2, 96, 4) {
  x_percent = 2;
  y_percent = 2;
  width_percent = 96;
  height_percent = 4;

  Vector3 textColor = 255;
  Vector3 textOutlineColor = 0;

  goalCount[0] = 0;
  goalCount[1] = 0;
  int x, y, w, h;
  windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

  // percentages
  float xOffset[9];
  xOffset[0] = 1; // league logo
  xOffset[1] = 5; // time
  xOffset[2] = 15; // t1 logo
  xOffset[3] = 19; // t1 name
  xOffset[4] = 25; // t1 score
  xOffset[5] = 29; // t2 logo
  xOffset[6] = 33; // t2 name
  xOffset[7] = 39; // t2 score
  xOffset[8] = 43; // end
  float relToAbsMultiplier = w / (width_percent * 1.0f);
  content_yOffset = 0.2f;
  float content_xOffset = 0.2f;

  float bgAlpha = 100.0f;

  Gui2Image *bg = new Gui2Image(windowManager, "image_scoreboard_bg", 0, 0, xOffset[8] + 1, height_percent);
  bg->LoadImage("media/menu/scoreboard_bg.png");
  this->AddView(bg);
  bg->Show();

  leagueLogo = new Gui2Image(windowManager, "game_scoreboard_leaguelogo", xOffset[0], 0, height_percent / windowManager->GetAspectRatio(), height_percent);
  this->AddView(leagueLogo);
  leagueLogo->LoadImage("media/menu/league.png");
  leagueLogo->Show();

  timeCaption = new Gui2Caption(windowManager, "game_scoreboard_timecaption", xOffset[1] + content_xOffset, 0, 5, height_percent * 0.9f, "0:00");
  teamNameCaption[0] = new Gui2Caption(windowManager, "game_scoreboard_team1name", xOffset[3] + content_xOffset, 0, 5, height_percent * 0.9f, match->GetTeam(0)->GetTeamData()->GetShortName());
  teamNameCaption[1] = new Gui2Caption(windowManager, "game_scoreboard_team2name", xOffset[6] + content_xOffset, 0, 5, height_percent * 0.9f, match->GetTeam(1)->GetTeamData()->GetShortName());
  goalCountCaption[0] = new Gui2Caption(windowManager, "game_scoreboard_team1goals", xOffset[4] + content_xOffset, 0, 5, height_percent * 0.9f, "0");
  goalCountCaption[1] = new Gui2Caption(windowManager, "game_scoreboard_team2goals", xOffset[7] + content_xOffset, 0, 5, height_percent * 0.9f, "0");

  timeCaption->SetColor(textColor);
  timeCaption->SetOutlineColor(textOutlineColor);
  teamNameCaption[0]->SetColor(textColor);
  teamNameCaption[0]->SetOutlineColor(textOutlineColor);
  teamNameCaption[1]->SetColor(textColor);
  teamNameCaption[1]->SetOutlineColor(textOutlineColor);
  goalCountCaption[0]->SetColor(textColor);
  goalCountCaption[0]->SetOutlineColor(textOutlineColor);
  goalCountCaption[1]->SetColor(textColor);
  goalCountCaption[1]->SetOutlineColor(textOutlineColor);

  this->AddView(timeCaption);
  timeCaption->Show();
  this->AddView(teamNameCaption[0]);
  teamNameCaption[0]->Show();
  this->AddView(teamNameCaption[1]);
  teamNameCaption[1]->Show();
  this->AddView(goalCountCaption[0]);
  goalCountCaption[0]->Show();
  this->AddView(goalCountCaption[1]);
  goalCountCaption[1]->Show();

  SetGoalCount(0, 0);
  SetGoalCount(1, 0);

  teamLogo[0] = new Gui2Image(windowManager, "game_scoreboard_team1logo", xOffset[2], 0, height_percent / windowManager->GetAspectRatio(), height_percent);
  this->AddView(teamLogo[0]);
  teamLogo[0]->LoadImage(match->GetTeam(0)->GetTeamData()->GetLogoUrl());
  teamLogo[0]->Show();

  teamLogo[1] = new Gui2Image(windowManager, "game_scoreboard_team2logo", xOffset[5], 0, height_percent / windowManager->GetAspectRatio(), height_percent);
  this->AddView(teamLogo[1]);
  teamLogo[1]->LoadImage(match->GetTeam(1)->GetTeamData()->GetLogoUrl());
  teamLogo[1]->Show();

  this->Show();
}

Gui2ScoreBoard::~Gui2ScoreBoard() {
  /* will be cleaned up while deleting gui2 tree automatically
  bg->Exit();
  delete bg;
  leagueLogo->Exit();
  delete leagueLogo;
  timeCaption->Exit();
  delete timeCaption;
  teamNameCaption[0]->Exit();
  delete teamNameCaption[0];
  teamNameCaption[1]->Exit();
  delete teamNameCaption[1];
  goalCountCaption[0]->Exit();
  delete goalCountCaption[0];
  goalCountCaption[1]->Exit();
  delete goalCountCaption[1];
  teamLogo[0]->Exit();
  delete teamLogo[0];
  teamLogo[1]->Exit();
  delete teamLogo[1];
  tvLogo->Exit();
  delete tvLogo;
  */
}

void Gui2ScoreBoard::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
  Gui2View::GetImages(target);
}

void Gui2ScoreBoard::Redraw() {
}

void Gui2ScoreBoard::SetTimeStr(const std::string &timeStr) {
  this->timeStr = timeStr;
  timeCaption->SetCaption(timeStr);
}

void Gui2ScoreBoard::SetGoalCount(int teamID, int goalCount) {
  this->goalCount[teamID] = goalCount;
  std::string goalStr = "";
  if (goalCount < 10) goalStr.append(" ");
  goalStr.append(int_to_str(goalCount));
  goalCountCaption[teamID]->SetCaption(goalStr);
}
