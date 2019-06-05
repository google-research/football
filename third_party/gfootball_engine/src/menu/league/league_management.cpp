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

#include "league_management.hpp"

#include "../pagefactory.hpp"

LeagueManagementPage::LeagueManagementPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management", 20, 20, 60, 3, "Management");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueManagementPage::~LeagueManagementPage() {
}



LeagueManagementContractsPage::LeagueManagementContractsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management_contracts", 20, 20, 60, 3, "Contracts");
  this->AddView(title);

  this->SetFocus();

  this->Show();
}

LeagueManagementContractsPage::~LeagueManagementContractsPage() {
}



LeagueManagementTransfersPage::LeagueManagementTransfersPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management_transfers", 20, 20, 60, 3, "Transfers");
  this->AddView(title);

  this->SetFocus();

  this->Show();
}

LeagueManagementTransfersPage::~LeagueManagementTransfersPage() {
}
