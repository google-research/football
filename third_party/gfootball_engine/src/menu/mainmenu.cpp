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

#include "mainmenu.hpp"

#include "../main.hpp"
#include "controllerselect.hpp"
#include "settings.hpp"
#include "credits.hpp"

#include "pagefactory.hpp"

#include "../blunted.hpp"

#include <boost/algorithm/string.hpp>
#include <cmath>
#include <set>

using namespace blunted;

IntroPage::IntroPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  windowManager->BlackoutBackground(true);

  Gui2Root *root = windowManager->GetRoot();

  bg = new Gui2Image(windowManager, "image_intro", 0, 0, 100, 100);
  bg->LoadImage("media/menu/backgrounds/intro01.png");

  this->AddView(bg);
  bg->Show();

  this->SetFocus();

  this->Show();
}

IntroPage::~IntroPage() {
  windowManager->BlackoutBackground(false);
}

void IntroPage::ProcessWindowingEvent(WindowingEvent *event) {
  if (event->IsEscape() || event->IsActivate()) {

    CreatePage((int)e_PageID_MainMenu, 0);
    return;
  }
}

void IntroPage::ProcessKeyboardEvent(KeyboardEvent *event) {
  CreatePage((int)e_PageID_MainMenu, 0);
}


OutroPage::OutroPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  windowManager->BlackoutBackground(true);

  Gui2Root *root = windowManager->GetRoot();

  bg = new Gui2Image(windowManager, "image_outro", 0, 0, 100, 100);
  bg->LoadImage("media/menu/backgrounds/outro01.png");

  this->AddView(bg);
  bg->Show();

  this->SetFocus();

  this->Show();
}

OutroPage::~OutroPage() {
  windowManager->BlackoutBackground(false);
}

void OutroPage::ProcessWindowingEvent(WindowingEvent *event) {
  if (event->IsEscape() || event->IsActivate()) {

    GetMenuTask()->QuitGame();
  }
}

void OutroPage::ProcessKeyboardEvent(KeyboardEvent *event) {
  GetMenuTask()->QuitGame();
}


MainMenuPage::MainMenuPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  Gui2Image *title = new Gui2Image(windowManager, "image_main_title", 28, 32, 44, 20);
  title->LoadImage("media/menu/main/title01.png");

  this->AddView(title);
  title->Show();

  buttons.push_back(new Gui2Button(windowManager, "button_main_start", 0, 0, 20, 3, "Match"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_cup", 0, 0, 20, 3, "Cup"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_league", 0, 0, 20, 3, "League"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_edit", 0, 0, 20, 3, "Editor"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_settings", 0, 0, 20, 3, "Settings"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_credits", 0, 0, 20, 3, "Credits"));
  buttons.push_back(new Gui2Button(windowManager, "button_main_quit", 0, 0, 20, 3, "Exit"));
  if (!IsReleaseVersion()) {
    buttons.push_back(new Gui2Button(windowManager, "button_main_import", 0, 0, 20, 3, "Import FM"));
  }

  buttons.at(0)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoControllerSelect, this));
  buttons.at(2)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoLeague, this));
  buttons.at(4)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoSettings, this));
  buttons.at(5)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoCredits, this));
  if (!IsReleaseVersion()) {
    buttons.at(6)->sig_OnClick.connect(boost::bind(&MenuTask::QuitGame, GetMenuTask()));
  } else {
    buttons.at(6)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoOutro, this));
  }
  if (!IsReleaseVersion()) {
    buttons.at(7)->sig_OnClick.connect(boost::bind(&MainMenuPage::GoImportDB, this));
  }

  buttons.at(1)->SetActive(false);
  buttons.at(2)->SetActive(false);
  buttons.at(3)->SetActive(false);

  grid = new Gui2Grid(windowManager, "grid_main", 29.25, 52, 41.5, 40);

  grid->AddView(buttons.at(0), 0, 0);
  grid->AddView(buttons.at(1), 1, 0);
  grid->AddView(buttons.at(2), 2, 0);
  grid->AddView(buttons.at(3), 3, 0);
  grid->AddView(buttons.at(4), 0, 1);
  grid->AddView(buttons.at(5), 1, 1);
  grid->AddView(buttons.at(6), 2, 1);
  if (!IsReleaseVersion()) grid->AddView(buttons.at(7), 3, 1);

  grid->UpdateLayout(0.25, 0.25, 0.25, 0.25);

  this->AddView(grid);
  grid->Show();

  buttons.at(pageData.properties->GetInt("selectedButtonID"))->SetFocus();

  this->Show();
}

MainMenuPage::~MainMenuPage() {
}

void MainMenuPage::GoControllerSelect() {

  pageData.properties->Set("selectedButtonID", 0);
  Properties properties;
  properties.SetBool("isInGame", false);
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_ControllerSelect, properties, 0);
}

void MainMenuPage::GoLeague() {
  pageData.properties->Set("selectedButtonID", 2);
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_League_Start, properties, 0);
}

void MainMenuPage::GoSettings() {
  pageData.properties->Set("selectedButtonID", 4);
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_Settings, properties, 0);
}

void MainMenuPage::GoCredits() {
  pageData.properties->Set("selectedButtonID", 5);
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_Credits, properties, 0);
}

void MainMenuPage::GoOutro() {
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_Outro, properties, 0);
}

bool SortClubPlayersByAverageStat(const PlayerImport &a, const PlayerImport &b) {
  if (a.averageStat < b.averageStat) return true; else return false;
  return false;
}

bool SortClubPlayersByRole(const PlayerImport &a, const PlayerImport &b) {
  float aDef = 0;
  float aAtt = 0;
  float bDef = 0;
  float bAtt = 0;
  int defPos = -1;
  int attPos = -1;
  for (unsigned int statPos = 0; statPos < a.profileStats.size(); statPos++) {
    if (a.profileStats.at(statPos).name.compare("mental_defensivepositioning") == 0) { aDef = a.profileStats.at(statPos).value; defPos = statPos; }
    if (a.profileStats.at(statPos).name.compare("mental_offensivepositioning") == 0) { aAtt = a.profileStats.at(statPos).value; attPos = statPos; }
    if (defPos != -1 && attPos != -1) break;
  }

  // statPositions are the same for each player, so use cached pos
  bDef = b.profileStats.at(defPos).value;
  bAtt = b.profileStats.at(attPos).value;

  float aRole = aAtt - aDef;
  float bRole = bAtt - bDef;

  if (aRole < bRole) return true; else return false;
  return false;
}

