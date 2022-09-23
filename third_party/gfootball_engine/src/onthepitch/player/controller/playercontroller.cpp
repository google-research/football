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

#include "playercontroller.hpp"
#include <cmath>

#include "../../match.hpp"
#include "../../team.hpp"
#include "../player.hpp"
#include "../humanoid/humanoid_utils.hpp"

#include "../../AIsupport/mentalimage.hpp"
#include "../../AIsupport/AIfunctions.hpp"

#include "../../../main.hpp"

PlayerController::PlayerController(Match *match) : IController(match) {
  DO_VALIDATION;
  Reset();
}

void PlayerController::Process() {
  DO_VALIDATION;
  int reactionTime_ms = GetReactionTime_ms();
  if (match->GetLastTouchPlayer() == CastPlayer() && CastPlayer()->GetLastTouchType() != e_TouchType_Accidental) reactionTime_ms = 0;
  _mentalImageTime = reactionTime_ms;
}

void PlayerController::SetPlayer(PlayerBase *player) {
  DO_VALIDATION;
  IController::SetPlayer(player);
  this->team = CastPlayer()->GetTeam();
  this->oppTeam = match->GetTeam(abs(this->team->GetID() - 1));
  assert(this->oppTeam);
}

Player *PlayerController::CastPlayer() {
  DO_VALIDATION;
  return static_cast<Player *>(player);
}

const MentalImage *PlayerController::GetMentalImage() {
  DO_VALIDATION;
  return match->GetMentalImage(_mentalImageTime);
}

int PlayerController::GetReactionTime_ms() {
  DO_VALIDATION;
  int reactionTime_ms = IController::GetReactionTime_ms();
  reactionTime_ms += (1.0f - GetTeam()->GetAiDifficulty()) * 100;
  return reactionTime_ms;
}

float PlayerController::GetLastSwitchBias() {
  DO_VALIDATION;
  if (!match->IsInPlay() || match->IsInSetPiece()) {
    DO_VALIDATION;
    lastSwitchTime_ms = -10000;
  }
  if (lastSwitchTimeDuration_ms > 0) {
    DO_VALIDATION;
    return 1.0f - clamp((match->GetActualTime_ms() - lastSwitchTime_ms) /
                            (float)lastSwitchTimeDuration_ms,
                        0.0f, 1.0f);
  }
  DO_VALIDATION;
  return 0.0f;
}

void PlayerController::AddDefensiveComponent(Vector3 &desiredPosition,
                                             float bias, Player *forcedOpp) {
  DO_VALIDATION;

  Player* opponent = 0;
  if (!forcedOpp) opponent = CastPlayer()->GetManMarking(); else
                         opponent = forcedOpp;

  if (match->IsInPlay() && !match->IsInSetPiece() &&
      CastPlayer()->GetFormationEntry().role != e_PlayerRole_GK) {
    DO_VALIDATION;

    if (opponent) {
      DO_VALIDATION;

      Vector3 defendPosition = desiredPosition;

      float possessionPlayerShootThreshold = 24.0f; // distance at which opp must be marked tightly
      float genericOpponentShootThreshold = 8.0f; // distance at which opp must be marked tightly
      float minDistance = 0.4f; // distance 'in front' of opp - how far from opp we defend as minimum
      float bufferDistance = 4.0f; // we want to be at least this distance closer to shooting point than opp

      // calculate some basic vars
      PlayerImage oppImage = match->GetMentalImage(GetReactionTime_ms())->GetPlayerImage(opponent);
      Vector3 oppPos = oppImage.position + oppImage.movement * 0.5f;

      float shootThreshold = genericOpponentShootThreshold;
      if (opponent == match->GetDesignatedPossessionPlayer()) {
        DO_VALIDATION;
        shootThreshold = possessionPlayerShootThreshold;
      }

      Vector3 goalPos = Vector3(pitchHalfW * team->GetDynamicSide(), 0, 0);

      // calculate how close the opponent is to the goal/shooting threshold
      float oppToGoalDistance = (goalPos - oppPos).GetLength();
      float oppToThresholdDistance = clamp(oppToGoalDistance - shootThreshold, minDistance, pitchHalfW);
      Vector3 shootingPoint = oppPos + (goalPos - oppPos).GetNormalized(0) * oppToThresholdDistance;

      // if shootingPoint.coords[0] exceeds offside trap line, alter oppToThresholdDistance in such a way, that it results in the shootingPoint being at least offsideTrapX distance away from goal (unless player is closer already)
      if (shootingPoint.coords[0] * team->GetDynamicSide() >
          team->GetController()->GetOffsideTrapX() * team->GetDynamicSide()) {
        DO_VALIDATION;
        Line oppToGoalLine;
        oppToGoalLine.SetVertex(0, oppPos);
        oppToGoalLine.SetVertex(1, goalPos);
        Line offsideLine;
        offsideLine.SetVertex(0, Vector3(team->GetController()->GetOffsideTrapX(), -pitchHalfH, 0));
        offsideLine.SetVertex(1, Vector3(team->GetController()->GetOffsideTrapX(),  pitchHalfH, 0));
        shootingPoint = oppToGoalLine.GetIntersectionPoint(offsideLine);
        oppToThresholdDistance = (shootingPoint - oppPos).GetLength();
      }

      // now we want to keep the distance to shootingPoint smaller than the opponent's.
      Vector3 meToThreshold = shootingPoint - desiredPosition;//CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.14f;
      float meToThresholdDistance = meToThreshold.GetLength();

      float slackedDistance = meToThresholdDistance - (oppToThresholdDistance - bufferDistance);
      if (slackedDistance > 0.0f) {
        DO_VALIDATION;
        defendPosition = desiredPosition + meToThreshold.GetNormalized(0) * clamp(slackedDistance, 0.0f, meToThresholdDistance);
      }

      // if we are too late, don't run straight towards the shootingPoint, but instead, more towards our goal
      Vector3 actualPos = CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.14f;
      float actualToThresholdDistance = (shootingPoint - actualPos).GetLength();
      float actualSlackedDistance = actualToThresholdDistance - oppToThresholdDistance;//(oppToThresholdDistance - bufferDistance);
      if (actualSlackedDistance > 0.0f) {
        DO_VALIDATION;
        defendPosition += (goalPos - defendPosition).GetNormalized(0) * actualSlackedDistance * 0.7f;
      }

      desiredPosition = desiredPosition * (1.0f - bias) +
                        defendPosition  * bias;
    }
  }
}

