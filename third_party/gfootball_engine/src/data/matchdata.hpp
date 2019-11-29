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

#ifndef _HPP_MATCHDATA
#define _HPP_MATCHDATA

#include "../defines.hpp"

#include "../gamedefines.hpp"

#include "teamdata.hpp"

class MatchData {

  public:
    MatchData();
    TeamData& GetTeamData(int id) { DO_VALIDATION; return teamData[id]; }
    int GetGoalCount(int id) { DO_VALIDATION; return goalCount[id]; }
    void SetGoalCount(int id, int amount) { DO_VALIDATION; goalCount[id] = amount; }
    void AddPossessionTime(int teamID, unsigned long time);
    float GetPossessionFactor_60seconds() { DO_VALIDATION;
      return possession60seconds / 60.0f;
    }  // REMEMBER THESE ARE IRL INGAME SECONDS (because, I guess the tactics
       // should be based on irl possession time instead of gametime? not sure
       // yet, think about this)
    void ProcessState(EnvState* state, int first_team);
   protected:
    TeamData teamData[2];

    int goalCount[2];

    float possession60seconds; // -60 to 60 for possession of team 1 / 2 respectively

};

#endif