std::string RenameLeague(Database *namedb, const std::string &name, const std::string &srcfield) {
  std::string query = "select " + srcfield + " from leaguenames where name = \"" + name + "\" limit 1;";
  DatabaseResult *result = namedb->Query(query);
  std::string targetName;
  if (result->data.size() > 0) {
    targetName = result->data.at(0).at(0);
  } else targetName = name;
  delete result;
  return targetName;
}

std::string RenameClub(Database *namedb, const std::string &name, const std::string &srcfield) {
  std::string query = "select " + srcfield + " from clubnames where name = \"" + name + "\" limit 1;";
  DatabaseResult *result = namedb->Query(query);
  std::string targetName;
  if (result->data.size() > 0) {
    targetName = result->data.at(0).at(0);
  } else targetName = name;
  delete result;
  return targetName;
}

ClubData GetClubData(Database *namedb, const std::string &name) {
  std::string query = "select formation_xml, tactics_xml, shortname, color1, color2 from clubnames where name = \"" + name + "\" limit 1;";
  DatabaseResult *result = namedb->Query(query);
  ClubData data;
  if (result->data.size() > 0) {
    data.formation_xml = result->data.at(0).at(0);
    data.tactics_xml = result->data.at(0).at(1);
    data.shortName = result->data.at(0).at(2);
    data.color1 = result->data.at(0).at(3);
    data.color2 = result->data.at(0).at(4);
  } else {
    data.formation_xml = "";
    data.tactics_xml = "";
    data.shortName = name;
    data.shortName.erase(remove_if(data.shortName.begin(), data.shortName.end(), isspace), data.shortName.end());
    data.shortName = boost::to_upper_copy(data.shortName.substr(0, 3));
    data.color1 = "40, 40, 40";
    data.color2 = "200, 200, 200";
  }
  delete result;
  return data;
}

void GetPlayerData(Database *namedb, const std::string &firstname, const std::string &lastname, std::string &fakefirstname_ret, std::string &fakelastname_ret, int &skincolor_ret, std::string &hairstyle_ret, std::string &haircolor_ret, float &height_ret, float &weight_ret, signed int &formationorder_ret, float &baseStatOffset_ret, std::string &customProfile_ret) {

  skincolor_ret = 0;
  hairstyle_ret = "";
  haircolor_ret = "";
  height_ret = 0.0f;
  weight_ret = 0.0f;

  std::string query = "select fakefirstname, fakelastname, skincolor, hairstyle, haircolor, height, weight, formationorder, base_stat_offset, profile_xml from playernames where firstname = \"" + firstname + "\" and lastname = \"" + lastname + "\" limit 1;";
  DatabaseResult *result = namedb->Query(query);
  if (result->data.size() > 0 && atoi(result->data.at(0).at(2).c_str()) != 0) {
    fakefirstname_ret =       result->data.at(0).at(0);
    fakelastname_ret =        result->data.at(0).at(1);
    skincolor_ret =      atoi(result->data.at(0).at(2).c_str());
    hairstyle_ret =           result->data.at(0).at(3);
    haircolor_ret =           result->data.at(0).at(4);
    height_ret =         atof(result->data.at(0).at(5).c_str());
    weight_ret =         atof(result->data.at(0).at(6).c_str());
    formationorder_ret = atoi(result->data.at(0).at(7).c_str());
    baseStatOffset_ret = atof(result->data.at(0).at(8).c_str());
    customProfile_ret =       result->data.at(0).at(9);
  } else {
    fakefirstname_ret =  firstname;
    fakelastname_ret =   lastname;
    formationorder_ret = -1;
    baseStatOffset_ret = 0.0f;
    customProfile_ret =  "";
  }

  if (skincolor_ret == 0) skincolor_ret = int(std::round(random(1, 4)));
  if (hairstyle_ret == "") hairstyle_ret = "short01";
  if (haircolor_ret == "") haircolor_ret = "black";
  if (height_ret == 0.0f)  height_ret    = 1.8f;
  if (weight_ret == 0.0f)  weight_ret    = 80.0f;

  delete result;
}


/* THIS CODE IMPORTS FM DATABASES, exported with some FM management tool, of which I forgot the name. maybe it's useful for somebody. */

