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

#include "teamselect.hpp"

#include "../../main.hpp"

#include "../../utils/database.hpp"

#include "../pagefactory.hpp"

using namespace blunted;

void AddCompetitions(Gui2IconSelector *selector) {
  /*
  selector->AddEntry("1", "National teams", "databases/default/images_competitions/nationalteams.png");
  selector->AddEntry("2", "Premier league", "databases/default/images_competitions/premierleague.png");
  selector->AddEntry("3", "Eredivisie", "databases/default/images_competitions/eredivisie.png");
  selector->AddEntry("4", "Bundesliga", "databases/default/images_competitions/bundesliga.png");
  selector->AddEntry("5", "LFP", "databases/default/images_competitions/lfp.png");
  selector->AddEntry("6", "Serie A", "databases/default/images_competitions/serie_a.png");
  selector->AddEntry("7", "Ligue 1", "databases/default/images_competitions/ligue1.png");
  */
  DatabaseResult *result = GetDB()->Query("select id, name, logo_url from leagues");

  for (unsigned int r = 0; r < result->data.size(); r++) {
    int id = atoi(result->data.at(r).at(0).c_str());
    std::string name = result->data.at(r).at(1).c_str();
    std::string logo_url = result->data.at(r).at(2).c_str();

    std::string logoPath = "databases/default/" + logo_url;
    if (!boost::filesystem::exists(logoPath)) logoPath = "media/textures/orange.jpg";
    selector->AddEntry(int_to_str(id), name, logoPath);
  }

  delete result;
}

void AddTeams(Gui2IconSelector *selector, const std::string &competition_id) {

  DatabaseResult *result = GetDB()->Query("select id, name, logo_url, kit_url from teams where league_id = " + competition_id + " order by name");

  for (unsigned int r = 0; r < result->data.size(); r++) {
    int id = atoi(result->data.at(r).at(0).c_str());
    std::string name = result->data.at(r).at(1).c_str();
    std::string logo_url = result->data.at(r).at(2).c_str();

    std::string logoPath = "databases/default/" + logo_url;
    if (!boost::filesystem::exists(logoPath)) logoPath = "media/textures/orange.jpg";
    selector->AddEntry(int_to_str(id), name, logoPath);
  }

  delete result;

  selector->Show();
}

TeamSelectPage::TeamSelectPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Image *bg1 = new Gui2Image(windowManager, "teamselect_image_bg1", 19, 24, 30, 42);
  this->AddView(bg1);
  bg1->LoadImage("media/menu/backgrounds/black.png");
  bg1->Show();

  bg2 = new Gui2Image(windowManager, "teamselect_image_bg2", 51, 24, 30, 42);
  this->AddView(bg2);
  bg2->LoadImage("media/menu/backgrounds/black.png");

  Gui2Caption *teamEmblemCredits = new Gui2Caption(windowManager, "teamselect_emblemcredits", 19, 70, 28, 3, "Team emblems by TureckiRumun, broxopios, balder, and NLP !");
  this->AddView(teamEmblemCredits);
  teamEmblemCredits->SetColor(Vector3(200, 200, 200));
  teamEmblemCredits->SetTransparency(0.5f);
  teamEmblemCredits->SetPosition(50 - teamEmblemCredits->GetTextWidthPercent() / 2, 70);
  teamEmblemCredits->Show();

  Gui2Caption *p1 = new Gui2Caption(windowManager, "teamselect_caption_p1", 19, 20, 28, 3, "Player 1");
  p2 = new Gui2Caption(windowManager, "teamselect_caption_p2", 51, 20, 28, 3, "Player 2");
  Gui2Grid *grid1 = new Gui2Grid(windowManager, "teamselect_grid_team1", 19, 24, 30, 41);
  grid2 = new Gui2Grid(windowManager, "teamselect_grid_team2", 51, 24, 30, 41);

  competitionSelect1 = new Gui2IconSelector(windowManager, "teamselect_iconselector_competition1", 0, 0, 29, 18, "Competition select");
  competitionSelect2 = new Gui2IconSelector(windowManager, "teamselect_iconselector_competition2", 0, 0, 29, 18, "Competition select");
  teamSelect1 = new Gui2IconSelector(windowManager, "teamselect_iconselector_team1", 0, 0, 29, 18, "Team select");
  teamSelect2 = new Gui2IconSelector(windowManager, "teamselect_iconselector_team2", 0, 0, 29, 18, "Team select");
  buttonStart1 = new Gui2Button(windowManager, "teamselect_button_start1", 0, 0, 29, 3, "Ready");
  buttonStart2 = new Gui2Button(windowManager, "teamselect_button_start2", 0, 0, 29, 3, "Ready");

  competitionSelect1->sig_OnClick.connect(boost::bind(&TeamSelectPage::FocusTeamSelect1, this));
  teamSelect1->sig_OnClick.connect(boost::bind(&TeamSelectPage::FocusStart1, this));
  buttonStart1->sig_OnClick.connect(boost::bind(&TeamSelectPage::FocusCompetitionSelect2, this));
  competitionSelect2->sig_OnClick.connect(boost::bind(&TeamSelectPage::FocusTeamSelect2, this));
  teamSelect2->sig_OnClick.connect(boost::bind(&TeamSelectPage::FocusStart2, this));
  buttonStart2->sig_OnClick.connect(boost::bind(&TeamSelectPage::GoOptionsMenu, this));

  competitionSelect1->sig_OnChange.connect(boost::bind(&TeamSelectPage::SetupTeamSelect1, this));
  competitionSelect2->sig_OnChange.connect(boost::bind(&TeamSelectPage::SetupTeamSelect2, this));

  this->AddView(p1);
  p1->Show();
  this->AddView(grid1);
  grid1->AddView(competitionSelect1, 0, 0);
  grid1->AddView(teamSelect1, 1, 0);
  grid1->AddView(buttonStart1, 2, 0);
  grid1->UpdateLayout(0.5);
  grid1->Show();

  AddCompetitions(competitionSelect1);
  AddTeams(teamSelect1, "1");

  this->AddView(p2);
  this->AddView(grid2);
  grid2->AddView(competitionSelect2, 0, 0);
  grid2->AddView(teamSelect2, 1, 0);
  grid2->AddView(buttonStart2, 2, 0);
  grid2->UpdateLayout(0.5);

  AddCompetitions(competitionSelect2);
  AddTeams(teamSelect2, "1");

  competitionSelect1->SetFocus();

  SetActiveController(-1, true);

  p2->Hide();
  grid2->Hide();
  bg2->Hide();

  this->Show();
}

