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

#include <cmath>

#include "../match.hpp"
#include "../team.hpp"

#include "controller/elizacontroller.hpp"
#include "controller/strategies/strategy.hpp"

#include "../../main.hpp"
#include "../../utils.hpp"

#include "../../base/geometry/triangle.hpp"

Player::Player(Team *team, PlayerData *playerData) : PlayerBase(team->GetMatch(), playerData), team(team) {
  menuTask = GetMenuTask();
  hasPossession = false;
  hasBestPossession = false;
  hasUniquePossession = false;
  possessionDuration_ms = 0;
  timeNeededToGetToBall_ms = 1000;
  timeNeededToGetToBall_optimistic_ms = 1000;
  timeNeededToGetToBall_previous_ms = 1000;
  SetDesiredTimeToBall_ms(0);
  manMarkingID = -1;
  buf_nameCaption = "...";
  buf_debugCaption = "debug";
  nameCaption = 0;
  debugCaption = 0;

  triggerControlledBallCollision = false;

  tacticalSituation.forwardSpaceRating = 0;
  tacticalSituation.toGoalSpaceRating = 0;
  tacticalSituation.spaceRating = 0;

  cards = 0;

  cardEffectiveTime_ms = 0;
}

Player::~Player() {
  if (nameCaption) menuTask->GetWindowManager()->MarkForDeletion(nameCaption);
  if (debugCaption) menuTask->GetWindowManager()->MarkForDeletion(debugCaption);
  menuTask.reset();
}

Humanoid *Player::CastHumanoid() { return static_cast<Humanoid*>(humanoid); }

ElizaController *Player::CastController() {
  return static_cast<ElizaController*>(controller);
}

int Player::GetTeamID() const {
  return team->GetID();
}

Team *Player::GetTeam() {
  return team;
}

void Player::Activate(boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection, bool lazyPlayer) {

  assert(!isActive);

  isActive = true;

  humanoid = new Humanoid(this, humanoidSourceNode, fullbodySourceNode, colorCoords, animCollection, GetTeam()->GetSceneNode(), kit, GetTeam()->GetID());

  controller = new ElizaController(match, lazyPlayer);
  CastController()->SetPlayer(this);
  CastController()->LoadStrategies();

  buf_nameCaptionShowCondition = false;

  nameCaption = new Gui2Caption(GetMenuTask()->GetWindowManager(), "game_player_name_" + int_to_str(id), 0, 0, 1, 2.0, playerData->GetLastName());
  nameCaption->SetTransparency(0.3f);
  GetMenuTask()->GetWindowManager()->GetRoot()->AddView(nameCaption);
  debugCaption = new Gui2Caption(GetMenuTask()->GetWindowManager(), "game_player_debug_" + int_to_str(id), 0, 0, 1, 1.6, "debug");
  GetMenuTask()->GetWindowManager()->GetRoot()->AddView(debugCaption);

  CastHumanoid()->ResetPosition(GetFormationEntry().position * 25 * Vector3(-team->GetSide(), -team->GetSide(), 0), Vector3(0));

  SetDynamicFormationEntry(GetFormationEntry());
}

void Player::Deactivate() {
  ResetSituation(GetPosition());

  menuTask->GetWindowManager()->MarkForDeletion(nameCaption);
  menuTask->GetWindowManager()->MarkForDeletion(debugCaption);
  nameCaption = 0;
  debugCaption = 0;

  if (team->IsHumanControlled(this->GetID())) {
    team->DeselectPlayer(this); // don't want any humangamer to have control of this player anymore
  }

  PlayerBase::Deactivate();
  GetTeam()->UpdateDesignatedTeamPossessionPlayer();
}

FormationEntry Player::GetFormationEntry() {
  return team->GetFormationEntry(id);
}

bool Player::HasPossession() const {
  return hasPossession;
}

bool Player::HasBestPossession() const {
  return hasBestPossession;
}

