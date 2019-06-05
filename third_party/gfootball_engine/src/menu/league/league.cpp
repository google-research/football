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

#include "league.hpp"

#include "../pagefactory.hpp"

#include "../../league/leaguecode.hpp"

#include "../../utils/gui2/widgets/root.hpp"
#include "../../utils/gui2/widgets/frame.hpp"
#include "../../utils/gui2/widgets/caption.hpp"
#include "../../utils/gui2/widgets/text.hpp"

#include "../../base/utils.hpp"

LeaguePage::LeaguePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league", 20, 20, 60, 3, "League");
  this->AddView(title);
  title->Show();

  captionTime = new Gui2Caption(windowManager, "caption_league_time", 60, 20, 20, 3, "time");
  this->AddView(captionTime);
  captionTime->Show();

  SetTimeCaption();

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_league_main", 20, 30, 60, 50);

  Gui2Button *buttonStepTime = new Gui2Button(windowManager, "button_league_steptime", 20, 30, 30, 3, "Step time");
  buttonStepTime->sig_OnClick.connect(boost::bind(&LeaguePage::StepTime, this));
  buttonStepTime->SetFocus();

  grid->AddView(buttonStepTime, 0, 0);

  this->AddView(grid);
  grid->UpdateLayout();
  grid->Show();

  this->Show();
}

LeaguePage::~LeaguePage() {
}

void LeaguePage::StepTime() {
  DatabaseResult *result = GetDB()->Query("SELECT strftime(\"%w\", timestamp), strftime(\"%Y\", timestamp), seasonyear FROM settings LIMIT 1");
  int dayOfWeek = atoi(result->data.at(0).at(0).c_str());
  int actualyear = atoi(result->data.at(0).at(1).c_str());
  int seasonyear = atoi(result->data.at(0).at(2).c_str());
  delete result;

  int offset = 0;
  if (dayOfWeek < 3) offset = 3 - dayOfWeek;
  else if (dayOfWeek < 6) offset = 6 - dayOfWeek;
  else offset = 4;

  result = GetDB()->Query("UPDATE settings SET timestamp = date(timestamp, '+" + int_to_str(offset) + " day')");
  delete result;

  // check if season complete
  if (actualyear > seasonyear) {
    result = GetDB()->Query("UPDATE settings SET seasonyear = " + int_to_str(seasonyear + 1));
    delete result;
    GenerateSeasonCalendars();
  }

  SetTimeCaption();
}

void LeaguePage::SetTimeCaption() {
  DatabaseResult *result = GetDB()->Query("SELECT timestamp, strftime('%w', timestamp) FROM settings LIMIT 1");
  std::string dayName;
  switch (atoi(result->data.at(0).at(1).c_str())) {
    case 0: dayName = "sunday"; break;
    case 1: dayName = "monday"; break;
    case 2: dayName = "tuesday"; break;
    case 3: dayName = "wednesday"; break;
    case 4: dayName = "thursday"; break;
    case 5: dayName = "friday"; break;
    case 6: dayName = "saturday"; break;
    default: dayName = "bug"; break;
  }
  captionTime->SetCaption(result->data.at(0).at(0) + " (" + dayName + ")");
  delete result;
}



LeagueStartPage::LeagueStartPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Frame *frame = new Gui2Frame(windowManager, "bg_league_start", 30, 35, 40, 30, true);
  this->AddView(frame);
  frame->Show();
  //bg->Redraw();

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_start", 5, 5, 20, 3, "Start/load league");
  frame->AddView(title);
  title->Show();

  Gui2Button *buttonLoad = new Gui2Button(windowManager, "button_league_start_load", 0, 0, 30, 3, "Continue saved league");
  buttonLoad->sig_OnClick.connect(boost::bind(&LeagueStartPage::GoLoad, this));
  Gui2Button *buttonNew = new Gui2Button(windowManager, "button_league_start_new", 0, 0, 30, 3, "Start new league");
  buttonNew->sig_OnClick.connect(boost::bind(&LeagueStartPage::GoNew, this));

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_league_start_choices", 5, 10, 90, 80);
  grid->AddView(buttonLoad, 0, 0);
  grid->AddView(buttonNew, 1, 0);
  grid->UpdateLayout(0.5);
  frame->AddView(grid);
  grid->Show();

  buttonLoad->SetFocus();

  this->Show();
}

LeagueStartPage::~LeagueStartPage() {
}

void LeagueStartPage::GoLoad() {
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_League_Start_Load, properties, 0);
}

void LeagueStartPage::GoNew() {
  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_League_Start_New, properties, 0);
}



