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

#include "matchdata.hpp"
#include "../main.hpp"
#include <vector>
#include <algorithm>

MatchData::MatchData()
    : teamData{TeamData(3, GetScenarioConfig().left_team),
               TeamData(8, GetScenarioConfig().right_team)} {
  goalCount[0] = 0;
  goalCount[1] = 0;

  possession60seconds = 0.0f;
}

void MatchData::AddPossessionTime(int teamID, unsigned long time) {
  DO_VALIDATION;
  if (teamID == 0) possession60seconds = std::max(possession60seconds - (0.001f * time), -60.0f);
  else if (teamID == 1) possession60seconds = std::min(possession60seconds + (0.001f * time), 60.0f);
}

void MatchData::ProcessState(EnvState* state, int first_team) {
  DO_VALIDATION;
  state->process(goalCount[first_team]);
  state->process(goalCount[1 - first_team]);
  if (first_team == 1) {
    DO_VALIDATION;
    possession60seconds = -possession60seconds;
  }
  state->process(possession60seconds);
  if (first_team == 1) {
    DO_VALIDATION;
    possession60seconds = -possession60seconds;
  }
}