bool Player::HasUniquePossession() const {
  return hasUniquePossession;
}

bool Player::AllowLastDitch(bool includingPossessionAmount) const {
  if (includingPossessionAmount && team->GetTeamPossessionAmount() < 1.0f) return true; // why team possession amount and not player's? answer: because we don't have that info here (todo: fix that)
  return (GetTimeNeededToGetToBall_optimistic_ms() * 1.7f + 800 < GetTimeNeededToGetToBall_ms());
}

float Player::GetAverageVelocity(float timePeriod_sec) {
  assert((int)timePeriod_sec > 0);
  unsigned int logSize = positionHistoryPerSecond.size();
  if (logSize == 0) return 0;
  Vector3 prevPos;
  float totalDistance = 0;
  unsigned int count = 0;
  while (count <= (unsigned int)timePeriod_sec) {
    Vector3 pos = positionHistoryPerSecond.at(logSize - 1 - count);
    if (count > 0) totalDistance += (pos - prevPos).GetLength();
    count++;
    if (logSize - count == 0) break;
    prevPos = pos;
  }
  return totalDistance / timePeriod_sec; // don't divide by count, since lack of entries should not influence average
}

void Player::UpdatePossessionStats(bool onInterval) {

  if (!onInterval) return;

  timeNeededToGetToBall_previous_ms = timeNeededToGetToBall_ms;

  // default
  timeNeededToGetToBall_ms = std::max(
      ballPredictionSize_ms,
      (unsigned int)(std::round(
          (match->GetBall()->Predict(ballPredictionSize_ms - 10).Get2D() -
           (GetPosition() + GetMovement() * 0.2f))
              .GetLength() /
          (GetMaxVelocity() * 0.75f) * 1000)));
  timeNeededToGetToBall_optimistic_ms = timeNeededToGetToBall_ms;

  unsigned int startTime_ms = 0;
  if ((CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_ShortPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_LongPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_HighPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_Shot) && !TouchPending()) {
    startTime_ms = 500;
  }

  bool refine = false;
  unsigned int timeStep_ms = 10;
  unsigned int previous_ms = 0;
  bool precise = (team->GetDesignatedTeamPossessionPlayer() == this) ? true : false;
  float previousDist = 0; // debug
  for (unsigned int ms = startTime_ms; ms < ballPredictionSize_ms; ms += timeStep_ms) {
    if (match->GetBall()->Predict(ms).coords[2] < 1.5f) {
      TimeNeeded result = AI_GetTimeNeededForDistance_ms(GetPosition(), GetMovement(), match->GetBall()->Predict(ms).Get2D(), GetMaxVelocity(), precise, ms);
      unsigned int timeNeeded = result.usual_ms;
      unsigned int timeNeeded_optimistic = result.optimistic_ms;

      if (timeNeeded_optimistic <= ms) {
        if (ms < timeNeededToGetToBall_optimistic_ms) timeNeededToGetToBall_optimistic_ms = ms;
      }

      if (timeNeeded <= ms) {

        // refinement round!
        if (!refine) {

          ms = previous_ms;
          timeStep_ms = 10;
          refine = true;
          // found!
        } else {

          timeNeededToGetToBall_ms = ms;
          break;

        }
      }
    }

    // refine timestep (optimisation)
    if (!refine) {
      float balldist = (GetPosition() - match->GetBall()->Predict(ms).Get2D()).GetLength() + 0.2f; // add a little buffer
      float maxBallVelo = 50;
      // how long does it take for the ball at max velo to travel balldist?
      unsigned int timeToGo_ms =
          int(std::round((balldist / maxBallVelo) * 1000.0f));
      timeStep_ms = clamp(timeToGo_ms, 10, 500);
      // round to 10s
      timeStep_ms = (timeStep_ms / 10) * 10;
    } else timeStep_ms = 10;

    previous_ms = ms;
  }

  if (TouchAnim() && TouchPending()) {
    unsigned int animTimeToBall_ms = (CastHumanoid()->GetTouchFrame() - GetCurrentFrame()) * 10;
    timeNeededToGetToBall_ms = std::min(timeNeededToGetToBall_ms, animTimeToBall_ms);
    timeNeededToGetToBall_optimistic_ms = timeNeededToGetToBall_ms;
  }

  if (timeNeededToGetToBall_ms < defaultTouchOffset_ms) {
    // apply quantum mechanics on the scale of the very small ;)
    timeNeededToGetToBall_ms = NormalizedClamp(((GetPosition() + GetMovement() * (defaultTouchOffset_ms * 0.001)) - match->GetBall()->Predict(defaultTouchOffset_ms).Get2D()).GetLength(), 0.0f, 0.6f) * defaultTouchOffset_ms;
    timeNeededToGetToBall_optimistic_ms = timeNeededToGetToBall_ms;
  }

  if ((CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_ShortPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_LongPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_HighPass ||
       CastHumanoid()->GetCurrentFunctionType() == e_FunctionType_Shot) && !TouchPending()) {
    hasPossession = false;
  } else {
    hasPossession = AI_HasPossession(match->GetBall(), this);
  }

  this->hasBestPossession = hasPossession && match->GetTeam(abs(team->GetID() - 1))->GetTimeNeededToGetToBall_ms() > this->GetTimeNeededToGetToBall_ms();
  this->hasUniquePossession = hasPossession && !match->GetTeam(abs(team->GetID() - 1))->HasPossession();

  if (match->GetBallRetainer() == this) {
    timeNeededToGetToBall_ms = 1;
    timeNeededToGetToBall_optimistic_ms = 1;
    SetDesiredTimeToBall_ms(timeNeededToGetToBall_ms);
    hasPossession = true;
    hasBestPossession = true;
    hasUniquePossession = true;
  } else if (match->GetBallRetainer() != 0) {
    hasPossession = false;
    hasBestPossession = false;
    hasUniquePossession = false;
  }

}

