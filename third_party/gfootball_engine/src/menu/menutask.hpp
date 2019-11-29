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

#ifndef _HPP_GUI2_MENUTASK
#define _HPP_GUI2_MENUTASK

#include "ingame/gamepage.hpp"

#include "../utils/gui2/guitask.hpp"

#include "../utils/gui2/widgets/image.hpp"

#include "../scene/scene3d/scene3d.hpp"

#include "../gamedefines.hpp"

class Match;
class MatchData;

using namespace blunted;

enum e_MenuAction {
  e_MenuAction_Menu, // start main menu
  e_MenuAction_Game, // start game
  e_MenuAction_None
};

struct SideSelection {
  int controllerID = 0;
  Gui2Image *controllerImage;
  int side = 0; // -1, 0, 1
};


struct QueuedFixture {
  QueuedFixture() { DO_VALIDATION;
    team1KitNum = 1;
    team2KitNum = 2;
    matchData = 0;
  }
  std::vector<SideSelection> sides; // queued match fixture
  int team1KitNum, team2KitNum;
  MatchData *matchData;
};

class MenuTask : public Gui2Task {

  public:
    MenuTask(float aspectRatio, float margin, TTF_Font *defaultFont, TTF_Font *defaultOutlineFont, const Properties* config);
    virtual ~MenuTask();

    void SetControllerSetup(const std::vector<SideSelection> &sides) { DO_VALIDATION; queuedFixture.sides = sides;  }
    const std::vector<SideSelection> GetControllerSetup() { DO_VALIDATION;
      return queuedFixture.sides;
    }
    int GetTeamKitNum(int teamID) { DO_VALIDATION; if (teamID == 0) return queuedFixture.team1KitNum; else return queuedFixture.team2KitNum; }
    void SetMatchData(MatchData *matchData) { DO_VALIDATION;  queuedFixture.matchData = matchData;  }
    MatchData *GetMatchData() { DO_VALIDATION; return queuedFixture.matchData; } // hint: this lock is useless, since we are returning the pointer and not a copy

  protected:
   QueuedFixture queuedFixture;

};

#endif
