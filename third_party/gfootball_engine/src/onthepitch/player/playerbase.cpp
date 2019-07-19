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

#include "player.hpp"

#include "../match.hpp"

#include "controller/elizacontroller.hpp"
#include "controller/strategies/strategy.hpp"

#include "../../main.hpp"
#include "../../utils.hpp"

#include "../../base/geometry/triangle.hpp"

int PlayerBase::playerCount = 0;
int PlayerBase::stablePlayerCount = 0;

PlayerBase::PlayerBase(Match *match, PlayerData *playerData) : match(match), playerData(playerData), id(playerCount++), stable_id(stablePlayerCount++), humanoid(0), controller(0), externalController(0), isActive(false) {
  lastTouchTime_ms = 0;
  lastTouchType = e_TouchType_None;
  fatigueFactorInv = 1.0;
}

PlayerBase::~PlayerBase() {
  if (isActive) Deactivate();
  if (humanoid) delete humanoid;
}

void PlayerBase::Deactivate() {
  ResetSituation(GetPosition());

  isActive = false;

  if (humanoid) humanoid->Hide();

  externalController = nullptr;
  delete controller;
}

IController *PlayerBase::GetController() {
  if (externalController) return externalController;
                     else return controller;
}

void PlayerBase::RequestCommand(PlayerCommandQueue &commandQueue) {
  if (externalController) externalController->RequestCommand(commandQueue);
                     else controller->RequestCommand(commandQueue);
}

void PlayerBase::SetExternalController(HumanController *externalController) {
  this->externalController = externalController;
  if (this->externalController) {
    this->externalController->Reset();
    this->externalController->SetPlayer(this);
    //debug = true;
  } else {
    controller->Reset();
    //debug = false;
  }
}

HumanController *PlayerBase::GetExternalController() {
  return externalController;
}

void PlayerBase::Process() {
  if (isActive) {
    if (externalController) externalController->Process(); else controller->Process();
    humanoid->Process();
  } else {
    if (humanoid) humanoid->Hide();
  }
  //if (debug) printf("::%f velo\n", GetMovement().GetLength());
}

void PlayerBase::PreparePutBuffers(unsigned long snapshotTime_ms) {
  humanoid->PreparePutBuffers(snapshotTime_ms);
}

void PlayerBase::FetchPutBuffers(unsigned long putTime_ms) {
  humanoid->FetchPutBuffers(putTime_ms);
}

void PlayerBase::Put() {
  humanoid->Put();
}

float PlayerBase::GetStat(PlayerStat name) const {
  return playerData->GetStat(name);
}

float PlayerBase::GetMaxVelocity() const {
  // see humanoidbase's physics function
  return sprintVelocity * GetVelocityMultiplier();
}

float PlayerBase::GetVelocityMultiplier() const {
  // see humanoid_utils' physics function
  return 0.9f + playerData->get_physical_velocity() * 0.1f;
}

float PlayerBase::GetLastTouchBias(int decay_ms, unsigned long time_ms) {
  unsigned long adaptedTime_ms = time_ms;
  if (time_ms == 0) adaptedTime_ms = match->GetActualTime_ms();
  if (decay_ms > 0) return 1.0f - clamp((adaptedTime_ms - GetLastTouchTime_ms()) / (float)decay_ms, 0.0f, 1.0f);
  return 0.0f;
}

void PlayerBase::ResetSituation(const Vector3 &focusPos) {
  positionHistoryPerSecond.clear();
  lastTouchTime_ms = 0;
  lastTouchType = e_TouchType_None;
  if (IsActive()) humanoid->ResetSituation(focusPos);
  if (GetController()) GetController()->Reset();
}