float Player::GetClosestOpponentDistance() const {
  Player *opp = AI_GetClosestPlayer(match->GetTeam(abs(team->GetID() - 1)), GetPosition(), false);
  return opp->GetPosition().GetDistance(GetPosition());
}

void Player::Process() {

  if (isActive) {

    desiredTimeToBall_ms = std::max(desiredTimeToBall_ms - 10, 0);

    if (externalController) externalController->Process(); else CastController()->Process();

    if (match->IsInPlay()) {
      if (match->GetActualTime_ms() % 1000 == 0) {
        positionHistoryPerSecond.push_back(GetPosition());
      }
      if (hasPossession) possessionDuration_ms += 10; else possessionDuration_ms = 0;
      if ((match->GetActualTime_ms() + GetStableID() * 10) % 100 == 0) {
        _CalculateTacticalSituation();
      }
    }

    Vector3 posBefore = CastHumanoid()->GetPosition();

    CastHumanoid()->Process();

    Vector3 posAfter = CastHumanoid()->GetPosition();

    float distance = (posAfter - posBefore).GetLength();
    fatigueFactorInv -= distance * 0.00003f * (2.0f - GetStaminaStat()) * (1.0f / match->GetMatchDurationFactor());
    fatigueFactorInv = clamp(fatigueFactorInv, 0.01f, 1.0f);
    // Don't send off the last player on the team.
    if (cards > 1 && cardEffectiveTime_ms <= match->GetActualTime_ms()
        && GetTeam()->GetActivePlayersCount() > 1) {
      SendOff();
    }
  }

}