Vector3 PlayerController::GetDefendPosition(Player *opp, float distance) {
  DO_VALIDATION;

  PlayerImage oppImage = match->GetMentalImage(_mentalImageTime)->GetPlayerImage(opp);

  // find the position on the opp -> goal line where we want to go to intercept. this point is the same distance away from opp as it is from us.
  // to find this point:
  // 1) create a line AB between us and op
  // 2) create another line CD perpendicular to AB, that intersects the middle point of AB
  // 3) find the intersection point of CD with the original opp to goal line. if this intersection point is between opp and goal, it's our target. (if not, we're too late, just run to goal :p)

  Vector3 goalPos = Vector3(pitchHalfW * team->GetDynamicSide(), 0, 0);
  Vector3 oppPosition = oppImage.position; // lol @ varname
  Vector3 oppToGoal = goalPos - oppPosition;

  Line oppToGoalLine;
  oppToGoalLine.SetVertex(0, oppPosition);
  oppToGoalLine.SetVertex(1, goalPos);
  // 1
  Line meToOppLine;
  meToOppLine.SetVertex(0, player->GetPosition());
  meToOppLine.SetVertex(1, oppPosition);
  Vector3 midPoint = meToOppLine.GetVertex(0) * 0.5f + meToOppLine.GetVertex(1) * 0.5f;
  // 2
  Line midToOppPerpendicularLine;
  midToOppPerpendicularLine.SetVertex(0, midPoint);
  midToOppPerpendicularLine.SetVertex(1, midPoint + (meToOppLine.GetVertex(0) - midPoint).GetRotated2D(0.5 * pi));
  // 3
  float u = 0.0f;
  Vector3 intersect = oppToGoalLine.GetIntersectionPoint(midToOppPerpendicularLine, u);

  u = clamp(u, 0.0f, 1.0f);
  Vector3 target = oppToGoalLine.GetVertex(0) + (oppToGoalLine.GetVertex(1) - oppToGoalLine.GetVertex(0)) * u;

  /*
    if (player->GetDebug()) { DO_VALIDATION;
      SetRedDebugPilon(intersect + Vector3(0, 0, 0.1f));
      SetBlueDebugPilon(target);
    }
  */

  target += (oppToGoal.GetNormalized(0) * sprintVelocity * 0.1f) + (oppImage.movement * 0.14f) + (oppToGoal.GetNormalized(0) * distance);

  return target;
}

