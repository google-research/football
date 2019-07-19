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

#include "default_off.hpp"
#include <cmath>

#include "../../../../../main.hpp"

DefaultOffenseStrategy::DefaultOffenseStrategy(ElizaController *controller) : Strategy(controller) {
  name = "default offense";
}

DefaultOffenseStrategy::~DefaultOffenseStrategy() {
}

void DefaultOffenseStrategy::RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity) {

  bool offensiveComponents = true;
  bool defensiveComponents = true;
  bool laziness = true;

  Vector3 desiredPosition_static = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), false);
  Vector3 desiredPosition_dynamic = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), true);
  float actionDistance = NormalizedClamp(player->GetPosition().GetDistance(match->GetDesignatedPossessionPlayer()->GetPosition()), 15.0f, 20.0f);
  float staticPositionBias = curve(0.8f * actionDistance, 1.0f); // lower values = swap position with other players' formation positions more easily
  Vector3 desiredPosition = desiredPosition_static * staticPositionBias + desiredPosition_dynamic * (1.0f - staticPositionBias);

  if (offensiveComponents) {
    // support position
    float attackBias = NormalizedClamp((controller->GetFadingTeamPossessionAmount() - 0.5f) * 1.0f, 0.1f, 0.6f);
    bool makeRun = false;
    if (attackBias > 0.7f) {
      if (team->GetController()->GetEndApplyAttackingRun_ms() > match->GetActualTime_ms() && team->GetController()->GetAttackingRunPlayer() == player) {
        makeRun = true;
      }
    }
    Vector3 supportPosition = controller->GetSupportPosition_ForceField(mentalImage, desiredPosition, makeRun);
    desiredPosition = desiredPosition * (1.0f - attackBias) + supportPosition * attackBias;
  }

  if (defensiveComponents) {
    float mindset = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
    controller->AddDefensiveComponent(
        desiredPosition,
        std::pow(
            clamp(1.3f - mindset - controller->GetFadingTeamPossessionAmount(),
                  0.0f, 1.0f),
            0.7f));
  }

  direction = (desiredPosition - player->GetPosition()).GetNormalized(player->GetDirectionVec());
  float desiredVelocity = (desiredPosition - player->GetPosition()).GetLength() * distanceToVelocityMultiplier;

  // laziness
  if (laziness) desiredVelocity = controller->GetLazyVelocity(desiredVelocity);

  desiredVelocity = clamp(desiredVelocity, 0, sprintVelocity);

  velocity = desiredVelocity;
}