void Player::PreparePutBuffers(unsigned long snapshotTime_ms) {

  PlayerBase::PreparePutBuffers(snapshotTime_ms);

  buf_nameCaptionShowCondition = team->IsHumanControlled(id);
  if (team->GetHumanGamerCount() == 0) buf_nameCaptionShowCondition = team->GetDesignatedTeamPossessionPlayer() == this;
  e_PlayerColor playerColor = team->GetPlayerColor(id);
  switch (playerColor) {
    case e_PlayerColor_Green:
      buf_playerColor = Vector3(100, 255, 140);
      break;
    case e_PlayerColor_Red:
      buf_playerColor = Vector3(255, 110, 110);
      break;
    case e_PlayerColor_Blue:
      buf_playerColor = Vector3(100, 140, 255);
      break;
    case e_PlayerColor_Yellow:
      buf_playerColor = Vector3(255, 255, 60);
      break;
    case e_PlayerColor_Purple:
      buf_playerColor = Vector3(200, 80, 200);
      break;
    case e_PlayerColor_Default:
      buf_playerColor = Vector3(200, 200, 200);
      break;
  };

  std::string name = playerData->GetLastName();

  buf_nameCaption = name;

  if (GetExternalController()) {
    if (static_cast<HumanController*>(GetExternalController())->GetActionMode() == 1) {
      //buf_nameCaption += " X";
    } else if (static_cast<HumanController*>(GetExternalController())->GetActionMode() == 2) {
      //buf_nameCaption += " !";
      buf_playerColor =
          buf_playerColor *
          (std::sin(match->GetActualTime_ms() * 0.02f) * 0.3f + 0.7f);
    }

    //if (hasPossession) buf_nameCaption.append(" P");
  }

}

void Player::FetchPutBuffers(unsigned long putTime_ms) {

  PlayerBase::FetchPutBuffers(putTime_ms);

  fetchedbuf_nameCaptionShowCondition = buf_nameCaptionShowCondition;
  fetchedbuf_debugCaptionShowCondition = false;
  fetchedbuf_nameCaption = buf_nameCaption;
  fetchedbuf_debugCaption = buf_debugCaption;
  fetchedbuf_nameCaptionPos = buf_nameCaptionPos;
  fetchedbuf_debugCaptionPos = buf_debugCaptionPos;
  fetchedbuf_playerColor = buf_playerColor;
  fetchedbuf_debugCaptionColor = buf_debugCaptionColor;
}

void Player::Put2D() {
  if (fetchedbuf_nameCaptionShowCondition) {
    //Vector3 captionPos3D = fetchedbuf_nameCaptionPos;
    //Vector3 captionPos2D = GetProjectedCoord(captionPos3D, match->GetCamera());
    Vector3 captionPos3D = GetProjectedCoord(GetGeomPosition() + Vector3(0, 0.5f, 2.4f), match->GetCamera()); // geom pos because in Put2D, we cannot access normal class vars (because multithreading)
    float w, h;
    nameCaption->GetSize(w, h);
    nameCaption->SetColor(fetchedbuf_playerColor);
    nameCaption->SetOutlineColor(fetchedbuf_playerColor * 0.4f);
    nameCaption->SetPosition(captionPos3D.coords[0] - w * 0.5f, captionPos3D.coords[1] - h);

    nameCaption->SetCaption(fetchedbuf_nameCaption);
    nameCaption->Show();
  } else {
    nameCaption->Hide();
  }

  if (fetchedbuf_debugCaptionShowCondition) {
    Vector3 captionPos3D = GetProjectedCoord(GetGeomPosition() + Vector3(0, 0.3f, 2.0f), match->GetCamera());
    float w, h;
    debugCaption->GetSize(w, h);
    debugCaption->SetPosition(captionPos3D.coords[0] - w * 0.5f, captionPos3D.coords[1] - h);
    //printf("%s\n", fetchedbuf_debugCaption.c_str());
    debugCaption->SetCaption(fetchedbuf_debugCaption);
    debugCaption->SetColor(fetchedbuf_debugCaptionColor);
    debugCaption->Show();
  } else {
    debugCaption->Hide();
  }
}

