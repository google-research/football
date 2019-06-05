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

#ifndef _HPP_PAGEFACTORY
#define _HPP_PAGEFACTORY

#include "../utils/gui2/page.hpp"

using namespace blunted;

enum e_PageID {
  e_PageID_MainMenu,
  e_PageID_Game,
  e_PageID_Intro,
  e_PageID_Outro,
  e_PageID_Credits,
  e_PageID_Settings,
  e_PageID_ControllerSelect,
  e_PageID_TeamSelect,
  e_PageID_MatchOptions,
  e_PageID_LoadingMatch,
  e_PageID_MatchPhase,
  e_PageID_GameOver,
  e_PageID_Ingame,
  e_PageID_PreQuit,
  e_PageID_GamePlan,
  e_PageID_VisualOptions,
  e_PageID_Camera,
  e_PageID_Gameplay,
  e_PageID_Controller,
  e_PageID_Keyboard,
  e_PageID_Gamepads,
  e_PageID_GamepadSetup,
  e_PageID_GamepadCalibration,
  e_PageID_GamepadMapping,
  e_PageID_GamepadFunction,
  e_PageID_Graphics,
  e_PageID_Audio,

  e_PageID_League_Start,
  e_PageID_League_Start_Load,
  e_PageID_League_Start_New,
  e_PageID_League,
  e_PageID_League_Forward,
  e_PageID_League_Inbox,
  e_PageID_League_Team,
  e_PageID_League_Team_Formation,
  e_PageID_League_Team_PlayerSelection,
  e_PageID_League_Team_Tactics,
  e_PageID_League_Team_PlayerOverview,
  e_PageID_League_Team_PlayerDevelopment,
  e_PageID_League_Team_Setup,
  e_PageID_League_Calendar,
  e_PageID_League_Standings,
  e_PageID_League_Standings_League,
  e_PageID_League_Standings_League_Table,
  e_PageID_League_Standings_League_Stats,
  e_PageID_League_Standings_NCup,
  e_PageID_League_Standings_NCup_Tree,
  e_PageID_League_Standings_NCup_Stats,
  e_PageID_League_Standings_ICup1,
  e_PageID_League_Standings_ICup1_GroupTable,
  e_PageID_League_Standings_ICup1_Tree,
  e_PageID_League_Standings_ICup1_Stats,
  e_PageID_League_Standings_ICup2,
  e_PageID_League_Standings_ICup2_GroupTable,
  e_PageID_League_Standings_ICup2_Tree,
  e_PageID_League_Standings_ICup2_Stats,
  e_PageID_League_Management,
  e_PageID_League_Management_Contracts,
  e_PageID_League_Management_Transfers,
  e_PageID_League_System,
  e_PageID_League_System_Save,
  e_PageID_League_System_Settings
};

class PageFactory : public Gui2PageFactory {

  public:
    using Gui2PageFactory::CreatePage;
    virtual Gui2Page *CreatePage(const Gui2PageData &pageData);
    virtual Gui2Page *GetMostRecentlyCreatedPage();
};

#endif
