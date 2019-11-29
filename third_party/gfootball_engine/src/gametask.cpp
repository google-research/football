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

#include "gametask.hpp"

#include "main.hpp"

#include "blunted.hpp"

GameTask::GameTask() {
  DO_VALIDATION;
  // prohibits deletion of the scene before this object is dead
  scene3D = GetScene3D();
}

GameTask::~GameTask() {
  DO_VALIDATION;
  StopMatch();
}

void GameTask::StartMatch(bool animations) {
  DO_VALIDATION;
  randomize(GetScenarioConfig().game_engine_random_seed);
  MatchData *matchData = GetMenuTask()->GetMatchData();
  assert(matchData);
  assert(!match);
  match.reset(new Match(matchData, GetControllers(), animations));
}

bool GameTask::StopMatch() {
  DO_VALIDATION;
  if (match) {
    DO_VALIDATION;
    match->Exit();
    match.reset();
    return true;
  }
  return false;
}

void GameTask::ProcessPhase() {
  DO_VALIDATION;
  bool process = match->Process();
  match->UpdateCamera();
  if (process) {
    match->PreparePutBuffers();
    match->FetchPutBuffers();
  }
}

void GameTask::PrepareRender() {
  match->Put();
  std::vector<Player*> players;
  match->GetActiveTeamPlayers(match->FirstTeam(), players);
  match->GetActiveTeamPlayers(match->SecondTeam(), players);
  std::vector<PlayerBase*> officials;
  match->GetOfficialPlayers(officials);

  for (auto player : players) {
    DO_VALIDATION;
    player->UpdateFullbodyModel();
    boost::static_pointer_cast<Geometry>(player->GetFullbodyNode()->GetObject("fullbody"))->OnUpdateGeometryData();
  }
  for (auto official : officials) {
    DO_VALIDATION;
    official->UpdateFullbodyModel();
    boost::static_pointer_cast<Geometry>(official->GetFullbodyNode()->GetObject("fullbody"))->OnUpdateGeometryData();
  }
  match->UploadGoalNetting(); // won't this block the whole process thing too? (opengl busy == wait, while mutex locked == no process)
  DO_VALIDATION;
}