void PlayerController::ProcessPlayerController(EnvState *state) {
  DO_VALIDATION;
  auto p = CastPlayer();
  state->process(p);
  player = p;
  state->process(inputDirection);
  state->process(inputVelocityFloat);
  state->process(team);
  state->process(oppTeam);
  state->process(_oppPlayer);
  state->process(_timeNeeded_ms);
  state->process(_mentalImageTime);
  state->process(lastSwitchTime_ms);
  state->process(lastSwitchTimeDuration_ms);
  state->process(hasPossession);
  state->process(hasUniquePossession);
  state->process(teamHasPossession);
  state->process(teamHasUniquePossession);
  state->process(oppTeamHasPossession);
  state->process(oppTeamHasUniquePossession);
  state->process(hasBestPossession);
  state->process(teamHasBestPossession);
  state->process(possessionAmount);
  state->process(teamPossessionAmount);
  state->process(fadingTeamPossessionAmount);
  state->process(oppTimeNeededToGetToBall);
  state->process(hasBestChanceOfPossession);
}

void PlayerController::Reset() {
  DO_VALIDATION;

  lastSwitchTimeDuration_ms = 0;
  lastSwitchTime_ms = -10000;
  _mentalImageTime = 0;
  inputDirection = Vector3(0, -1, 0);
  inputVelocityFloat = 0;

  hasPossession = false;
  hasUniquePossession = false;
  teamHasPossession = false;
  teamHasUniquePossession = false;
  oppTeamHasPossession = false;
  oppTeamHasUniquePossession = false;
  hasBestChanceOfPossession = false;
  teamHasBestPossession = false;
  possessionAmount = 0.9f;
  teamPossessionAmount = 1.0f;
  fadingTeamPossessionAmount = 1.0f;
  oppTimeNeededToGetToBall = 100;
  hasBestPossession = false;
  _timeNeeded_ms = 0;
}

float PlayerController::OppBetweenBallAndMeDot() {
  DO_VALIDATION;
  Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();
  Vector3 MeToOpp = (opp->GetPosition() + opp->GetMovement() * 0.1f) - (CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.1f);
  Vector3 oppToBall = match->GetMentalImage(_mentalImageTime)->GetBallPrediction(100).Get2D() - (opp->GetPosition() + opp->GetMovement() * 0.1f);
  float dot = MeToOpp.GetNormalized(0).GetDotProduct(oppToBall.GetNormalized(0));

  // if dot nears 1, it means opp is somewhat between ball and me
  return dot;
}

float PlayerController::CouldWinABallDuelLikeliness() {
  DO_VALIDATION;
  // uses the OppBetweenBallAndMeDot to check for correct angle, but also checks ball distance from opp
  float dot = OppBetweenBallAndMeDot() * 0.5f + 0.5f;

  /* taking into account the relative distance to ball doesn't really seem to work all too well
  Vector3 meToBall = _mentalImage->GetBallPrediction(0).Get2D() - player->GetPosition();

  Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();
  Vector3 oppToBall = _mentalImage->GetBallPrediction(0).Get2D() - opp->GetPosition();

  float meBallDistFactor = NormalizedClamp(meToBall.GetLength(), 0.0f, 2.0f);
  float oppBallDistFactor = NormalizedClamp(oppToBall.GetLength(), 0.0f, 2.0f);

  return (1.0f - dot) * NormalizedClamp(oppBallDistFactor - meBallDistFactor, -1.0f, 0.0f); // 'same distance' is considered best now
  */

  return (1.0f - dot);
}

void PlayerController::_Preprocess() {
  DO_VALIDATION;
  _oppPlayer = match->GetTeam(abs(team->GetID() - 1))->GetBestPossessionPlayer();
  _timeNeeded_ms = CastPlayer()->GetTimeNeededToGetToBall_ms(); // needed for synced version - if we would ask on the spot and compare to opp, opp may already have calculated a new one while ours has not been processed yet
}

void PlayerController::_KeeperDeflectCommand(PlayerCommandQueue &commandQueue,
                                             bool onlyPickupAnims) {
  DO_VALIDATION;

  if (CastPlayer()->GetFormationEntry().role != e_PlayerRole_GK) return;
  if (match->GetBall()->Predict(400).GetDistance(player->GetPosition()) > ballDistanceOptimizeThreshold + 10.0f) return;

  // can't use hands if teammate intentionally kicked ball to us
  if (match->GetLastTouchTeamID() == team->GetID() && match->GetLastTouchPlayer() != CastPlayer() && match->GetLastTouchTeamID(e_TouchType_Intentional_Kicked) == team->GetID()) return;

  if (match->GetBallRetainer() != 0) return;

  // can't use hands outside of keeper's 16 yard box (todo: make precise, probably in humanoid.cpp's getbestcheatableanim)
  if (fabs(match->GetBall()->Predict(160).coords[1]) > 20.05f) return;
  if (match->GetBall()->Predict(160).coords[0] * -team->GetDynamicSide() >
      -pitchHalfW + 16.4)
    return;

  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Deflect;
  command.useDesiredMovement = false;
  command.useDesiredLookAt = false;
  if (onlyPickupAnims) command.onlyDeflectAnimsThatPickupBall = true;
  commandQueue.push_back(command);
}

