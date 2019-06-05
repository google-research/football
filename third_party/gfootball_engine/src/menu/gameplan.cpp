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

#include "gameplan.hpp"

#include "../main.hpp"

#include "mainmenu.hpp"

using namespace blunted;

GamePlanPage::GamePlanPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  teamID = pageData.properties->GetInt("teamID", 0);

  int xOffset = 32.5;//14;
  teamData = GetGameTask()->GetMatch()->GetTeam(teamID)->GetTeamData();

  Gui2Image *bg1 = new Gui2Image(windowManager, "gameplan_image_bg", xOffset, 15, 35, 72);
  this->AddView(bg1);
  bg1->LoadImage("media/menu/backgrounds/black.png");
  bg1->Show();

  Gui2Caption *header = new Gui2Caption(windowManager, "gameplan_header", xOffset, 11, 35, 3, "Team " + int_to_str(teamID + 1) + " game plan");
  grid = new Gui2Grid(windowManager, "gameplan_grid", xOffset, 15, 0, 0);
  gridNav = new Gui2Grid(windowManager, "gameplan_grid_navigation", xOffset, 0, 0, 0);

  map = new Gui2PlanMap(windowManager, "gameplan_planmap", 0, 0, 35, 28, teamData);
  buttonLineup = new Gui2Button(windowManager, "gameplan_button_lineup", 0, 0, 34, 3, "Line-up");
  buttonTactics = new Gui2Button(windowManager, "gameplan_button_tactics", 0, 0, 34, 3, "Tactics");
  Gui2Button *buttonFormation = new Gui2Button(windowManager, "gameplan_button_formation", 0, 0, 34, 3, "Formation");

  buttonLineup->sig_OnClick.connect(boost::bind(&GamePlanPage::GoLineupMenu, this));
  buttonTactics->sig_OnClick.connect(boost::bind(&GamePlanPage::GoTacticsMenu, this));


  if (IsReleaseVersion()) {
    buttonLineup->SetActive(false);
    buttonFormation->SetActive(false);
  }

  this->sig_OnClose.connect(boost::bind(&GamePlanPage::OnClose, this));

  this->AddView(header);
  header->Show();

  this->AddView(grid);
  gridNav->AddView(buttonLineup, 0, 0);
  gridNav->AddView(buttonTactics, 1, 0);
  gridNav->AddView(buttonFormation, 2, 0);
  gridNav->UpdateLayout(0.5);
  grid->AddView(map, 0, 0);
  grid->AddView(gridNav, 1, 0);

  grid->UpdateLayout(0.0);
  grid->Show();

  buttonTactics->SetFocus();

  this->Show();

  if (UpdateNonImportableDB()) {
    namedb = new Database();
    bool dbSuccess = namedb->Load("databases/names.sqlite");
    if (!dbSuccess) Log(e_FatalError, "MainMenuPage", "GoImportDB", "Could not open database");
  } else {
    namedb = 0;
  }
}

GamePlanPage::~GamePlanPage() {
}

void GamePlanPage::OnClose() {
  if (namedb) delete namedb;
}

void GamePlanPage::Deactivate() {
  grid->RemoveView(1, 0);
}

void GamePlanPage::Reactivate() {
  grid->AddView(gridNav, 1, 0);
  grid->UpdateLayout(0.0);
  gridNav->Show();
}

Vector3 GamePlanPage::GetButtonColor(int id) {
  Vector3 color = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);
  if (id > 10) color = Vector3(240, 140, 60);
  if (id > 21) color = Vector3(80, 140, 255);
  return color;
}

void GamePlanPage::GoLineupMenu() {
  Deactivate();

  lineupMenu = new GamePlanSubMenu(windowManager, buttonLineup, grid, "lineup_submenu");
  lineupMenu->sig_OnClose.connect(boost::bind(&GamePlanPage::SaveLineup, this));
  lineupMenu->sig_OnClose.connect(boost::bind(&GamePlanPage::Reactivate, this));

  const std::vector<PlayerData*> &playerData = teamData->GetPlayerData();
  for (unsigned int i = 0; i < playerData.size(); i++) {
    Vector3 color = GetButtonColor(i);
    Gui2Button *button = lineupMenu->AddButton("playerbutton_id" + int_to_str(playerData.at(i)->GetDatabaseID()), playerData.at(i)->GetLastName(), i, 0, color);
    button->sig_OnClick.connect(boost::bind(&GamePlanPage::LineupMenuOnClick, this, _1));
    button->SetToggleable(true);
    if (i == 0) button->SetFocus();
  }

  lineupMenu->Show();
}

void GamePlanPage::LineupMenuOnClick(Gui2Button *button) {
  Gui2Button *selected = lineupMenu->GetToggledButton(button);
  if (selected) {
    // switch players
    selected->SetToggled(false);
    button->SetToggled(false);

    int rowSelected = lineupMenu->GetGrid()->GetRow(selected);
    int rowButton = lineupMenu->GetGrid()->GetRow(button);
    assert(rowSelected != -1 && rowButton != -1);
    lineupMenu->GetGrid()->RemoveView(rowSelected, 0);
    lineupMenu->GetGrid()->RemoveView(rowButton, 0);
    lineupMenu->GetGrid()->AddView(button, rowSelected, 0);
    button->Show();
    button->SetColor(GetButtonColor(rowSelected));
    lineupMenu->GetGrid()->AddView(selected, rowButton, 0);
    selected->Show();
    selected->SetColor(GetButtonColor(rowButton));
    lineupMenu->GetGrid()->UpdateLayout(0.5);
    selected->SetFocus();

    int id1 = atoi(selected->GetName().substr(selected->GetName().rfind("id") + 2, std::string::npos).c_str());
    int id2 = atoi(  button->GetName().substr(  button->GetName().rfind("id") + 2, std::string::npos).c_str());
    teamData->SwitchPlayers(id1, id2);
  }
}

