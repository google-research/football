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

#include "league_team.hpp"

#include "../pagefactory.hpp"

LeagueTeamPage::LeagueTeamPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team", 20, 20, 60, 3, "Team");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPage::~LeagueTeamPage() {
}



LeagueTeamFormationPage::LeagueTeamFormationPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_formation", 20, 20, 60, 3, "Formation");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamFormationPage::~LeagueTeamFormationPage() {
}



LeagueTeamPlayerSelectionPage::LeagueTeamPlayerSelectionPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playerselection", 20, 20, 60, 3, "Player selection");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerSelectionPage::~LeagueTeamPlayerSelectionPage() {
}



LeagueTeamTacticsPage::LeagueTeamTacticsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_tactics", 20, 20, 60, 3, "Tactics");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamTacticsPage::~LeagueTeamTacticsPage() {
}



LeagueTeamPlayerOverviewPage::LeagueTeamPlayerOverviewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playeroverview", 20, 20, 60, 3, "Player overview");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerOverviewPage::~LeagueTeamPlayerOverviewPage() {
}



LeagueTeamPlayerDevelopmentPage::LeagueTeamPlayerDevelopmentPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playerdevelopment", 20, 20, 60, 3, "Player development");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerDevelopmentPage::~LeagueTeamPlayerDevelopmentPage() {
}



LeagueTeamSetupPage::LeagueTeamSetupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_setup", 20, 20, 60, 3, "Team setup");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamSetupPage::~LeagueTeamSetupPage() {
}