void PlayerController::_SetPieceCommand(PlayerCommandQueue &commandQueue) {
  DO_VALIDATION;
  // do not allow running away :p
  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Movement;
  command.useDesiredMovement = true;
  command.useDesiredLookAt = true;
  command.desiredDirection = CastPlayer()->GetDirectionVec();
  command.desiredVelocityFloat = idleVelocity;
  if (match->GetBallRetainer() == player) {
    DO_VALIDATION;
    command.desiredLookAt = player->GetPosition() + inputDirection * 10.0f;
  } else {
    command.desiredLookAt = match->GetBall()->Predict(0).Get2D();
  }
  commandQueue.push_back(command);
}

void PlayerController::_BallControlCommand(PlayerCommandQueue &commandQueue,
                                           bool idleTurnToOpponentGoal,
                                           bool knockOn,
                                           bool stickyRunDirection,
                                           bool keepCurrentBodyDirection) {
  DO_VALIDATION;
  if (match->GetBall()->Predict(200).GetDistance(player->GetPosition()) > ballDistanceOptimizeThreshold) return;
  if (match->GetBallRetainer() != 0) return;

  if (!CastPlayer()->HasPossession() && !CastPlayer()->AllowLastDitch()) {
    DO_VALIDATION;
    float strictnessInv = 3.0f;
    if (fabs(match->GetBall()->GetMovement().coords[2]) > 5.0f * strictnessInv) return;
    if ((match->GetBall()->GetMovement().Get2D() - CastPlayer()->GetMovement()).GetLength() > 5.0f * strictnessInv) return;
  }

  if (match->GetDesignatedPossessionPlayer() == player ||
      (team->GetDesignatedTeamPossessionPlayer() == player &&
       CouldWinABallDuelLikeliness() >= 0.25f)) {
    DO_VALIDATION;

    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_BallControl;
    command.useDesiredMovement = true;
    command.desiredDirection = inputDirection;
    if (quantizeDirection) QuantizeDirection(command.desiredDirection, GetQuantizedDirectionBias());
    command.desiredVelocityFloat = inputVelocityFloat;

    if (FloatToEnumVelocity(command.desiredVelocityFloat) == e_Velocity_Idle &&
        idleTurnToOpponentGoal) {
      DO_VALIDATION;
      command.desiredDirection = Vector3(-team->GetDynamicSide(), 0, 0);
    }

    if (knockOn) command.modifier |= e_PlayerCommandModifier_KnockOn;

    if (hasPossession && hasBestPossession &&
        command.desiredVelocityFloat > walkSprintSwitch &&
        CastPlayer()->GetFloatVelocity() > dribbleWalkSwitch &&
        stickyRunDirection) {
      DO_VALIDATION;
      radian angle = command.desiredDirection.GetAngle2D(CastPlayer()->GetDirectionVec());
      if (fabs(angle) > 0.125f * pi && fabs(angle) < 0.7f * pi) {
        DO_VALIDATION;
        if (angle > 0) command.desiredDirection = CastPlayer()->GetDirectionVec().GetRotated2D(0.125f *  pi);
        if (angle < 0) command.desiredDirection = CastPlayer()->GetDirectionVec().GetRotated2D(0.125f * -pi);
      }
    }

    command.useDesiredLookAt = true;
    if (keepCurrentBodyDirection &&
        (CastPlayer()->GetEnumVelocity() == e_Velocity_Walk ||
         CastPlayer()->GetEnumVelocity() == e_Velocity_Dribble) &&
        FloatToEnumVelocity(command.desiredVelocityFloat) ==
            e_Velocity_Dribble) {
      DO_VALIDATION;
      // sidestep dribble and such tricks! (not implemented yet, methinks)
      command.desiredVelocityFloat = walkVelocity;
      command.desiredLookAt = CastPlayer()->GetPosition() + CastPlayer()->GetDirectionVec() * 10.0f;
    } else {
      command.desiredLookAt = CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.2f + command.desiredDirection * 10.0f;
    }

    //if (player->GetDebug()) SetRedDebugPilon(player->GetPosition() + command.desiredDirection * (2.0f + command.desiredVelocityFloat));
    //if (player->GetDebug()) SetYellowDebugPilon(command.desiredLookAt);
    //if (player->GetDebug()) SetSmallDebugCircle1(player->GetPosition());
    commandQueue.push_back(command);
  }
}