TeamSelectPage::~TeamSelectPage() {
  GetMenuTask()->SetActiveJoystickID(0);
  GetMenuTask()->EnableKeyboard();
}

void TeamSelectPage::FocusTeamSelect1() {
  teamSelect1->SetFocus();
}

void TeamSelectPage::FocusStart1() {
  buttonStart1->SetFocus();
}

void TeamSelectPage::FocusCompetitionSelect2() {
  p2->Show();
  grid2->Show();
  bg2->Show();

  competitionSelect2->SetFocus();

  SetActiveController(1, true);
}

void TeamSelectPage::FocusTeamSelect2() {
  teamSelect2->SetFocus();
}

void TeamSelectPage::FocusStart2() {
  buttonStart2->SetFocus();
}

void TeamSelectPage::SetupTeamSelect1() {
  teamSelect1->ClearEntries();
  AddTeams(teamSelect1, competitionSelect1->GetSelectedEntryID());
}

void TeamSelectPage::SetupTeamSelect2() {
  teamSelect2->ClearEntries();
  AddTeams(teamSelect2, competitionSelect2->GetSelectedEntryID());

  // hax lol, well doesn't seem to work :(
  /*
  teamSelect2->Process();
  WindowingEvent *right = new WindowingEvent();
  right->SetDirection(Vector3(1, 0, 0));
  teamSelect2->ProcessWindowingEvent(right);
  delete right;
  */
}

void TeamSelectPage::GoOptionsMenu() {
  GetMenuTask()->SetTeamIDs(teamSelect1->GetSelectedEntryID(), teamSelect2->GetSelectedEntryID());
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_MatchOptions, properties, 0);
}

void TeamSelectPage::ProcessWindowingEvent(WindowingEvent *event) {
  if (event->IsEscape()) {
    if (windowManager->GetFocus() == competitionSelect1) {
      Gui2Page::ProcessWindowingEvent(event);
    } else if (windowManager->GetFocus() == teamSelect1) {
      windowManager->SetFocus(competitionSelect1);
    } else if (windowManager->GetFocus() == buttonStart1) {
      windowManager->SetFocus(teamSelect1);
    } else if (windowManager->GetFocus() == competitionSelect2) {
      windowManager->SetFocus(buttonStart1);

      p2->Hide();
      grid2->Hide();
      bg2->Hide();

      SetActiveController(-1, true);

    } else if (windowManager->GetFocus() == teamSelect2) {
      windowManager->SetFocus(competitionSelect2);
    } else if (windowManager->GetFocus() == buttonStart2) {
      windowManager->SetFocus(teamSelect2);
    }

  }

}
