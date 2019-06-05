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

#ifndef _HPP_MENU_GAMEPLAN
#define _HPP_MENU_GAMEPLAN

#include "../utils/gui2/windowmanager.hpp"

#include "../utils/gui2/page.hpp"
#include "../utils/gui2/widgets/root.hpp"
#include "../utils/gui2/widgets/grid.hpp"
#include "../utils/gui2/widgets/button.hpp"
#include "../utils/gui2/widgets/slider.hpp"
#include "../utils/gui2/widgets/image.hpp"

#include "widgets/gameplansubmenu.hpp"

#include "widgets/planmap.hpp"

#include "../onthepitch/match.hpp"

#include "../data/teamdata.hpp"

#include "../utils/database.hpp"

using namespace blunted;

struct TacticsSlider {
  int id = 0;
  Gui2Slider* widget;
  std::string tacticName;
};

class GamePlanPage : public Gui2Page {

  public:
    GamePlanPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamePlanPage();

    void OnClose();

    virtual void Deactivate();
    virtual void Reactivate();

    Vector3 GetButtonColor(int id);

    void GoLineupMenu();
    void LineupMenuOnClick(Gui2Button *button);
    void SaveLineup();

    void GoTacticsMenu();
    void TacticsMenuOnChange(Gui2Slider *slider, int id);
    void SaveTactics();

  protected:
    int teamID = 0;

    Gui2PlanMap *map;
    Gui2Grid *grid;
    Gui2Grid *gridNav;
    Gui2Button *buttonLineup;
    Gui2Button *buttonTactics;

    TeamData *teamData;

    GamePlanSubMenu *lineupMenu;
    GamePlanSubMenu *tacticsMenu;

    std::vector<TacticsSlider> tacticsSliders;

    Database *namedb;

};

#endif