void PlayerController::_TrapCommand(PlayerCommandQueue &commandQueue,
                                    bool idleTurnToOpponentGoal, bool knockOn) {
  DO_VALIDATION;

  if (match->GetBall()->GetMovement().Get2D().GetLength() < 2.0f) return;// - CastPlayer()->GetMovement()).GetLength()

  if (match->GetBall()->Predict(200).GetDistance(player->GetPosition()) > ballDistanceOptimizeThreshold) return;
  if (match->GetBallRetainer() != 0) return;

  if (!hasPossession &&
      (match->GetDesignatedPossessionPlayer() == player ||
       (team->GetDesignatedTeamPossessionPlayer() == player &&
        CastPlayer()->GetTimeNeededToGetToBall_optimistic_ms() < 1000 &&
        oppTimeNeededToGetToBall > 400 && !oppTeamHasPossession &&
        CouldWinABallDuelLikeliness() >= 0.5f))) {
    DO_VALIDATION;  // opp time was 500 ms

    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Trap;
    command.useDesiredMovement = true;
    command.desiredDirection = inputDirection;
    if (quantizeDirection) QuantizeDirection(command.desiredDirection, GetQuantizedDirectionBias());

    command.desiredVelocityFloat = inputVelocityFloat;
    if (CastPlayer()->GetFormationEntry().role == e_PlayerRole_GK &&
        !player->ExternalControllerActive()) {
      DO_VALIDATION;
      command.desiredDirection = Vector3(-team->GetDynamicSide(), 0, 0);
      command.desiredVelocityFloat = idleVelocity;
    }

    if (FloatToEnumVelocity(command.desiredVelocityFloat) == e_Velocity_Idle &&
        idleTurnToOpponentGoal) {
      DO_VALIDATION;
      command.desiredDirection = Vector3(-team->GetDynamicSide(), 0, 0);
    }

    if (knockOn) {
      DO_VALIDATION;
      command.modifier |= e_PlayerCommandModifier_KnockOn;
    }

    command.useDesiredLookAt = true;
    command.desiredLookAt = CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.3f + command.desiredDirection * 10.0f;

    commandQueue.push_back(command);
  }
}

void PlayerController::_InterfereCommand(PlayerCommandQueue &commandQueue,
                                         bool byAnyMeans) {
  DO_VALIDATION;
  if (match->GetBall()->Predict(200).GetDistance(player->GetPosition()) > ballDistanceOptimizeThreshold) return;
  if (match->GetBallRetainer() != 0) return;

  if (!teamHasBestPossession) {
    DO_VALIDATION;

    if (!byAnyMeans) {
      DO_VALIDATION;
      // if dot nears 1, it means opp is somewhat between ball and me
      if (CouldWinABallDuelLikeliness() < 0.2f) return;
    }

    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Interfere;
    command.useDesiredMovement = true;

    command.strictMovement = e_StrictMovement_True;
    if (byAnyMeans) {
      DO_VALIDATION;
      command.strictMovement = e_StrictMovement_False;
    }
    command.desiredDirection = inputDirection;
    command.desiredVelocityFloat = inputVelocityFloat;
    commandQueue.push_back(command);
  }
}

void PlayerController::_SlidingCommand(PlayerCommandQueue &commandQueue) {
  DO_VALIDATION;
  if (team->GetHumanGamerCount() != 0) return;
  if (match->GetBallRetainer() != 0) return;
  if (CouldWinABallDuelLikeliness() < 0.7f) return;

  if (!teamHasBestPossession && possessionAmount < 0.6f &&
      match->GetDesignatedPossessionPlayer() != player &&
      oppTeamHasPossession) {
    DO_VALIDATION;

    Vector3 ballPos = match->GetMentalImage(20)->GetBallPrediction(200);
    Vector3 playerPos = player->GetPosition() + player->GetMovement() * 0.2;
    Vector3 oppPos = _oppPlayer->GetPosition() + _oppPlayer->GetMovement() * 0.2;

    float ballDist = (playerPos - ballPos).GetLength();
    if ((ballDist > 0.7f && ballDist < 1.6f &&
         oppTimeNeededToGetToBall > 260) ||
        (ballDist > 0.6f && ballDist < 1.8f &&
         _oppPlayer->GetCurrentFunctionType() == e_FunctionType_Shot &&
         _oppPlayer->TouchPending())) {
      DO_VALIDATION;

      // no opp in the way?
      PlayerCommand command;
      command.desiredFunctionType = e_FunctionType_Sliding;
      command.useDesiredMovement = true;
      command.desiredDirection =
          ((ballPos.Get2D() + _oppPlayer->GetMovement() * 0.2f) - playerPos)
              .GetNormalized(
                  Vector3(-team->GetDynamicSide(), 0, 0));  // inputDirection;
      command.desiredVelocityFloat = sprintVelocity;
      command.useDesiredLookAt = true;
      command.desiredLookAt = CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.1f + command.desiredDirection * 10.0f;
      commandQueue.push_back(command);
    }
  }
}

