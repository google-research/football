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

#include "officials.hpp"

#include "../scene/scene3d/scene3d.hpp"

#include "../utils/objectloader.hpp"
#include "../scene/objectfactory.hpp"

#include "player/playerofficial.hpp"
#include "player/humanoid/humanoidbase.hpp"

#include "../data/playerdata.hpp"

#include "../main.hpp"

Officials::Officials(Match *match,
                     boost::intrusive_ptr<Node> fullbodySourceNode,
                     std::map<Vector3, Vector3> &colorCoords,
                     boost::intrusive_ptr<Resource<Surface> > kit,
                     boost::shared_ptr<AnimCollection> animCollection)
    : match(match) {
  DO_VALIDATION;
  ObjectLoader loader;
  boost::intrusive_ptr<Node> playerNode = loader.LoadObject("media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetLocalMode(e_LocalMode_Absolute);

  playerData = new PlayerData();
  referee = new PlayerOfficial(e_OfficialType_Referee, match, playerData);
  linesmen[0] = new PlayerOfficial(e_OfficialType_Linesman, match, playerData);
  linesmen[1] = new PlayerOfficial(e_OfficialType_Linesman, match, playerData);

  referee->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection(), false);
  linesmen[0]->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection(), false);
  linesmen[1]->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection(), false);
  playerNode->Exit();
  playerNode.reset();

  referee->CastHumanoid()->ResetPosition(Vector3(10, -10, 0), Vector3(0));
  linesmen[0]->CastHumanoid()->ResetPosition(Vector3(25, -36.5, 0), Vector3(0));
  linesmen[1]->CastHumanoid()->ResetPosition(Vector3(-25, 36.5, 0), Vector3(0));

  boost::intrusive_ptr<Resource<GeometryData> > geometry =
      GetContext().geometry_manager.Fetch(
          "media/objects/officials/yellowcard.ase", true);
  yellowCard = new Geometry("yellowcard");
  GetScene3D()->CreateSystemObjects(yellowCard);
  yellowCard->SetGeometryData(geometry);
  yellowCard->SetLocalMode(e_LocalMode_Absolute);
  yellowCard->SetPosition(Vector3(0, 0, -10));

  geometry = GetContext().geometry_manager.Fetch(
      "media/objects/officials/redcard.ase", true);
  redCard = new Geometry("redCard");
  GetScene3D()->CreateSystemObjects(redCard);
  redCard->SetGeometryData(geometry);
  redCard->SetLocalMode(e_LocalMode_Absolute);
  redCard->SetPosition(Vector3(0, 0, -10));
}

Officials::~Officials() {
  DO_VALIDATION;
  delete referee;
  delete linesmen[0];
  delete linesmen[1];
  delete playerData;

  redCard.reset();
  yellowCard.reset();
}

void Officials::Mirror() {
  DO_VALIDATION;
  referee->Mirror();
  linesmen[0]->Mirror();
  linesmen[1]->Mirror();
}

void Officials::GetPlayers(std::vector<PlayerBase *> &players) {
  DO_VALIDATION;
  players.push_back(referee);
  players.push_back(linesmen[0]);
  players.push_back(linesmen[1]);
}

void Officials::Process() {
  DO_VALIDATION;
  referee->Process();
  GetContext().tracker_disabled++;
  linesmen[0]->Process();
  linesmen[1]->Process();
  GetContext().tracker_disabled--;
}

void Officials::FetchPutBuffers() {
  DO_VALIDATION;
  referee->FetchPutBuffers();
  linesmen[0]->FetchPutBuffers();
  linesmen[1]->FetchPutBuffers();
}

void Officials::Put(bool mirror) {
  DO_VALIDATION;
  referee->Put(mirror);
  linesmen[0]->Put(mirror);
  linesmen[1]->Put(mirror);

  if (referee->GetCurrentFunctionType() == e_FunctionType_Special &&
      (match->GetReferee()->GetCurrentFoulType() == 2 ||
       match->GetReferee()->GetCurrentFoulType() == 3)) {
    DO_VALIDATION;
    if (mirror) {
      referee->Mirror();
    }
    BodyPart bodyPartName = right_elbow;
    if (referee->GetCurrentAnim()->anim->GetName().find("mirror") != std::string::npos) bodyPartName = left_elbow;

    const NodeMap &nodeMap = referee->GetNodeMap();
    auto bodyPart = nodeMap[bodyPartName];
    if (bodyPart) {
      DO_VALIDATION;
      Vector3 position = bodyPart->GetDerivedPosition() + bodyPart->GetDerivedRotation() * Vector3(0.04, 0, -0.25); // -0.4
      if (match->GetReferee()->GetCurrentFoulType() == 2) {
        DO_VALIDATION;
        yellowCard->SetPosition(position);
        yellowCard->SetRotation(bodyPart->GetDerivedRotation());
      } else {
        redCard->SetPosition(position);
        redCard->SetRotation(bodyPart->GetDerivedRotation());
      }
    }
    if (mirror) {
      referee->Mirror();
    }
  } else if (referee->GetPreviousFunctionType() == e_FunctionType_Special) {
    DO_VALIDATION;
    yellowCard->SetPosition(Vector3(0, 0, -10));
    redCard->SetPosition(Vector3(0, 0, -10));
  }
}

void Officials::ProcessState(EnvState *state) {
  DO_VALIDATION;
  referee->ProcessStateBase(state);
  state->setValidate(false);
  linesmen[0]->ProcessStateBase(state);
  linesmen[1]->ProcessStateBase(state);
  state->setValidate(true);
}