void GamePlanPage::SaveLineup() {

  if (UpdateNonImportableDB()) {
    // saves to temp names db, which is used when importing the actual db.

    const std::vector<Gui2Button*> &allButtons = lineupMenu->GetAllButtons();

    for (unsigned int i = 0; i < allButtons.size(); i++) {
      Gui2View *button = lineupMenu->GetGrid()->FindView(i, 0);
      int id = atoi(button->GetName().substr(button->GetName().rfind("id") + 2, std::string::npos).c_str());
      PlayerData* playerData = teamData->GetPlayerDataByDatabaseID(id);
      unsigned int formationorder = i;

      // find player
      DatabaseResult *result = namedb->Query("select id from playernames where fakefirstname = \"" + playerData->GetFirstName() + "\" and fakelastname = \"" + playerData->GetLastName() + "\" limit 1;");
      int playerDatabaseID = -1;
      if (result->data.size() > 0) {
        playerDatabaseID = atoi(result->data.at(0).at(0).c_str());
        delete result;
        result = namedb->Query("update playernames set formationorder = " + int_to_str(formationorder) + " where id = " + int_to_str(playerDatabaseID) + ";");
        delete result;
      } else { // player does not yet exist in namedb
        if (Verbose()) printf("WARNING: player does not exist in namedb: %s %s\n", playerData->GetFirstName().c_str(), playerData->GetLastName().c_str());
        delete result;
      }
    }
  }

  teamData->SaveLineup();
}


void GamePlanPage::GoTacticsMenu() {
  Deactivate();

  tacticsSliders.clear(); // could still be here from previous tactics subpage visit. should clear this on leaving page but we have no mechanism to do so (<- todo, could use onclose probably)

  tacticsMenu = new GamePlanSubMenu(windowManager, buttonTactics, grid, "tactics_submenu");
  tacticsMenu->sig_OnClose.connect(boost::bind(&GamePlanPage::SaveTactics, this));
  tacticsMenu->sig_OnClose.connect(boost::bind(&GamePlanPage::Reactivate, this));

  const Properties &userProps = teamData->GetTactics().userProperties;
  const map_Properties *userPropMap = userProps.GetProperties();
  const Properties &factoryProps = teamData->GetTactics().factoryProperties;
  const map_Properties *factoryPropMap = factoryProps.GetProperties();

  map_Properties::const_iterator iter = userPropMap->begin();
  int i = 0;
  while (iter != userPropMap->end()) {
    const std::string &tacticName = (*iter).first;
    if (Verbose()) printf("adding %s\n", tacticName.c_str());
    TacticsSlider slider;
    slider.id = i;
    slider.tacticName = tacticName;
    slider.widget = tacticsMenu->AddSlider("tacticsslider_" + slider.tacticName, teamData->GetTactics().humanReadableNames.Get(slider.tacticName.c_str(), slider.tacticName), i, 0);
    slider.widget->AddHelperValue(Vector3(80, 80, 250), "factory default for this team", factoryProps.GetReal(slider.tacticName.c_str()));
    slider.widget->SetValue(userProps.GetReal(slider.tacticName.c_str()));
    slider.widget->sig_OnChange.connect(boost::bind(&GamePlanPage::TacticsMenuOnChange, this, _1, slider.id));
    if (i == 0) slider.widget->SetFocus();
    tacticsSliders.push_back(slider);
    i++;
    iter++;
  }

  tacticsMenu->Show();
}

void GamePlanPage::SaveTactics() {

  if (UpdateNonImportableDB()) {
    // saves to temp names db, which is used when importing the actual db.

    std::string tactics_xml;

    for (unsigned int i = 0; i < tacticsSliders.size(); i++) {
      tactics_xml += "<" + tacticsSliders.at(i).tacticName + ">" + real_to_str(tacticsSliders.at(i).widget->GetValue()) + "</" + tacticsSliders.at(i).tacticName + ">\n";
    }
    printf("tactics:\n%s\n", tactics_xml.c_str());

    // find club
    DatabaseResult *result = namedb->Query("select id from clubnames where faketargetname = \"" + teamData->GetName() + "\" limit 1;");
    int teamDatabaseID = -1;
    if (result->data.size() > 0) {
      teamDatabaseID = atoi(result->data.at(0).at(0).c_str());
      delete result;
      result = namedb->Query("update clubnames set tactics_xml = \"" + tactics_xml + "\" where id = " + int_to_str(teamDatabaseID) + ";");
      delete result;
    } else { // team does not yet exist in namedb
      if (Verbose()) printf("WARNING: team does not exist in namedb: %s\n", teamData->GetName().c_str());
      delete result;
    }
  }

  teamData->SaveTactics();
}

void GamePlanPage::TacticsMenuOnChange(Gui2Slider *slider, int id) {
  //printf("slider %i (%s) altered\n", id, slider->GetName().c_str());
  Properties &userProps = teamData->GetTacticsWritable().userProperties;
  userProps.Set(tacticsSliders.at(id).tacticName.c_str(), tacticsSliders.at(id).widget->GetValue());
}