void Player::Hide2D() {
  if (fetchedbuf_nameCaptionShowCondition) {
    assert(nameCaption);
    nameCaption->Hide();
  }
  if (fetchedbuf_debugCaptionShowCondition) {
    assert(debugCaption);
    debugCaption->Hide();
  }
}

void Player::SendOff() {
  float x = random(0, 3);
  std::string message;
  if (x < 1.0) {
    message = "an early shower for " + playerData->GetLastName() + "!";
  } else if (x < 2.0) {
    message = playerData->GetLastName() + " is sent off!";
  } else {
    message = "it's all over for " + playerData->GetLastName() + "!";
  }
  match->SpamMessage(message);

  Deactivate();

  if (GetFormationEntry().role == e_PlayerRole_GK) {
    FormationEntry entry = GetFormationEntry();
    std::vector<Player*> activePlayers;
    team->GetActivePlayers(activePlayers);
    assert(activePlayers.size() > 0);
    int newGoalieID = (*activePlayers.begin())->GetID();
    team->SetFormationEntry(newGoalieID, entry);
  }

  std::vector<Player*> activePlayers;
  team->GetActivePlayers(activePlayers);

  // For simplicity don't forfeit for now.
  // int remainingPlayers = activePlayers.size();
  // if (remainingPlayers <= 6) {
    // too many red cards - forfeit

}

float Player::GetStaminaStat() const {
  return playerData->GetStat(physical_stamina);
}

float Player::GetStat(PlayerStat name) const {
  float multiplier = 1.0f;
  if (team->GetHumanGamerCount() == 0) multiplier = 0.3f + 0.7f * team->GetMatch()->GetMatchDifficulty();
  multiplier *= 0.7f + 0.3f * GetFatigueFactorInv();

  return playerData->GetStat(name) * multiplier;
}

void Player::ResetSituation(const Vector3 &focusPos) {
  PlayerBase::ResetSituation(focusPos);

  hasPossession = false;
  hasBestPossession = false;
  hasUniquePossession = false;
  possessionDuration_ms = 0;
  timeNeededToGetToBall_ms = 1000;
  timeNeededToGetToBall_optimistic_ms = 1000;
  SetDesiredTimeToBall_ms(0);
  manMarkingID = -1;

  triggerControlledBallCollision = false;

  tacticalSituation.forwardSpaceRating = 0;
  tacticalSituation.toGoalSpaceRating = 0;
  tacticalSituation.spaceRating = 0;
}

void Player::_CalculateTacticalSituation() {
  const MentalImage *mentalImage = static_cast<PlayerController*>(GetController())->GetMentalImage();
  assert(mentalImage);
  assert(IsActive());

  // calculate how free the path forward is
  float time_sec = 0.5f;
  Vector3 checkPos = GetPosition() + Vector3(-team->GetSide(), 0, 0) * sprintVelocity * time_sec;
  tacticalSituation.forwardSpaceRating = AI_CalculateFreeSpace(match, mentalImage, team->GetID(), checkPos, 5.0f, time_sec); // FREESPACE :D :D

  // calculate the amount of space this player has
  //tacticalSituation.spaceRating = AI_CalculatePersonalFreeSpace(match, mentalImage, this, 8.0f, 8.0f, 0.2f);
  time_sec = 0.1f;
  checkPos = GetPosition() + GetMovement() * time_sec;
  tacticalSituation.spaceRating = AI_CalculateFreeSpace(match, mentalImage, team->GetID(), checkPos, 5.0f, time_sec); // FREESPACE :D :D

  // distance to opponent goal 0 .. 1 == farthest .. closest
  tacticalSituation.forwardRating = 1.0f - clamp((Vector3(pitchHalfW * -team->GetSide(), 0, 0) - GetPosition()).GetLength() / (pitchHalfW * 2.0f), 0.0f, 1.0f);
  tacticalSituation.forwardRating =
      std::pow(tacticalSituation.forwardRating,
               1.5f);  // more important when close to goal
}