void PlayerController::_MovementCommand(PlayerCommandQueue &commandQueue,
                                        bool forceMagnet, bool extraHaste) {
  DO_VALIDATION;
  auto _mentalImage = match->GetMentalImage(_mentalImageTime);
  int defaultLookAtTime_ms = 40;

  Vector3 quantizedInputDirection = inputDirection;
  if (quantizeDirection) QuantizeDirection(quantizedInputDirection, GetQuantizedDirectionBias());

  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Movement;
  command.useDesiredMovement = true;
  command.useDesiredLookAt = true;

  // defaults
  Vector3 manualDirection = quantizedInputDirection;
  float manualVelocityFloat = inputVelocityFloat;

  Vector3 defaultLookDirection = manualDirection;
  float desiredVeloFactor =
      1.0f - std::pow(NormalizedClamp(std::min(inputVelocityFloat,
                                               player->GetFloatVelocity()),
                                      idleDribbleSwitch, sprintVelocity),
                      0.5f) *
                 0.3f;
  Vector3 focusPos = match->GetMentalImage(_mentalImageTime)->GetBallPrediction(defaultLookAtTime_ms).Get2D();
  focusPos += player->GetDirectionVec() * 0.5f; // to keep looking forward if ball is very close
  radian toFocusAngle = (focusPos - player->GetPosition()).GetNormalized(manualDirection).GetAngle2D(manualDirection);
  defaultLookDirection = manualDirection.GetRotated2D(
      toFocusAngle * std::pow(desiredVeloFactor, 0.7f));

  command.desiredDirection = manualDirection;
  command.desiredVelocityFloat = manualVelocityFloat;

  Vector3 autoDirection = command.desiredDirection;
  float autoVelocityFloat = command.desiredVelocityFloat;
  Vector3 autoLookDirection = defaultLookDirection;

  float adaptedPossessionAmount = possessionAmount;

  CastPlayer()->SetDesiredTimeToBall_ms(0);

  float autoBias = 0.0;


  // decide what type of magnet is to be used

  float inputDirIsOwnHalfFactor = NormalizedClamp(
      inputDirection.GetDotProduct(Vector3(team->GetDynamicSide(), 0, 0)),
      -1.0f, 1.0f);

  if (match->GetBallRetainer() == player) {
    DO_VALIDATION;

    autoBias = 0.0f;

  } else if (forceMagnet || match->GetDesignatedPossessionPlayer() == player ||
             ((CastPlayer()->GetLastTouchBias(2000) > 0.01f &&
               possessionAmount > 0.5f) &&
              team->GetDesignatedTeamPossessionPlayer() == player) ||
             ((!oppTeamHasPossession && possessionAmount > 0.5f) &&
              team->GetDesignatedTeamPossessionPlayer() == player) ||
             (possessionAmount > 0.99f &&
              team->GetDesignatedTeamPossessionPlayer() ==
                  player) ||  // for air balls and such, where multiple players
                              // are as likely to get to the ball first
             (hasBestPossession &&
              team->GetDesignatedTeamPossessionPlayer() == player)) {
    DO_VALIDATION;  // this is constructed inefficiently (double teamplayer
                    // thing) on purpose, for clarity.

    // WE ARE THE MAN OF THE MOMENT! WOOHOO

    if (hasBestPossession) {
      DO_VALIDATION;

      Vector3 autoLookAt; // dud
      CastPlayer()->SetDesiredTimeToBall_ms(AI_GetBallControlMovement(
          match->GetMentalImage(_mentalImageTime), CastPlayer(), quantizedInputDirection,
          inputVelocityFloat, autoDirection, autoVelocityFloat, autoLookAt));
      autoLookDirection = (autoLookAt - player->GetPosition()).GetNormalized(0);
      autoBias = 1.0f;

    } else {  // !hasPossession

      float haste = 0.0f;
      if (extraHaste || forceMagnet) {
        DO_VALIDATION;
        haste = 1.0f;
      } else {
        float thresholdPossessionAmountForHaste = 1.1f; // 2.0f - AI_GetMindSet(CastPlayer()->GetFormationEntry().role) * 2.0f; // attacking players may want to gamble on the defenders missing the ball
        if (adaptedPossessionAmount < thresholdPossessionAmountForHaste) haste = 1.0f;
      }

      Vector3 autoLookAt; // dud
      CastPlayer()->SetDesiredTimeToBall_ms(AI_GetToBallMovement(
          match, match->GetMentalImage(_mentalImageTime), CastPlayer(), quantizedInputDirection,
          inputVelocityFloat, autoDirection, autoVelocityFloat, autoLookAt,
          haste));

      autoLookDirection = (autoLookAt - player->GetPosition()).GetNormalized(0);
      autoBias = 1.0f;

      if (match->GetDesignatedPossessionPlayer() != player) {
        DO_VALIDATION;
        float sameDirFactor = 1.0f - clamp(fabs(autoDirection.GetAngle2D(manualDirection)) / pi, 0.0f, 1.0f);
        sameDirFactor = sameDirFactor * 0.5f + 0.5f;
        autoBias = 0.0f;
        if (CastPlayer()->GetLastTouchBias(2000) > 0.01f) {
          DO_VALIDATION;
          if (CastPlayer()->GetTimeNeededToGetToBall_ms() < 1700)
            autoBias = std::pow(CastPlayer()->GetLastTouchBias(2000), 0.4f) *
                       std::pow(sameDirFactor, 0.5f);
        }
        if (!oppTeamHasPossession) {
          DO_VALIDATION;
          if (player->ExternalController()) {
            DO_VALIDATION;
            float magnetBias = std::max(curve(sameDirFactor, 0.8f), powf(GetLastSwitchBias(), 0.5f)); // needed for when we get passed a ball while we are not the designated player. we want to at least try to get there.
            magnetBias *= 1.0f - inputDirIsOwnHalfFactor; // if we run for our own half, don't accidentally magnet somewhere

            autoBias =
                clamp(std::pow((std::max(possessionAmount, 0.5f) - 0.5f) * 2.0f,
                               0.5f) *
                          magnetBias,
                      autoBias, 1.0f);
          }
        }
        if (forceMagnet) {
          DO_VALIDATION;
          autoBias = 1.0f;
        }
      }
    }

  } else if (match->GetDesignatedPossessionPlayer()->GetTeam() != team) {
    DO_VALIDATION;

    // OTHER TEAM IS IN BALL CONTROL, DEM BASTERDS

    if (team->GetDesignatedTeamPossessionPlayer() == player) {
      DO_VALIDATION;

      // WE ARE THE BEST OUR TEAM HAS GOT, CHOOSE OUR ACTIONS WISELY


      // if we aren't the designated possession player, we don't ever want to just run straight to the ball like that.
      // we want some combination of manual movement, defensive movement, and to-ball movement.

      // virtual action area in front of opponent
      PlayerImage oppImage = _mentalImage->GetPlayerImage(_oppPlayer);
      Vector3 oppPos = oppImage.position + oppImage.movement * 0.14f + oppImage.directionVec * 0.6f;
      Vector3 oppToGoalDirection =
          (Vector3(pitchHalfW * team->GetDynamicSide(), 0, 0) - oppPos)
              .GetNormalized(0);
      float actionRadius = 5.0f;
      Vector3 focusPosition = oppPos + (oppToGoalDirection * actionRadius * 0.7f);
      float actionBias = 1.0f - curve(NormalizedClamp((player->GetPosition() - focusPosition).GetLength(), 0.0f, actionRadius), 0.7f);

      // if we're close to the focus position, go do more defending or ballhuntin' (as opposed to manual movement)
      if (player->ExternalControllerActive())
        autoBias = actionBias * 0.0f + GetLastSwitchBias() * 0.3f;
      else
        autoBias = actionBias * 0.0f; // not sure what would be a proper value here. needs more testing

      autoBias *= 1.0f - inputDirIsOwnHalfFactor; // if we want to run back to more defensive positions, don't autobias

      // now decide on what auto direction to use, if applicable
      // when farther away from ball/opp, get towards ball/opp, when already close, mimic opponent's movement (sort of an auto 'man marking' thing)
      if (autoBias > 0.0f) {
        DO_VALIDATION;
        float playerOppDistance = (oppPos - player->GetPosition()).GetLength();
        float manMarkingBias = std::pow(
            1.0f - curve(NormalizedClamp(playerOppDistance, 0.0f, 7.0f), 0.7f),
            1.2f);

        Vector3 autoDirection_manMarking = oppImage.movement.GetNormalized(inputDirection);
        float autoVelocityFloat_manMarking = EnumToFloatVelocity(oppImage.velocity);
        Vector3 autoLookDirection_manMarking = (_mentalImage->GetBallPrediction(defaultLookAtTime_ms).Get2D() - player->GetPosition()).GetNormalized(0);

        Vector3 huntTarget = oppPos + oppToGoalDirection * playerOppDistance * 0.3f;
        Vector3 huntMovement = huntTarget - player->GetPosition();
        Vector3 autoDirection_hunt = huntMovement.GetNormalized(inputDirection);
        float autoVelocityFloat_hunt = clamp(huntMovement.GetLength() * distanceToVelocityMultiplier, idleVelocity, sprintVelocity);
        Vector3 autoLookDirection_hunt = (_mentalImage->GetBallPrediction(defaultLookAtTime_ms).Get2D() - player->GetPosition()).GetNormalized(0);

        autoDirection = ( autoDirection_manMarking * manMarkingBias + autoDirection_hunt * (1.0f - manMarkingBias) ).GetNormalized(inputDirection);
        autoVelocityFloat = clamp(autoVelocityFloat_manMarking * manMarkingBias + autoVelocityFloat_hunt * (1.0f - manMarkingBias), idleVelocity, sprintVelocity);
        autoLookDirection = ( (autoLookDirection_manMarking * manMarkingBias + autoLookDirection_hunt * (1.0f - manMarkingBias)) ).GetNormalized(0);
      }
    }
  }

  {
    Vector3 autoMovement = autoDirection * autoVelocityFloat;
    Vector3 manualMovement = manualDirection * manualVelocityFloat;
    Vector3 resultingMovement = manualMovement * (1.0 - autoBias) + autoMovement * autoBias;
    command.desiredDirection = resultingMovement.GetNormalized(quantizedInputDirection);
    command.desiredVelocityFloat = clamp(resultingMovement.GetLength(), idleVelocity, sprintVelocity);

    if (command.desiredVelocityFloat < idleDribbleSwitch) command.desiredDirection = autoLookDirection;

    Vector3 resultLookDirection = autoLookDirection;
    command.desiredLookAt = player->GetPosition() + resultLookDirection * 10.0f;
  }

  commandQueue.push_back(command);
}

