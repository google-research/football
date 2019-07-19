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

#include "framework/scheduler.hpp"
#include "managers/resourcemanagerpool.hpp"

#include "blunted.hpp"

GameTask::GameTask() {

  match = 0;
  menuScene = 0;

  // prohibits deletion of the scene before this object is dead
  scene3D = GetScene3D();
}

GameTask::~GameTask() {
  Exit();
}

void GameTask::Exit() {

  Action(e_GameTaskMessage_StopMatch);
  Action(e_GameTaskMessage_StopMenuScene);

  ResourceManagerPool::CleanUp();

  scene3D.reset();
}

void GameTask::Action(e_GameTaskMessage message) {

  switch (message) {

    case e_GameTaskMessage_StartMatch:
      {

        randomize(GetScenarioConfig().game_engine_random_seed);

        MatchData *matchData = GetMenuTask()->GetMatchData();
        assert(matchData);
        Match *tmpMatch = new Match(matchData, GetControllers());
        assert(!match);
        match = tmpMatch;
        GetScheduler()->ResetTaskSequenceTime("game");
      }
      break;

    case e_GameTaskMessage_StopMatch:
      if (match) {
        match->Exit();
        delete match;
        match = 0;
      }
      break;

    case e_GameTaskMessage_StartMenuScene:
      assert(!menuScene);
      menuScene = new MenuScene();
      GetScheduler()->ResetTaskSequenceTime("game");
      break;

    case e_GameTaskMessage_StopMenuScene:
      if (menuScene) {
        delete menuScene;
        menuScene = 0;
      }
      break;

    default:
      break;

  }
}

void GameTask::GetPhase() {

  // process messageQueue
  if (match) match->Get();
  if (menuScene) menuScene->Get();
}

void GameTask::ProcessPhase() {

  for (unsigned int i = 0; i < GetControllers().size(); i++) {
    GetControllers()[i]->Process();
  }

  if (match) {
    match->Process();
    match->PreparePutBuffers();
  }

  if (menuScene) {
    menuScene->Process();
  }

}

void GameTask::PutPhase() {


  if (match) {
    match->FetchPutBuffers();
    match->Put();
    std::vector<Player*> players;
    match->GetActiveTeamPlayers(0, players);
    match->GetActiveTeamPlayers(1, players);
    std::vector<PlayerBase*> officials;
    match->GetOfficialPlayers(officials);

    for (auto player : players) {
      if (match->GetPause() || player->NeedsModelUpdate()) {
        player->UpdateFullbodyModel();
        boost::static_pointer_cast<Geometry>(player->GetFullbodyNode()->GetObject("fullbody"))->OnUpdateGeometryData();
      }
    }
    for (auto official : officials) {
      official->UpdateFullbodyModel();
      boost::static_pointer_cast<Geometry>(official->GetFullbodyNode()->GetObject("fullbody"))->OnUpdateGeometryData();
    }
    match->UploadGoalNetting(); // won't this block the whole process thing too? (opengl busy == wait, while mutex locked == no process)
  } // !match
  if (menuScene) menuScene->Put();
}
