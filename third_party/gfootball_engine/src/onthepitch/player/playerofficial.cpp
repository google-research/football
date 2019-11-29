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

#include "playerofficial.hpp"

#include "../match.hpp"

#include "controller/refereecontroller.hpp"

#include "../../main.hpp"
#include "../../utils.hpp"

PlayerOfficial::PlayerOfficial(e_OfficialType officialType, Match *match,
                               PlayerData *playerData)
    : PlayerBase(match, playerData), officialType(officialType) {
  DO_VALIDATION;
}

PlayerOfficial::~PlayerOfficial() { DO_VALIDATION; }

HumanoidBase *PlayerOfficial::CastHumanoid() {
  DO_VALIDATION;
  return static_cast<HumanoidBase *>(humanoid.get());
}

RefereeController *PlayerOfficial::CastController() {
  DO_VALIDATION;
  return static_cast<RefereeController *>(controller.get());
}

void PlayerOfficial::Activate(boost::intrusive_ptr<Node> humanoidSourceNode,
                              boost::intrusive_ptr<Node> fullbodySourceNode,
                              std::map<Vector3, Vector3> &colorCoords,
                              boost::intrusive_ptr<Resource<Surface> > kit,
                              boost::shared_ptr<AnimCollection> animCollection,
                              bool lazyPlayer) {
  DO_VALIDATION;
  isActive = true;
  humanoid.reset(new HumanoidBase(
      this, match, humanoidSourceNode, fullbodySourceNode, colorCoords,
      animCollection, match->GetDynamicNode(), kit));

  CastHumanoid()->ResetPosition(Vector3(0), Vector3(0));

  controller.reset(new RefereeController(match));
  controller->SetPlayer(this);
}

void PlayerOfficial::Deactivate() {
  DO_VALIDATION;
  PlayerBase::Deactivate();
}

void PlayerOfficial::Process() {
  DO_VALIDATION;
  CastController()->Process();
  CastHumanoid()->Process();
}

void PlayerOfficial::FetchPutBuffers() {
  DO_VALIDATION;
  PlayerBase::FetchPutBuffers();
}