void PlayerController::_CalculateSituation() {
  DO_VALIDATION;
  hasPossession = CastPlayer()->HasPossession();
  hasUniquePossession = CastPlayer()->HasUniquePossession();
  teamHasPossession = team->HasPossession();
  teamHasUniquePossession = team->HasUniquePossession();
  oppTeamHasPossession = match->GetTeam(abs(team->GetID() - 1))->HasPossession();
  oppTeamHasUniquePossession = match->GetTeam(abs(team->GetID() - 1))->HasUniquePossession();
  hasBestChanceOfPossession = team->GetDesignatedTeamPossessionPlayer() == player;
  teamHasBestPossession = match->GetBestPossessionTeam() == team;
  possessionAmount = (float)(match->GetTeam(abs(team->GetID() - 1))->GetTimeNeededToGetToBall_ms() + 200) / (float)(_timeNeeded_ms + 200);
  teamPossessionAmount = team->GetTeamPossessionAmount();
  fadingTeamPossessionAmount = team->GetFadingTeamPossessionAmount();

  // when ball is close, don't fade possession bias (so we can take immediate action)
  if (hasBestChanceOfPossession) {
    DO_VALIDATION;
    float distanceBias = std::pow(
        NormalizedClamp(
            (match->GetBall()->Predict(300).Get2D() - player->GetPosition())
                .GetLength(),
            2.0f, 14.0f),
        2.0f);
    fadingTeamPossessionAmount = fadingTeamPossessionAmount * distanceBias + teamPossessionAmount * (1.0f - distanceBias);
  }

  oppTimeNeededToGetToBall = match->GetTeam(abs(team->GetID() - 1))->GetTimeNeededToGetToBall_ms();
  hasBestPossession = hasPossession && possessionAmount >= 1.0f;
}