bool MainMenuPage::GoImportDB() {

  Database *namedb = new Database();
  bool dbSuccess = namedb->Load("databases/names.sqlite");
  if (!dbSuccess) Log(e_FatalError, "MainMenuPage", "GoImportDB", "Could not open database");


  // create database

  std::string query = "begin transaction;";
  query += "drop table regions;";
  query += "drop table countries;";
  query += "drop table leagues;";
  query += "drop table teams;";
  query += "drop table players;";
  query += "drop table tournaments;";
  query += "drop table tournamentdata;";
  query += "commit;";
  DatabaseResult *result = GetDB()->Query(query);
  delete result;

  query = "vacuum;"; // *woooooshhh* http://www.sqlite.org/lang_vacuum.html
  result = GetDB()->Query(query);
  delete result;

  result = GetDB()->Query("CREATE TABLE regions(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                               "name VARCHAR(64));");
  delete result;

  result = GetDB()->Query("CREATE TABLE countries(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                 "region_id INTEGER, "
                                                 "name VARCHAR(64));");
  delete result;

  result = GetDB()->Query("CREATE TABLE leagues(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                               "country_id INTEGER, "
                                               "name VARCHAR(64), "
                                               "logo_url VARCHAR(512));");
  delete result;

  result = GetDB()->Query("CREATE TABLE teams(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                             "league_id INTEGER, "
                                             "name VARCHAR(64), "
                                             "logo_url VARCHAR(512), "
                                             "kit_url VARCHAR(512), "
                                             "formation_xml TEXT, "
                                             "formation_factory_xml TEXT, "
                                             "tactics_xml TEXT, "
                                             "tactics_factory_xml TEXT, "
                                             "shortname VARCHAR(3), "
                                             "color1 VARCHAR(16), "
                                             "color2 VARCHAR(16));");
  delete result;

  result = GetDB()->Query("CREATE TABLE players(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                               "team_id INTEGER, "
                                               "nationalteam_id INTEGER, "
                                               "firstname VARCHAR(64), "
                                               "lastname VARCHAR(64), "
                                               "role VARCHAR(32), "
                                               "age INTEGER, "
                                               "base_stat FLOAT, "
                                               "profile_xml TEXT, "
                                               "skincolor INTEGER, "
                                               "hairstyle VARCHAR(64), "
                                               "haircolor VARCHAR(64), "
                                               "height FLOAT, "
                                               "weight FLOAT, "
                                               "formationorder INTEGER, "
                                               "nationalteamformationorder INTEGER);");
  delete result;

  result = GetDB()->Query("CREATE TABLE tournaments(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                   "name VARCHAR(64), "
                                                   "location_type INTEGER, " // 1 = regional, 2 = national, 3 = league
                                                   "location_id INTEGER, "
                                                   "has_knockoutphase INTEGER, "
                                                   "has_groupphase INTEGER, "
                                                   "prizemoney INTEGER);");
  delete result;

  result = GetDB()->Query("CREATE TABLE tournamentdata(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                      "tournament_id INTEGER, "
                                                      "year INTEGER, "
                                                      "xmldata TEXT);");
  delete result;

  //std::string defaultStats = "<defense>0.8</defense><bodybalance>0.8</bodybalance><acceleration>0.6</acceleration><velocity>0.6</velocity><agility>0.8</agility><reaction>1.0</reaction><ballcontrol>0.8</ballcontrol><dribblevelocity>0.6</dribblevelocity><shotpower>0.8</shotpower><passtechnique>1.0</passtechnique><tactics>1.0</tactics><condition>1.0</condition>";
  //std::string defaultFormation = "<p1><position>-1, 0</position><role>GK</role></p1><p2><position>-0.6, 0.6</position><role>LB</role></p2><p3><position>-0.7, 0.2</position><role>CB</role></p3><p4><position>-0.7, -0.2</position><role>CB</role></p4><p5><position>-0.6, -0.6</position><role>RB</role></p5><p6><position>0.0, 0.8</position><role>LM</role></p6><p7><position>0.1, 0.0</position><role>AM</role></p7><p8><position>0.0, -0.8</position><role>RM</role></p8><p9><position>0.7, 0.5</position><role>CF</role></p9><p10><position>0.8, 0.0</position><role>CF</role></p10><p11><position>0.7, -0.5</position><role>CF</role></p11>";

/* possession game, high pressure
<dribble_centermagnet>0.5</dribble_centermagnet>
<dribble_offensiveness>0.5</dribble_offensiveness>

<position_defense_depth_factor>0.3</position_defense_depth_factor>
<position_defense_microfocus_strength>0.8</position_defense_microfocus_strength>
<position_defense_midfieldfocus>0.8</position_defense_midfieldfocus>
<position_defense_sidefocus_strength>0.7</position_defense_sidefocus_strength>
<position_defense_width_factor>0.3</position_defense_width_factor>

<position_offense_depth_factor>0.6</position_offense_depth_factor>
<position_offense_microfocus_strength>0.8</position_offense_microfocus_strength>
<position_offense_midfieldfocus>0.8</position_offense_midfieldfocus>
<position_offense_sidefocus_strength>0.4</position_offense_sidefocus_strength>
<position_offense_width_factor>0.6</position_offense_width_factor>
*/

/* counter
<dribble_centermagnet>0.7</dribble_centermagnet>
<dribble_offensiveness>0.9</dribble_offensiveness>

<position_defense_depth_factor>0.3</position_defense_depth_factor>
<position_defense_microfocus_strength>0.3</position_defense_microfocus_strength>
<position_defense_midfieldfocus>0.2</position_defense_midfieldfocus>
<position_defense_sidefocus_strength>0.3</position_defense_sidefocus_strength>
<position_defense_width_factor>0.6</position_defense_width_factor>

<position_offense_depth_factor>0.9</position_offense_depth_factor>
<position_offense_microfocus_strength>0.4</position_offense_microfocus_strength>
<position_offense_midfieldfocus>0.7</position_offense_midfieldfocus>
<position_offense_sidefocus_strength>0.3</position_offense_sidefocus_strength>
<position_offense_width_factor>0.3</position_offense_width_factor>
*/

/* balanced offensive
<dribble_centermagnet>0.6</dribble_centermagnet>
<dribble_offensiveness>0.8</dribble_offensiveness>

<position_defense_depth_factor>0.6</position_defense_depth_factor>
<position_defense_microfocus_strength>0.5</position_defense_microfocus_strength>
<position_defense_midfieldfocus>0.7</position_defense_midfieldfocus>
<position_defense_sidefocus_strength>0.5</position_defense_sidefocus_strength>
<position_defense_width_factor>0.5</position_defense_width_factor>

<position_offense_depth_factor>0.6</position_offense_depth_factor>
<position_offense_microfocus_strength>0.5</position_offense_microfocus_strength>
<position_offense_midfieldfocus>0.8</position_offense_midfieldfocus>
<position_offense_sidefocus_strength>0.5</position_offense_sidefocus_strength>
<position_offense_width_factor>0.7</position_offense_width_factor>
*/

/* balanced defensive
<dribble_centermagnet>0.3</dribble_centermagnet>
<dribble_offensiveness>0.2</dribble_offensiveness>

<position_defense_depth_factor>0.4</position_defense_depth_factor>
<position_defense_microfocus_strength>0.6</position_defense_microfocus_strength>
<position_defense_midfieldfocus>0.3</position_defense_midfieldfocus>
<position_defense_sidefocus_strength>0.4</position_defense_sidefocus_strength>
<position_defense_width_factor>0.4</position_defense_width_factor>

<position_offense_depth_factor>0.6</position_offense_depth_factor>
<position_offense_microfocus_strength>0.5</position_offense_microfocus_strength>
<position_offense_midfieldfocus>0.5</position_offense_midfieldfocus>
<position_offense_sidefocus_strength>0.5</position_offense_sidefocus_strength>
<position_offense_width_factor>0.5</position_offense_width_factor>
*/


/*
<p1><position>-1.0,  0.0</position><role>GK</role></p1>
<p2><position>-0.6,  0.7</position><role>LB</role></p2>
<p3><position>-0.7,  0.2</position><role>CB</role></p3>
<p4><position>-0.7, -0.2</position><role>CB</role></p4>
<p5><position>-0.6, -0.7</position><role>RB</role></p5>
<p6><position> 0.1,  0.8</position><role>LM</role></p6>
<p7><position>-0.1,  0.0</position><role>DM</role></p7>
<p8><position> 0.1, -0.8</position><role>RM</role></p8>
<p9><position> 0.5,  0.0</position><role>AM</role></p9>
<p10><position>0.8,  0.3</position><role>CF</role></p10>
<p11><position>0.8, -0.3</position><role>CF</role></p11>
*/

  // 4-2-3-1
  std::string defaultFormation = "<p1><position>-1.0,  0.0</position><role>GK</role></p1>"

                                 "<p2><position>-0.7,  0.75</position><role>LB</role></p2>"
                                 "<p3><position>-1.0,  0.25</position><role>CB</role></p3>"
                                 "<p4><position>-1.0, -0.25</position><role>CB</role></p4>"
                                 "<p5><position>-0.7, -0.75</position><role>RB</role></p5>"

                                 "<p6><position>-0.2,  0.3</position><role>CM</role></p6>"
                                 "<p7><position>-0.2, -0.3</position><role>CM</role></p7>"

                                 "<p8><position> 0.7,  0.9</position><role>LM</role></p8>"
                                 "<p9><position> 0.2,  0.0</position><role>AM</role></p9>"
                                 "<p10><position>0.7, -0.9</position><role>RM</role></p10>"

                                 "<p11><position>1.0,  0.0</position><role>CF</role></p11>";

  //<p1><position>-1.0,  0.0</position><role>GK</role></p1><p2><position>-0.7,  0.75</position><role>LB</role></p2><p3><position>-1.0,  0.25</position><role>CB</role></p3><p4><position>-1.0, -0.25</position><role>CB</role></p4><p5><position>-0.7, -0.75</position><role>RB</role></p5><p6><position>-0.2,  0.3</position><role>CM</role></p6><p7><position>-0.2, -0.3</position><role>CM</role></p7><p8><position> 0.7,  0.9</position><role>LM</role></p8><p9><position> 0.2,  0.0</position><role>AM</role></p9><p10><position>0.7, -0.9</position><role>RM</role></p10><p11><position>1.0,  0.0</position><role>CF</role></p11>

  /*
  // 4-4-2
  std::string defaultFormation = "<p1><position>-1.0,  0.0</position><role>GK</role></p1>"

                                 "<p2><position>-0.7,  0.6</position><role>LB</role></p2>"
                                 "<p3><position>-0.8,  0.2</position><role>CB</role></p3>"
                                 "<p4><position>-0.8, -0.2</position><role>CB</role></p4>"
                                 "<p5><position>-0.7, -0.6</position><role>RB</role></p5>"

                                 "<p6><position> 0.0,  0.9</position><role>LM</role></p6>"
                                 "<p7><position>-0.3,  0.0</position><role>DM</role></p7>"
                                 "<p8><position> 0.0, -0.9</position><role>RM</role></p8>"
                                 "<p9><position> 0.3,  0.0</position><role>AM</role></p9>"

                                 "<p10><position>0.8,  0.3</position><role>CF</role></p10>"
                                 "<p11><position>0.8, -0.3</position><role>CF</role></p11>";

  //<p1><position>-1.0,  0.0</position><role>GK</role></p1><p2><position>-0.7,  0.6</position><role>LB</role></p2><p3><position>-0.8,  0.2</position><role>CB</role></p3><p4><position>-0.8, -0.2</position><role>CB</role></p4><p5><position>-0.7, -0.6</position><role>RB</role></p5><p6><position> 0.0,  0.9</position><role>LM</role></p6><p7><position>-0.3,  0.0</position><role>DM</role></p7><p8><position> 0.0, -0.9</position><role>RM</role></p8><p9><position> 0.3,  0.0</position><role>AM</role></p9><p10><position>0.8,  0.3</position><role>CF</role></p10><p11><position>0.8, -0.3</position><role>CF</role></p11>
  */

  /*
  // 4-3-3
  std::string defaultFormation = "<p1><position> -1.0,  0.0 </position><role>GK</role></p1>"

                                 "<p2><position> -0.7,  0.75</position><role>LB</role></p2>"
                                 "<p3><position> -1.0,  0.25</position><role>CB</role></p3>"
                                 "<p4><position> -1.0, -0.25</position><role>CB</role></p4>"
                                 "<p5><position> -0.7, -0.75</position><role>RB</role></p5>"

                                 "<p6><position>  0.0,  0.5 </position><role>CM</role></p6>"
                                 "<p7><position> -0.2,  0.0 </position><role>CM</role></p7>"
                                 "<p8><position>  0.0, -0.5 </position><role>CM</role></p8>"

                                 "<p9><position>  0.6,  0.75 </position><role>LM</role></p9>"
                                 "<p10><position> 1.0,  0.0 </position><role>CF</role></p10>"
                                 "<p11><position> 0.6, -0.75 </position><role>RM</role></p11>";

  //<p1><position> -1.0,  0.0 </position><role>GK</role></p1><p2><position> -0.7,  0.75</position><role>LB</role></p2><p3><position> -1.0,  0.25</position><role>CB</role></p3><p4><position> -1.0, -0.25</position><role>CB</role></p4><p5><position> -0.7, -0.75</position><role>RB</role></p5><p6><position>  0.0,  0.5 </position><role>CM</role></p6><p7><position> -0.2,  0.0 </position><role>CM</role></p7><p8><position>  0.0, -0.5 </position><role>CM</role></p8><p9><position>  0.6,  0.75 </position><role>LM</role></p9><p10><position> 1.0,  0.0 </position><role>CF</role></p10><p11><position> 0.6, -0.75 </position><role>RM</role></p11>
  */

  std::string defaultTactics = "<position_offense_depth_factor>0.5</position_offense_depth_factor>"
                               "<position_defense_depth_factor>0.5</position_defense_depth_factor>"
                               "<position_offense_width_factor>0.5</position_offense_width_factor>"
                               "<position_defense_width_factor>0.5</position_defense_width_factor>"

                               "<position_offense_midfieldfocus>0.5</position_offense_midfieldfocus>"
                               "<position_defense_midfieldfocus>0.5</position_defense_midfieldfocus>"

                               "<position_offense_sidefocus_strength>0.5</position_offense_sidefocus_strength>"
                               "<position_defense_sidefocus_strength>0.5</position_defense_sidefocus_strength>"

                               "<position_offense_microfocus_strength>0.5</position_offense_microfocus_strength>"
                               "<position_defense_microfocus_strength>0.5</position_defense_microfocus_strength>"

                               "<dribble_offensiveness>0.5</dribble_offensiveness>"
                               "<dribble_centermagnet>0.5</dribble_centermagnet>";

  InitDefaultProfiles();


  // LOAD CLUBS

  std::vector<std::string> data;
  file_to_vector("databases/fm15_clubs.csv", data);

  std::set<std::string> countries;

  std::vector<Club> clubs;

  // put these in a map with country + competition (as string) as key to get only unique elements
  std::map<std::string, CountryCompetition> uniqueCompetitions;

  int id = 1;
  for (unsigned int i = 1; i < data.size(); i++) { // (skip header)
    std::vector<std::string> tokens;
    tokenize(data.at(i), tokens, ";");

    for (unsigned int t = 0; t < tokens.size(); t++) {
      tokens.at(t).erase( std::remove(tokens.at(t).begin(), tokens.at(t).end(), '\"'), tokens.at(t).end() );
      //printf("[%s]", tokens.at(t).c_str());
    }

    Club club;
    club.id = id;
    club.name = tokens.at(0);
    club.country = tokens.at(1);
    club.competition = tokens.at(2);
    club.reputation = atoi(tokens.at(3).c_str());
    club.balance = atoi(tokens.at(5).c_str());
    club.status = tokens.at(9);

    bool addClub = false;

    /* public beta 1
    if ((club.country.compare("England") == 0 && club.competition.compare("Premier Division") == 0) ||
        (club.country.compare("Germany") == 0 && club.competition.compare("First Division") == 0) ||
        (club.country.compare("Holland") == 0 && club.competition.compare("Eredivisie") == 0) ||
        (club.country.compare("Spain")   == 0 && club.competition.compare("LIGA BBVA") == 0)) {
      addClub = true;
    }
    */

    // public beta 2
    if (club.name.compare("PSV") == 0) addClub = true;
    if (club.name.compare("Ajax") == 0) addClub = true;
    if (club.name.compare("Dortmund") == 0) addClub = true;
    if (club.name.compare("Bayern") == 0) addClub = true;
    if (club.name.compare("R. Madrid") == 0) addClub = true;
    if (club.name.compare("Barcelona") == 0) addClub = true;
    if (club.name.compare("Arsenal") == 0) addClub = true;
    if (club.name.compare("Man Utd") == 0) addClub = true;

    //printf("club: %s, rep: %i\n", club.name.c_str(), club.reputation);

    if (addClub && club.status.compare("Professional") == 0) {
      countries.insert(tokens.at(1)); // sets only support unique elements so there'll be no dupes

      CountryCompetition cc;
      cc.country = tokens.at(1);
      cc.competition = tokens.at(2);
      uniqueCompetitions.insert(std::pair<std::string, CountryCompetition>(tokens.at(1) + tokens.at(2), cc)); // dito with map indices

      clubs.push_back(club);
      id++;
    }

    //printf("\n");
  }


  // insert region(s)

  query = "begin transaction; delete from regions; delete from sqlite_sequence where name=\"regions\";";
  query += "insert into regions (name) values (\"Europe\");";
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;


  // insert countries

  // map for looking up countries' db indices later
  std::map<std::string, int> countryIDs;

  query = "begin transaction; delete from countries; delete from sqlite_sequence where name=\"countries\";";
  std::set<std::string>::iterator iter = countries.begin();
  for (unsigned int i = 0; i < countries.size(); i++) {
    query += "insert into countries (id, region_id, name) values (" + int_to_str(i + 1) + ", 1, \"" + *iter + "\");";
    countryIDs.insert(std::pair<std::string, int>(*iter, i + 1));
    iter++;
  }
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;


  // insert leagues

  // map for looking up competitions' db indices later
  std::map<std::string, int> competitionIDs;

  query = "begin transaction; delete from leagues; delete from sqlite_sequence where name=\"leagues\";";
  std::map<std::string, CountryCompetition>::iterator compIter = uniqueCompetitions.begin();
  id = 1;
  while (compIter != uniqueCompetitions.end()) {
    std::map<std::string, int>::iterator countryIter = countryIDs.find(compIter->second.country);
    if (countryIter != countryIDs.end()) {
      int countryID = countryIter->second;
      std::string competitionName = RenameLeague(namedb, compIter->second.competition, "faketargetname");
      std::string strippedCompetitionName = StripString(competitionName);
      std::string competitionFilename = RenameLeague(namedb, compIter->second.competition, "targetname");
      std::string strippedCompetitionFilename = StripString(competitionFilename);
      std::transform(strippedCompetitionFilename.begin(), strippedCompetitionFilename.end(), strippedCompetitionFilename.begin(), ::tolower);

      printf("adding league: orig %s - renamed %s - stripped %s\n", compIter->second.competition.c_str(), competitionName.c_str(), strippedCompetitionName.c_str());

      query += "insert into leagues (id, name, logo_url, country_id) values (" + int_to_str(id) + ", \"" + competitionName + "\", \"images_competitions/" + strippedCompetitionFilename + ".png\", " + int_to_str(countryID) + ");";
      competitionIDs.insert(std::pair<std::string, int>(compIter->second.country + compIter->second.competition, id));
      id++;
    }
    compIter++;
  }
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;


  // insert clubs

  // map for looking up clubs' db indices later
  std::map<std::string, int> clubIDs;

  query = "begin transaction; delete from teams; delete from sqlite_sequence where name=\"teams\";";
  for (unsigned int i = 0; i < clubs.size(); i++) {
    int competitionID = competitionIDs.find(clubs.at(i).country + clubs.at(i).competition)->second;
    std::string competitionFilename = RenameLeague(namedb, clubs.at(i).competition, "targetname");
    std::string strippedCompetitionFilename = StripString(competitionFilename);
    std::transform(strippedCompetitionFilename.begin(), strippedCompetitionFilename.end(), strippedCompetitionFilename.begin(), ::tolower);
    std::string clubName = RenameClub(namedb, clubs.at(i).name, "faketargetname");
    std::string strippedClubName = StripString(clubName);
    std::string clubFilename = RenameClub(namedb, clubs.at(i).name, "targetname");
    std::string strippedClubFilename = StripString(clubFilename);
    std::transform(strippedClubFilename.begin(), strippedClubFilename.end(), strippedClubFilename.begin(), ::tolower);

    printf("adding team: orig %s - renamed %s - stripped %s\n", clubs.at(i).name.c_str(), clubName.c_str(), strippedClubName.c_str());

    ClubData clubData = GetClubData(namedb, clubs.at(i).name);
    if (clubData.formation_xml.compare("") == 0) clubData.formation_xml = defaultFormation;
    if (clubData.tactics_xml.compare("") == 0) clubData.tactics_xml = defaultTactics;

    //query += "insert into teams (id, league_id, name, logo_url, kit_url, formation) values (" + int_to_str(clubs.at(i).id) + ", " + int_to_str(competitionID) + ", \"" + clubs.at(i).name + "\", \"images_teams/" + strippedCompetition + "/" + strippedTeam + "_logo.png\", \"images_teams/" + strippedCompetition + "/" + strippedTeam + "_kit.png\", \"" + defaultFormation + "\");";
    query += "insert into teams (id, league_id, name, logo_url, kit_url, formation_xml, formation_factory_xml, tactics_xml, tactics_factory_xml, shortname, color1, color2) values (" + int_to_str(clubs.at(i).id) + ", " + int_to_str(competitionID) + ", \"" + clubName + "\", \"images_teams/" + strippedCompetitionFilename + "/" + strippedClubFilename + "_logo.png\", \"images_teams/" + strippedCompetitionFilename + "/" + strippedClubFilename + "\", \"" + clubData.formation_xml + "\", \"" + clubData.formation_xml + "\", \"" + clubData.tactics_xml + "\", \"" + clubData.tactics_xml + "\", \"" + clubData.shortName + "\", \"" + clubData.color1 + "\", \"" + clubData.color2 + "\");";
    clubIDs.insert(std::pair<std::string, int>(clubs.at(i).name, clubs.at(i).id));
  }
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;


  // insert players

  std::vector<std::string> players;
  file_to_vector("databases/fm15_players.csv", players);

  std::vector<PlayerImport> importedPlayers;

  int playerID = 1;

  for (unsigned int i = 1; i < players.size(); i++) { // (skip header)
    std::vector<std::string> tokens;
    tokenize(players.at(i), tokens, ";");

    for (unsigned int t = 0; t < tokens.size(); t++) {
      // remove quotes
      tokens.at(t).erase( std::remove(tokens.at(t).begin(), tokens.at(t).end(), '\"'), tokens.at(t).end() );
      //printf("[%s]", tokens.at(t).c_str());
    }

    // remove commas (from value)
    tokens.at(11).erase( std::remove(tokens.at(11).begin(), tokens.at(11).end(), ','), tokens.at(11).end() );

    PlayerImport player;
    player.age = atoi(tokens.at(7).c_str());
    player.value = atoi(tokens.at(11).c_str());

    std::map<std::string, int>::iterator club = clubIDs.find(tokens.at(3));
    if (club != clubIDs.end() && player.value > 0 && player.age >= 15 && player.age <= 40) { // only add player if he's with a club that's in the database

      player.id = playerID;
      playerID++;

      std::string name = tokens.at(0);
      player.nationality = tokens.at(1);
      player.position = tokens.at(2);
      //printf("%i\n", player.value);

      // name into first/last name
      std::size_t comma = name.find_first_of(",");
      if (comma == std::string::npos) {
        player.firstName = "";
        player.lastName = name;
      } else {
        player.lastName = name.substr(0, comma);
        player.firstName = name.substr(comma + 2, name.size() - (comma + 2));
      }
      //printf("[%s][%s]\n", player.firstName.c_str(), player.lastName.c_str());

      player.clubID = club->second;


      // save age/value for making statistics
      //XX Import_AgeValueStatsAdd(player.age, player.value);

      std::vector<WeightedPosition> weightedPositions;
      GetWeightedPositions(player.position, weightedPositions);
      assert(weightedPositions.size() > 0);

      std::vector<Stat> profileStats;
      GetDefaultProfile(weightedPositions, profileStats);

      //XX float averageStat = GetAverageStatFromValue(player.age, player.value);
      player.averageStat = NormalizedClamp(atof(tokens.at(13).substr(0, 5).c_str()) * 1.0f - 10, 0, 100); // don't go full range - the hypothetical best player ever doesn't exist in real life (yet? ;))
      player.averageStartStat = NormalizedClamp(atof(tokens.at(14).substr(0, 5).c_str()) * 1.0f - 10, 0, 100);
      // youngsters are underrated in FM db
      player.averageStat *= 1.0f - curve(NormalizedClamp(abs(player.age - 18), 0, 10), 1.0f) * 0.1f;
      player.averageStartStat *= 1.0f - curve(NormalizedClamp(abs(player.age - 18), 0, 10), 1.0f) * 0.1f;

      player.profileStats = profileStats;

      clubs.at(player.clubID - 1).players.push_back(player);
      importedPlayers.push_back(player);
    }
  }

  //Import_ProcessAgeValueStats();
  //XX float averageStatAt15 = averageStatPerAge.find(15)->second;

  std::map<float, int> bestYoungPlayersMap;

  query = "begin transaction; delete from players; delete from sqlite_sequence where name=\"players\";";
  for (unsigned int i = 0; i < importedPlayers.size(); i++) {

    PlayerImport &player = importedPlayers.at(i);

    //XX float averageStatAtCurrentAge = averageStatPerAge.find(player.age)->second;
    //XX player.averageStartStat = (player.averageStat / averageStatAtCurrentAge) * averageStatAt15;
    //float averageStartStat = ((player.averageStat / averageStatAtCurrentAge * 0.7) + player.averageStat * 0.3) * averageStatAt15;
    //float averageStartStat = player.averageStat * averageStatAt15;
    //printf("stat: %f (%f), age: %i (%s)\n", player.averageStartStat, player.averageStat, player.age, player.lastName.c_str());

    /*
    // TMP HAX, this should be done 'realtime' ingame
    for (unsigned int j = 0; j < player.profileStats.size(); j++) {
      //player.profileStats.at(j).value *= player.averageStartStat;
      //player.profileStats.at(j).value = GetIndividualStat(player.profileStats.at(j).value)

      player.profileStats.at(j).value = GetIndividualStat(player.averageStat, player.profileStats.at(j).value, player.age);
    }
    */

    std::string statsString = GetProfileString(player.profileStats);

    std::string firstName;
    std::string lastName;
    // deprecated RenamePlayer(namedb, player.firstName, player.lastName, firstName, lastName);
    int skinColor = 0;
    std::string hairStyle;
    std::string hairColor;
    float height = 0.0f;
    float weight = 0.0f;
    signed int formationOrder = -1;
    float baseStatOffset = 0.0f;
    std::string customProfileXML;
    GetPlayerData(namedb, player.firstName, player.lastName, firstName, lastName, skinColor, hairStyle, hairColor, height, weight, formationOrder, baseStatOffset, customProfileXML);
    if (customProfileXML.size() > 0) statsString = customProfileXML;
    player.averageStartStat = clamp(player.averageStartStat + baseStatOffset, 0.01f, 1.0f);
    player.averageStat = clamp(player.averageStat + baseStatOffset, 0.01f, 1.0f);

    bestYoungPlayersMap.insert(std::pair<float, int>(player.averageStartStat, i));

    printf("converted: %s to %s\n", player.lastName.c_str(), lastName.c_str());

    //printf("profilestring: %s\n", statsString.c_str());

    query += (std::string)("insert into players (id, firstname, lastname, team_id, role, formationorder, age, base_stat, profile_xml, skincolor, hairstyle, haircolor, height, weight, nationalteam_id, nationalteamformationorder) values ") +
             (std::string)("(" + int_to_str(player.id) + ", \"" + firstName + "\", \"" + lastName + "\", " + int_to_str(player.clubID) + ", \"" + player.position + "\", " + int_to_str(formationOrder) + ", " + int_to_str(player.age) + ", " + real_to_str(player.averageStartStat) + ", \"" + statsString + "\", " + int_to_str(skinColor) + ", \"" + hairStyle + "\", \"" + hairColor + "\", " + real_to_str(height) + ", " + real_to_str(weight) + ", -1, -1);");
  }
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;

  printf("WORST YOUNGSTERS HALL OF SHAME\n");
  std::map<float, int>::iterator bestYoungPlayersIter = bestYoungPlayersMap.begin();
  for (int i = 0; i < 30; i++) {
    printf("%f (current: %f, age %i): %s\n", bestYoungPlayersIter->first, importedPlayers.at(bestYoungPlayersIter->second).averageStat, importedPlayers.at(bestYoungPlayersIter->second).age, importedPlayers.at(bestYoungPlayersIter->second).lastName.c_str());
    bestYoungPlayersIter++;
  }

  printf("BEST YOUNGSTERS HALL OF FAME\n");
  bestYoungPlayersIter = bestYoungPlayersMap.end();
  bestYoungPlayersIter--;
  for (int i = 0; i < 70; i++) {
    printf("%f (current: %f, age %i): %s\n", bestYoungPlayersIter->first, importedPlayers.at(bestYoungPlayersIter->second).averageStat, importedPlayers.at(bestYoungPlayersIter->second).age, importedPlayers.at(bestYoungPlayersIter->second).lastName.c_str());
    bestYoungPlayersIter--;
  }


  // calculate order of players per club - to get players on their right positions, and the best players in the base eleven

  query = "begin transaction;";
  for (unsigned int c = 0; c < clubs.size(); c++) {

    // first, put the best keeper on top, and other keepers away
    std::vector<PlayerImport> keepers;
    for (unsigned int p = 0; p < clubs.at(c).players.size(); p++) {
      if (clubs.at(c).players.at(p).position.compare("GK") == 0) keepers.push_back(clubs.at(c).players.at(p));
    }
    //assert(keepers.size() > 0);
    if (keepers.size() > 0) {
      //printf("Team %s DOES have keeper.\n", clubs.at(c).name.c_str());
      std::sort(keepers.rbegin(), keepers.rend(), SortClubPlayersByAverageStat);
      clubs.at(c).players.insert(clubs.at(c).players.begin(), keepers.at(0));

      // delete other keepers for now
      std::vector<PlayerImport>::iterator iter = clubs.at(c).players.begin();
      iter++; // skip keeper we just added
      while (iter != clubs.at(c).players.end()) {
        if ((*iter).position.compare("GK") == 0) {
          if ((*iter).id != keepers.at(0).id) query += (std::string)("delete from players where players.id = " + int_to_str((*iter).id) + ";");
          iter = clubs.at(c).players.erase(iter);
        } else {
          iter++;
        }
      }

    } else {

      //Log(e_Warning, "MainMenuPage", "GoImportDB", "Team " + clubs.at(c).name + " has no keeper!");
    }

    // bestestest players on top
    std::vector<PlayerImport>::reverse_iterator rend = clubs.at(c).players.rend();
    std::vector<PlayerImport>::reverse_iterator rbegin = clubs.at(c).players.rbegin();
    if (keepers.size() > 0) rend--; // keep keeper intact
    //rbegin++;
    std::sort(rbegin, rend, SortClubPlayersByAverageStat);

    // now sort first 11 by role (defenders in da back)
    std::vector<PlayerImport>::iterator iter = clubs.at(c).players.begin();
    std::vector<PlayerImport>::iterator iterEnd = clubs.at(c).players.begin();
    if (keepers.size() > 0) std::advance(iter, 1); // keep keeper intact
    std::advance(iterEnd, std::min(11, int(clubs.at(c).players.size())));
    std::sort(iter, iterEnd, SortClubPlayersByRole);

    //printf("KEEPER?: %s\n", clubs.at(c).players.begin()->lastName.c_str());

/*
    // add second keeper
    if (keepers.size() > 1) {
      iter = clubs.at(c).players.begin();
      advance(iter, 11);
      clubs.at(c).players.insert(iter, keepers.at(1));
    }
*/

    // only keep the best
    unsigned int keepPlayers = 18;
    if (clubs.at(c).players.size() < keepPlayers) keepPlayers = clubs.at(c).players.size();

    // sort the remaining
    if (clubs.at(c).players.size() > 11) {
      iter = clubs.at(c).players.begin();
      iterEnd = clubs.at(c).players.begin();
      std::advance(iter, 11);
      std::advance(iterEnd, keepPlayers);
      std::sort(iter, iterEnd, SortClubPlayersByRole);
    }

    for (unsigned int p = 0; p < keepPlayers; p++) {
      //printf("%i: %s\n", p, clubs.at(c).players.at(p).lastName.c_str());
      query += (std::string)("update players set formationorder = " + int_to_str(p) + " where players.id = " + int_to_str(clubs.at(c).players.at(p).id) + " and players.formationorder = -1;");

      // if not exist, add to namesdb code
      std::string firstname = clubs.at(c).players.at(p).firstName;
      std::string lastname = clubs.at(c).players.at(p).lastName;
      //std::string namequery = "insert into playernames (firstname, lastname, fakefirstname, fakelastname, formationorder) values (\"" + firstname + "\", \"" + lastname + "\", \"" + firstname + "\", \"" + lastname + "\", " + int_to_str(p) + ");";
      std::string namequery = "insert into playernames (firstname, lastname, fakefirstname, fakelastname, formationorder) select \"" + firstname + "\", \"" + lastname + "\", \"" + firstname + "\", \"" + lastname + "\", " + int_to_str(p) + " WHERE NOT EXISTS(SELECT 1 FROM playernames WHERE (firstname = \"" + firstname + "\" AND lastname = \"" + lastname + "\") OR (fakefirstname = \"" + firstname + "\" AND fakelastname = \"" + lastname + "\"));";
      printf("%s\n", namequery.c_str());
      DatabaseResult *nameresult = namedb->Query(namequery);
      delete nameresult;
    }

    // delete the rest
    for (unsigned int p = keepPlayers; p < clubs.at(c).players.size(); p++) {
      query += (std::string)("delete from players where players.id = " + int_to_str(clubs.at(c).players.at(p).id) + ";");
    }

  }
  query += "commit;";
  result = GetDB()->Query(query);
  delete result;

  delete namedb;

  return true;
}