LeagueStartLoadPage::LeagueStartLoadPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Frame *frame = new Gui2Frame(windowManager, "bg_league_start_load", 20, 5, 60, 90, true);
  this->AddView(frame);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_start_load", 5, 5, 20, 3, "Load saved league");
  frame->AddView(title);

  browser = new Gui2FileBrowser(windowManager, "filebrowser_league_start_load", 5, 10, 40, 50, "./saves", e_DirEntryType_Directory);
  frame->AddView(browser);

  browser->sig_OnClick.connect(boost::bind(&LeagueStartLoadPage::GoLoadSave, this));

  browser->SetFocus();

  this->Show();
}

LeagueStartLoadPage::~LeagueStartLoadPage() {
}

void LeagueStartLoadPage::GoLoadSave() {
  std::string saveName = browser->GetClickedEntry().name;
  SetActiveSaveDirectory(saveName);

  boost::filesystem::path saveLoc("saves");
  saveLoc /= saveName;

  SaveDatabaseToAutosave();

  GetDB()->Load(saveLoc.string() + "/autosave.sqlite");

  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_League, properties, 0);
}



LeagueStartNewPage::LeagueStartNewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  data_SelectedDatabase = "default";

  Gui2Frame *frame = new Gui2Frame(windowManager, "bg_league_start_new", 5, 5, 90, 90, true);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_start_new", 5, 5, 20, 3, "Start new league");
  frame->AddView(title);
  title->Show();

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_league_start_new_choices", 5, 15, 90, 80);

  Gui2Caption *databaseSelectCaption = new Gui2Caption(windowManager, "caption_league_start_new_dbselect", 0, 0, 30, 2.5, "Select foundation database");
  Gui2Caption *currencySelectCaption = new Gui2Caption(windowManager, "caption_league_start_new_currency", 0, 0, 30, 2.5, "Select currency");
  Gui2Caption *saveNameCaption = new Gui2Caption(windowManager, "caption_league_start_new_savegamename", 0, 0, 30, 2.5, "Savegame name");
  Gui2Caption *managerNameCaption = new Gui2Caption(windowManager, "caption_league_start_new_managername", 0, 0, 30, 2.5, "Manager name");

  databaseSelectButton = new Gui2Button(windowManager, "button_league_start_new_dbselect", 0, 0, 30, 3, data_SelectedDatabase);
  databaseSelectButton->sig_OnClick.connect(boost::bind(&LeagueStartNewPage::GoDatabaseSelectDialog, this));

  currencySelectPulldown = new Gui2Pulldown(windowManager, "pulldown_league_start_new_currencyselect", 0, 0, 30, 3);
  currencySelectPulldown->AddEntry("Euro", "euro");
  currencySelectPulldown->AddEntry("Dollar", "dollar");
  currencySelectPulldown->AddEntry("Yen", "yen");
  currencySelectPulldown->AddEntry("Pound", "pound");
  currencySelectPulldown->AddEntry("Swiss franc", "swissfranc");
  currencySelectPulldown->AddEntry("Australian dollar", "ausdollar");
  currencySelectPulldown->AddEntry("Canadian dollar", "candollar");
  currencySelectPulldown->AddEntry("Swedish krone", "swekrone");
  currencySelectPulldown->AddEntry("Hong Kong dollar", "hongkongdollar");
  currencySelectPulldown->AddEntry("Norwegian krone", "norkrone");

  difficultySlider = new Gui2Slider(windowManager, "slider_league_start_new_difficulty", 0, 0, 30, 6, "Initial difficulty");
  saveNameInput = new Gui2EditLine(windowManager, "editline_league_start_new_savegamename", 0, 0, 30, 3, "NewLeague");
  saveNameInput->SetAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ ");
  saveNameInput->SetMaxLength(24);
  managerNameInput = new Gui2EditLine(windowManager, "editline_league_start_new_managername", 0, 0, 30, 3, "Titi Fillanovi");
  managerNameInput->SetAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=`~@#$%^&*()_+[]{}\\,./<>?;':\" ");
  managerNameInput->SetMaxLength(32);
  Gui2Button *proceedButton = new Gui2Button(windowManager, "button_league_start_new_proceed", 0, 0, 30, 3, "Proceed");
  proceedButton->sig_OnClick.connect(boost::bind(&LeagueStartNewPage::GoProceed, this));

  Gui2Grid *gridDBSelect = new Gui2Grid(windowManager, "grid_league_start_new_choices_dbselect", 0, 0, 1, 1);
  gridDBSelect->AddView(databaseSelectCaption, 0, 0);
  gridDBSelect->AddView(databaseSelectButton, 1, 0);
  gridDBSelect->UpdateLayout(0.0, 0.0, 0.5, 0.5);
  gridDBSelect->SetWrapping(false);
  grid->AddView(gridDBSelect, 0, 0);

  Gui2Grid *gridCurrencySelect = new Gui2Grid(windowManager, "grid_league_start_new_choices_currencyselect", 0, 0, 1, 1);
  gridCurrencySelect->AddView(currencySelectCaption, 0, 0);
  gridCurrencySelect->AddView(currencySelectPulldown, 1, 0);
  gridCurrencySelect->UpdateLayout(0.0, 0.0, 0.5, 0.5);
  gridCurrencySelect->SetWrapping(false);
  grid->AddView(gridCurrencySelect, 1, 0);

  grid->AddView(difficultySlider, 2, 0);

  Gui2Grid *gridSaveName = new Gui2Grid(windowManager, "grid_league_start_new_choices_savegamename", 0, 0, 1, 1);
  gridSaveName->AddView(saveNameCaption, 0, 0);
  gridSaveName->AddView(saveNameInput, 1, 0);
  gridSaveName->UpdateLayout(0.0, 0.0, 0.5, 0.5);
  gridSaveName->SetWrapping(false);
  grid->AddView(gridSaveName, 3, 0);

  Gui2Grid *gridManagerName = new Gui2Grid(windowManager, "grid_league_start_new_choices_managername", 0, 0, 1, 1);
  gridManagerName->AddView(managerNameCaption, 0, 0);
  gridManagerName->AddView(managerNameInput, 1, 0);
  gridManagerName->UpdateLayout(0.0, 0.0, 0.5, 0.5);
  gridManagerName->SetWrapping(false);
  grid->AddView(gridManagerName, 4, 0);

  grid->AddView(proceedButton, 5, 0);

  grid->UpdateLayout(0.0, 0.0, 0.0, 3.0);
  frame->AddView(grid);
  grid->Show();

  Gui2Text *explanationText = new Gui2Text(windowManager, "grid_league_start_new_choices_explanation", 40, 15, 40, 75, 2.5, 40, "");

  explanationText->AddText((std::string)
                           "The foundation database will be copied to a new directory that will serve as a 'save file' for your league. So, this database is what " +
                           "your league will be based on; any changes to the foundation database later on won't affect your league save (or the other way round).");
  explanationText->AddEmptyLine();
  explanationText->AddText((std::string)
                           "You can find the foundation database(s) in the 'databases' subdirectory of your Gameplay Football installation, and the " +
                           "saved leagues and cups in the 'save' directory.");

  frame->AddView(explanationText);
  explanationText->Show();

  databaseSelectButton->SetFocus();

  this->AddView(frame);
  frame->Show();

  this->Show();
}

LeagueStartNewPage::~LeagueStartNewPage() {
}

void LeagueStartNewPage::GoDatabaseSelectDialog() {
  databaseSelectDialog = new Gui2Dialog(windowManager, "dialog_league_start_new_dbselect", 30, 25, 40, 50, "Select source database");
  previousFocus = windowManager->GetFocus();
  databaseSelectDialog->sig_OnClose.connect(boost::bind(&LeagueStartNewPage::CloseDatabaseSelectDialog, this));

  databaseSelectBrowser = new Gui2FileBrowser(windowManager, "filebrowser_league_start_new_dbselect", 0, 0, 39, 40, "./databases", e_DirEntryType_Directory);
  databaseSelectBrowser->sig_OnClick.connect(boost::bind(&LeagueStartNewPage::CloseDatabaseSelectDialog, this));
  databaseSelectDialog->AddContent(databaseSelectBrowser);

  this->AddView(databaseSelectDialog);
  databaseSelectDialog->Show();

  databaseSelectBrowser->SetFocus();

  databaseSelectDialog->Show();
}

void LeagueStartNewPage::CloseDatabaseSelectDialog() {
  previousFocus->SetFocus();

  if (databaseSelectBrowser->GetClickedEntry().type == e_DirEntryType_Directory) {
    data_SelectedDatabase = databaseSelectBrowser->GetClickedEntry().name;
    databaseSelectButton->SetCaption(data_SelectedDatabase);
  }

  databaseSelectDialog->Exit();
  delete databaseSelectDialog;
}

void LeagueStartNewPage::GoProceed() {

  /* the values:
  printf("dbname: %s\n", data_SelectedDatabase.c_str());
  printf("currency: %s\n", currencySelectPulldown->GetSelected().c_str());
  printf("diff: %f\n", difficultySlider->GetValue());
  printf("savegame: %s\n", saveNameInput->GetText().c_str());
  printf("manager: %s\n", managerNameInput->GetText().c_str());
  */

  int errorCode = CreateNewLeagueSave(data_SelectedDatabase, saveNameInput->GetText());
  errorCode = 0;


  // result dialog

  previousFocus = windowManager->GetFocus();

  createSaveDialog = new Gui2Dialog(windowManager, "dialog_league_start_new_createsave", 25, 30, 50, 40, "New league creation");
  createSaveDialog->sig_OnClose.connect(boost::bind(&LeagueStartNewPage::CloseCreateSaveDialog, this));

  if (errorCode == 0) {

    Gui2Text *explanationText = new Gui2Text(windowManager, "text_league_start_new_createsave", 5, 5, 90, 75, 2.5, 60, "");
    explanationText->AddText((std::string)
                             "Successfully created new database directory. If you want to backup your save directory, you can find it here: '<game directory>/saves/" + saveNameInput->GetText() + "/'");
    createSaveDialog->AddContent(explanationText);

    (createSaveDialog->AddSingleButton("Yippee!"))->SetFocus();
    createSaveDialog->sig_OnPositive.connect(boost::bind(&LeagueStartNewPage::CloseCreateSaveDialog, this));

    success = true;

  } else {

    std::string errorString;
    switch (errorCode) {
      case 1: errorString = "Could not create save directory. Do you have write permissions? Or does it already exist?"; break;
      case 2: errorString = "Could not copy database file. Disk full?"; break;
      case 3: errorString = "Could not open copied database file. I have no idea why."; break;
      case 4: errorString = "Could not copy some image file. Disk full?"; break;
    }

    Gui2Text *explanationText = new Gui2Text(windowManager, "text_league_start_new_createsave", 5, 5, 90, 75, 2.5, 60, "");
    explanationText->AddText((std::string)
                             "Something went wrong! Error: " + errorString);
    explanationText->AddEmptyLine();
    explanationText->AddText((std::string)
                             "Please try to fix the problem and delete any possible remains of new save dir (<game directory>/saves/" + saveNameInput->GetText() + ")");
    createSaveDialog->AddContent(explanationText);

    (createSaveDialog->AddSingleButton("Oh crud!"))->SetFocus();
    createSaveDialog->sig_OnPositive.connect(boost::bind(&LeagueStartNewPage::CloseCreateSaveDialog, this));
    // lol forwarding signals overload: createSaveDialog->sig_OnPositive.connect(boost::bind(boost:ref(Gui2Dialog::sig_OnClose), createSaveDialog));

    success = false;

  }

  createSaveDialog->Show();



  /* test

  DatabaseResult *result = database->Query("select * from players");

  for (unsigned int h = 0; h < result->header.size(); h++) {
    printf("%s - ", result->header.at(h).c_str());
  }
  printf("\n");
  for (unsigned int r = 0; r < result->data.size(); r++) {
    for (unsigned int c = 0; c < result->data.at(r).size(); c++) {
      printf("%s - ", result->data.at(r).at(c).c_str());
    }
    printf("\n");
  }

  delete result;
  */

}

void LeagueStartNewPage::CloseCreateSaveDialog() {

  previousFocus->SetFocus();

  createSaveDialog->Exit();
  delete createSaveDialog;

  SetActiveSaveDirectory(saveNameInput->GetText());

  boost::filesystem::path saveLoc("saves");
  saveLoc /= GetActiveSaveDirectory();
  GetDB()->Load(saveLoc.string() + "/autosave.sqlite");

  if (success) {

    bool noError = PrepareDatabaseForLeague();
    if (!noError) Log(e_FatalError, "LeagueStartNewPage", "CloseCreateSaveDialog", "Could not prepare database for league");

    DatabaseResult *result = GetDB()->Query("INSERT INTO settings (managername, team_id, currency, difficulty, seasonyear, timestamp) VALUES ('" + managerNameInput->GetText() + "', 5, '" + currencySelectPulldown->GetSelected() + "', " + real_to_str(difficultySlider->GetValue()) + ", 2013, '2013-06-01')");
    delete result;

    GenerateSeasonCalendars();

    noError = SaveAutosaveToDatabase();
    if (!noError) Log(e_FatalError, "LeagueStartNewPage", "CloseCreateSaveDialog", "Could not save autosave file to persistent database");

    Properties properties;
    windowManager->GetPageFactory()->CreatePage((int)e_PageID_League, properties, 0);
  }
}