/*XX deprecated
void MainMenuPage::Import_AgeValueStatsAdd(int age, int value) {
  std::map < int, std::vector<int> >::iterator iter = ageValues.find(age);
  if (iter == ageValues.end()) {
    std::vector<int> valueVec;
    valueVec.push_back(value);
    ageValues.insert(std::pair<int, std::vector<int> >(age, valueVec));
  } else {
    iter->second.push_back(value);
  }
}

void MainMenuPage::Import_ProcessAgeValueStats() {

  bool cachedVersion = false;

  if (cachedVersion) {

    averageStatPerAge.insert(std::pair<int, float>(15, 0.452861));
    averageStatPerAge.insert(std::pair<int, float>(16, 0.456137));
    averageStatPerAge.insert(std::pair<int, float>(17, 0.463531));
    averageStatPerAge.insert(std::pair<int, float>(18, 0.476644));
    averageStatPerAge.insert(std::pair<int, float>(19, 0.495670));
    averageStatPerAge.insert(std::pair<int, float>(20, 0.521165));
    averageStatPerAge.insert(std::pair<int, float>(21, 0.549251));
    averageStatPerAge.insert(std::pair<int, float>(22, 0.581290));
    averageStatPerAge.insert(std::pair<int, float>(23, 0.609627));
    averageStatPerAge.insert(std::pair<int, float>(24, 0.640026));
    averageStatPerAge.insert(std::pair<int, float>(25, 0.661062));
    averageStatPerAge.insert(std::pair<int, float>(26, 0.681876));
    averageStatPerAge.insert(std::pair<int, float>(27, 0.695848));
    averageStatPerAge.insert(std::pair<int, float>(28, 0.707468));
    averageStatPerAge.insert(std::pair<int, float>(29, 0.716388));
    averageStatPerAge.insert(std::pair<int, float>(30, 0.722636));
    averageStatPerAge.insert(std::pair<int, float>(31, 0.725265));
    averageStatPerAge.insert(std::pair<int, float>(32, 0.718620));
    averageStatPerAge.insert(std::pair<int, float>(33, 0.706918));
    averageStatPerAge.insert(std::pair<int, float>(34, 0.701204));
    averageStatPerAge.insert(std::pair<int, float>(35, 0.698097));
    averageStatPerAge.insert(std::pair<int, float>(36, 0.695981));
    averageStatPerAge.insert(std::pair<int, float>(37, 0.694231));
    averageStatPerAge.insert(std::pair<int, float>(38, 0.693000));
    averageStatPerAge.insert(std::pair<int, float>(39, 0.690599));
    averageStatPerAge.insert(std::pair<int, float>(40, 0.683223));

  } else {

    std::map<int, float> tempAverageStatPerAge; // needs to be smoothed later on into the non-temp version
    int lowestOverallValue = 10000000;
    int highestOverallValue = 0;
    for (int age = 15; age <= 40; age++) {
      std::map < int, std::vector<int> >::iterator iter = ageValues.find(age);
      if (iter != ageValues.end()) {
        int totalValues = 0;
        int cumulativeValue = 0;
        int lowestValue = 10000000;
        int highestValue = 0;
        std::vector<int> &values = iter->second;
        for (unsigned int i = 0; i < values.size(); i++) {
          cumulativeValue += values.at(i);
          totalValues++;
          if (values.at(i) < lowestValue) lowestValue = values.at(i);
          if (values.at(i) > highestValue) highestValue = values.at(i);
          if (values.at(i) < lowestOverallValue) lowestOverallValue = values.at(i);
          if (values.at(i) > highestOverallValue) highestOverallValue = values.at(i);
        }
        int average = int(round((float)cumulativeValue / (float)totalValues));
        std::sort(values.begin(), values.end());
        int median = values.at(clamp(values.size() / 2, 0, values.size() - 1));
        printf("age: %i, players: %i, average value: %i, median value: %i, lowest/highest: %i/%i\n", age, totalValues, average, median, lowestValue, highestValue);

        // save
        //float averageStat = GetAverageStatFromValue(age, average);
        float averageStat = GetAverageStatFromValue(age, median * 0.9 + average * 0.1);
        tempAverageStatPerAge.insert(std::pair<int, float>(age, averageStat));
      }
    }

    // average stats per age over multiple ages to smooth out the graph and to fix ages where few players are available (>~40 year old)
    averageStatPerAge = tempAverageStatPerAge;
    std::map<int, float>::iterator tmpIter = tempAverageStatPerAge.begin();
    std::map<int, float>::iterator iter = averageStatPerAge.begin();
    while (tmpIter != tempAverageStatPerAge.end()) {
      int amount = 1; // divide by how many? (for average)
      float average = tmpIter->second;
      if (tmpIter != tempAverageStatPerAge.begin()) { // add previous
        tmpIter--;
        average += tmpIter->second;
        amount++;
        tmpIter++;
      }
      if (boost::next(tmpIter) != tempAverageStatPerAge.end()) { // add next
        tmpIter++;
        average += tmpIter->second;
        amount++;
        tmpIter--;
      }

      iter->second = average / (float)amount;
      //printf("new: %f\n", iter->second);

      tmpIter++;
      iter++;
    }

    printf("overall lowest/highest value: %i/%i\n", lowestOverallValue, highestOverallValue);
    // right now: overall lowest/highest value: 10/45643480


    iter = averageStatPerAge.begin();
    while (iter != averageStatPerAge.end()) {
      //printf("average stat at age %i: %f\n", iter->first, iter->second);
      printf("averageStatPerAge.insert(std::pair<int, float>(%i, %f));\n", iter->first, iter->second);
      iter++;
    }

  } // uncached version

}
*/

// inhibit 'back' button from closing the game
void MainMenuPage::ProcessWindowingEvent(WindowingEvent *event) {
  event->Ignore();
}
