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

#include "../../../main.hpp"

#include "elizacontroller.hpp"

#include <cmath>

#include "../../AIsupport/mentalimage.hpp"
#include "../../AIsupport/AIfunctions.hpp"

#include "../humanoid/humanoid_utils.hpp"

#include "strategies/strategy.hpp"

#include "strategies/offtheball/default_def.hpp"
#include "strategies/offtheball/default_mid.hpp"
#include "strategies/offtheball/default_off.hpp"
#include "strategies/offtheball/goalie_default.hpp"
#include "../playerofficial.hpp"

ElizaController::ElizaController(Match *match, bool lazyPlayer) : PlayerController(match), lazyPlayer(lazyPlayer) {
  lastDesiredDirection = Vector3(0);
  lastDesiredVelocity = 0;
}

ElizaController::~ElizaController() {
  if (defenseStrategy) delete defenseStrategy;
  if (midfieldStrategy) delete midfieldStrategy;
  if (offenseStrategy) delete offenseStrategy;
  if (goalieStrategy) delete goalieStrategy;
}

void ElizaController::RequestCommand(PlayerCommandQueue &commandQueue) {

  lastSwitchTimeDuration_ms = 0;
  lastSwitchTime_ms = 0;

  CastPlayer()->SetDesiredTimeToBall_ms(0);

  _CalculateSituation();

  _Preprocess(); // calculate some variables


  FormationEntry entry = CastPlayer()->GetDynamicFormationEntry();
  float mindSet = AI_GetMindSet(entry.role);


  // input

  Vector3 rawInputDirection = player->GetDirectionVec();
  float rawInputVelocityFloat = idleVelocity;
  Vector3 manualMovementDirection = player->GetDirectionVec();
  float manualMovementVelocityFloat = idleVelocity;

  bool manualMovement = false;
  bool extraHaste = false;


  // celebrate good times come on!

  if (!match->IsInPlay() && match->IsGoalScored()) {
    _AddCelebration(commandQueue);
    return;
  }


  // look at referee

  else if (!match->IsInPlay() && match->GetReferee()->GetBuffer().active == true &&
           (match->GetReferee()->GetCurrentFoulType() == 2 || match->GetReferee()->GetCurrentFoulType() == 3) &&
           match->GetReferee()->GetBuffer().stopTime < match->GetActualTime_ms() - 1000 &&
           match->GetReferee()->GetBuffer().prepareTime > match->GetActualTime_ms()) {

    // look at referee
    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Movement;
    command.useDesiredMovement = true;
    command.useDesiredLookAt = true;
    command.desiredDirection = CastPlayer()->GetDirectionVec();
    assert(command.desiredDirection.coords[2] == 0.0f);
    command.desiredVelocityFloat = idleVelocity;
    command.desiredLookAt = match->GetOfficials()->GetReferee()->GetPosition();
    commandQueue.push_back(command);

    return;
  }


  // stand still
  else if (!match->IsInPlay() || lazyPlayer) { // this whole if/then/else structure is ugly and unclear

    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Movement;
    command.useDesiredMovement = true;
    if (match->GetBallRetainer() == player) {
      command.desiredDirection = (Vector3(0) - player->GetPosition()).GetNormalized(player->GetDirectionVec());
    } else {
      command.desiredDirection = player->GetDirectionVec();
    }
    command.desiredVelocityFloat = idleVelocity;
    //if (!match->IsInSetPiece()) {
      command.desiredDirection = (player->GetDirectionVec() * 0.6f + (Vector3(0) - player->GetPosition()).GetNormalized(player->GetDirectionVec()) * 0.4f).GetNormalized(player->GetDirectionVec());
      command.desiredVelocityFloat = ClampVelocity(player->GetFloatVelocity() * 0.95f - random(0.0f, 3.2f));
    //}
    command.useDesiredLookAt = true;
    command.desiredLookAt = player->GetPosition() + (match->GetBall()->Predict(0).Get2D() - player->GetPosition()).GetNormalized(command.desiredDirection) * 10.0f;
    commandQueue.push_back(command);

    return;
  }


  // set piece taking

  else if ((match->IsInSetPiece() && team->GetController()->GetPieceTaker() == player) || match->GetBallRetainer() == player) {

    PlayerCommand actionCommand;

    if (team->GetController()->GetSetPieceType() == e_GameMode_Penalty) {

      actionCommand.desiredFunctionType = e_FunctionType_Shot;
      actionCommand.useDesiredMovement = false;
      actionCommand.useDesiredLookAt = false;
      actionCommand.touchInfo.desiredDirection = (Vector3(-team->GetSide() * pitchHalfW, random(-5, 5), 0) - CastPlayer()->GetPosition()).GetNormalized(Vector3(-team->GetSide(), 0, 0));
      actionCommand.touchInfo.desiredPower = random(0.4f, 1.0f);
      commandQueue.push_back(actionCommand);

    } else {

      actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
      actionCommand.useDesiredMovement = false;
      actionCommand.useDesiredLookAt = false;
      actionCommand.touchInfo.inputDirection = player->GetDirectionVec();
      actionCommand.touchInfo.inputPower = 0.5f;
      actionCommand.touchInfo.autoDirectionBias = 1.0f;
      actionCommand.touchInfo.autoPowerBias = 1.0f;

      actionCommand.touchInfo.forcedTargetPlayer = 0;

      Vector3 desiredTargetPosition;
      bool doCommand = true;

      if (team->GetController()->GetSetPieceType() == e_GameMode_GoalKick) {
        if (random(0.0f, 1.0f) > 0.4f && team->GetHumanGamerCount() == 0) {
          actionCommand.desiredFunctionType = e_FunctionType_HighPass;
          desiredTargetPosition = Vector3((pitchHalfW * -team->GetSide()) * 0.2f, random(-pitchHalfH, pitchHalfH), 0.0f);
        } else {
          actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
          desiredTargetPosition = Vector3(player->GetPosition().coords[0] * 0.9f, random(-pitchHalfH, pitchHalfH), 0.0f);
        }

      } else if (team->GetController()->GetSetPieceType() == e_GameMode_KickOff) {
        actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
        desiredTargetPosition = player->GetPosition() + player->GetDirectionVec() * 1.0f;

      } else if (team->GetController()->GetSetPieceType() == e_GameMode_FreeKick) {
        if (random(0.0f, 1.0f) > 0.5f) {
          actionCommand.desiredFunctionType = e_FunctionType_HighPass;
          desiredTargetPosition = Vector3(pitchHalfW * -team->GetSide(), random(-10.0f, 10.0f), 0.0f);
        } else {
          actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
          desiredTargetPosition = player->GetPosition() + Vector3(-team->GetSide() * 10.0f, random(-10.0f, 10.0f), 0.0f);
        }

      } else if (team->GetController()->GetSetPieceType() == e_GameMode_Corner) {
        if (random(0.0f, 1.0f) > 0.3f) {
          actionCommand.desiredFunctionType = e_FunctionType_HighPass;
          desiredTargetPosition = Vector3((pitchHalfW * -team->GetSide()) * (0.99f - random(0.0f, 0.12f)), random(-10.0f, 10.0f), 0.0f);
        } else {
          actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
          desiredTargetPosition = Vector3((pitchHalfW * -team->GetSide()) * 0.8f, player->GetPosition().coords[1] * 0.8f, 0.0f);
        }

      } else if (team->GetController()->GetSetPieceType() == e_GameMode_ThrowIn) {
        actionCommand.desiredFunctionType = e_FunctionType_ShortPass;
        desiredTargetPosition = player->GetPosition(); // closest to player

      } else if (match->GetBallRetainer() == player) { // keeper fetched ball, probably
        actionCommand.desiredFunctionType = e_FunctionType_HighPass;
        desiredTargetPosition = Vector3(pitchHalfW * team->GetSide(), random(-pitchHalfH, pitchHalfH), 0.0f);
        Player *targetPlayer = AI_GetClosestPlayer(team, desiredTargetPosition, false, CastPlayer());

        if (targetPlayer) {
          // check if this player is away from opponents, before throwing ball to him
          Player *closestOpp = AI_GetClosestPlayer(match->GetTeam(abs(team->GetID() - 1)), targetPlayer->GetPosition(), false, 0);
          if (closestOpp) {
            if (((closestOpp->GetPosition() + closestOpp->GetMovement() * 0.1f) - targetPlayer->GetPosition()).GetLength() > 10.0f || CastPlayer()->GetPossessionDuration_ms() > 4000) { // last touch bias is maximum so he won't hold forever
              actionCommand.touchInfo.forcedTargetPlayer = targetPlayer;
            } else {
              doCommand = false;
            }
          }
        }
      }

      if (doCommand) {
        if (actionCommand.touchInfo.forcedTargetPlayer == 0) actionCommand.touchInfo.forcedTargetPlayer = AI_GetClosestPlayer(team, desiredTargetPosition, false, CastPlayer());
        AI_GetPass(CastPlayer(), actionCommand.desiredFunctionType, actionCommand.touchInfo.inputDirection, actionCommand.touchInfo.inputPower, actionCommand.touchInfo.autoDirectionBias, actionCommand.touchInfo.autoPowerBias, actionCommand.touchInfo.desiredDirection, actionCommand.touchInfo.desiredPower, actionCommand.touchInfo.targetPlayer, actionCommand.touchInfo.forcedTargetPlayer);
        commandQueue.push_back(actionCommand);
      }
    }

    // remember, we are still in the 'set piece / ballretainer' if{}

    if (match->GetBallRetainer() != player) { // must be set piece taker, then
      PlayerCommand command;
      command.desiredFunctionType = e_FunctionType_Movement;
      command.useDesiredMovement = true;
      command.useDesiredLookAt = true;

      AI_GetBallControlMovement(_mentalImage, CastPlayer(), player->GetDirectionVec(), walkVelocity, command.desiredDirection, command.desiredVelocityFloat, command.desiredLookAt);
      assert(command.desiredDirection.coords[2] == 0.0f);

      commandQueue.push_back(command);
      return;
    } else { // must be ball retainer
      PlayerCommand command;
      command.desiredFunctionType = e_FunctionType_Movement;
      command.useDesiredMovement = true;
      command.desiredDirection = (actionCommand.touchInfo.desiredDirection.GetLength() > 0.0f) ? actionCommand.touchInfo.desiredDirection.Get2D().GetNormalized(player->GetDirectionVec()) : (Vector3(0) - player->GetPosition()).GetNormalized(player->GetDirectionVec());
      command.desiredVelocityFloat = idleVelocity;
      command.useDesiredLookAt = true;
      command.desiredLookAt = player->GetPosition() + command.desiredDirection * 10.0f;
      commandQueue.push_back(command);
      if (team->GetController()->GetSetPieceType() == e_GameMode_ThrowIn) {
        //printf("elizacontroller throw in doCommand (green pilon == target)\n");
        //SetGreenDebugPilon(command.desiredLookAt);
      }
      return;
    }

  }


  // default strategies for defensively, midfielders and offensively positioned players

  else if (match->IsInPlay() && !match->IsInSetPiece() && match->GetBallRetainer() != player && match->GetDesignatedPossessionPlayer() != player && CastPlayer()->GetFormationEntry().role != e_PlayerRole_GK) { //(match->GetDesignatedPossessionPlayer() != player || (CastPlayer()->GetFormationEntry().role == e_PlayerRole_GK && !CastPlayer()->HasPossession()))) {
    if      (CastPlayer()->GetFormationEntry().role == e_PlayerRole_LB ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_CB ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_RB) {
      defenseStrategy->RequestInput(_mentalImage, rawInputDirection, rawInputVelocityFloat);
    }
    else if (CastPlayer()->GetFormationEntry().role == e_PlayerRole_DM ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_LM ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_CM ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_RM ||
             CastPlayer()->GetFormationEntry().role == e_PlayerRole_AM) {
      midfieldStrategy->RequestInput(_mentalImage, rawInputDirection, rawInputVelocityFloat);
    }
    else if (CastPlayer()->GetFormationEntry().role == e_PlayerRole_CF) {
      offenseStrategy->RequestInput(_mentalImage, rawInputDirection, rawInputVelocityFloat);
    }

  }

  // dribble, pass, etcetera
  else if (match->IsInPlay() && !match->IsInSetPiece() && match->GetDesignatedPossessionPlayer() == player && team->GetHumanGamerCount() == 0 && CastPlayer()->GetFormationEntry().role != e_PlayerRole_GK) {
    if (CastPlayer()->GetTimeNeededToGetToBall_ms() < 1000) {
      GetOnTheBallCommands(commandQueue, rawInputDirection, rawInputVelocityFloat);
    }
    extraHaste = false;
  }

  else if (match->IsInPlay() && !match->IsInSetPiece() && CastPlayer()->GetFormationEntry().role == e_PlayerRole_GK) {
    // keeper's mental image for deflections is near-instant; let's just call it premonition ;)
    const MentalImage *keeperMentalImage = _mentalImage;
    static_cast<GoalieDefaultStrategy*>(goalieStrategy)->CalculateIfBallIsBoundForGoal(keeperMentalImage);
    bool boundForGoal = static_cast<GoalieDefaultStrategy*>(goalieStrategy)->IsBallBoundForGoal();
    if (boundForGoal) manualMovement = true;
    if (CastPlayer() != match->GetDesignatedPossessionPlayer()) manualMovement = true;
    goalieStrategy->RequestInput(keeperMentalImage, manualMovementDirection, manualMovementVelocityFloat);

    //if (boundForGoal && hasUniquePossession) printf("has unique possession, so no deflect anims (poss: %f)\n", possessionAmount);
    if (!hasUniquePossession || possessionAmount < 3.4f) {
      bool onlyPickupAnims = false;
      if (!boundForGoal && possessionAmount > 1.3f) onlyPickupAnims = true;
      _KeeperDeflectCommand(commandQueue, onlyPickupAnims);
    }

    if (CastPlayer()->GetTimeNeededToGetToBall_ms() < 1000 && match->GetDesignatedPossessionPlayer() == player) {
      GetOnTheBallCommands(commandQueue, rawInputDirection, rawInputVelocityFloat);
    }
  }

  bool forceMagnet = false;

  // team pressure
  float ballDistance = _mentalImage->GetBallPrediction(0).Get2D().GetDistance(player->GetPosition());
  if (match->IsInPlay() && !match->IsInSetPiece() &&
      ((team->GetController()->GetEndApplyTeamPressure_ms() > match->GetActualTime_ms() && team->GetController()->GetTeamPressurePlayer() == player) /*|| (player == team->GetDesignatedTeamPossessionPlayer() && ballDistance < 8.0f && CastPlayer()->GetManMarkingID() == -1) */) &&
      !teamHasBestPossession && match->GetDesignatedPossessionPlayer() != player && CastPlayer()->GetFormationEntry().role != e_PlayerRole_GK) {
    forceMagnet = true;
  }

  _SetInput(rawInputDirection, rawInputVelocityFloat);
  if (rawInputDirection.GetLength() != 0) lastDesiredDirection = rawInputDirection; else
                                          lastDesiredDirection = player->GetDirectionVec();
  lastDesiredVelocity = rawInputVelocityFloat;

  if (match->IsInPlay() && !match->IsInSetPiece()) {

    // ball control?
    _BallControlCommand(commandQueue, false, false, false); // last param true == enable sticky run direction.

    // trap?
    _TrapCommand(commandQueue);

    // interfere?
    bool byAnyMeans = false;
    _InterfereCommand(commandQueue, byAnyMeans);

    // sliding?
    _SlidingCommand(commandQueue);
  }

  // movement
  if (!manualMovement && match->IsInPlay() && !match->IsInSetPiece()) {

    Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();

    if (CastPlayer() != match->GetDesignatedPossessionPlayer()) {

      float mindSet = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
      float huntDistanceThreshold = 10.0f + (1.0f - mindSet) * 10.0f; // 10 + .. * 10
      huntDistanceThreshold *= 0.5f * CastPlayer()->GetFatigueFactorInv() +
                               0.5f * (1.0f - NormalizedClamp(CastPlayer()->GetAverageVelocity(10), idleVelocity, sprintVelocity));
      huntDistanceThreshold *= 0.3f + GetMatch()->GetMatchDifficulty() * 0.7f;

      if (forceMagnet) {

        // make sure the movement command magnets get the best input (which is then used as 'hint' for the 'toball' functions)

        inputDirection = Vector3(team->GetSide() * pitchHalfW, 0, 0) - CastPlayer()->GetPosition();
        inputVelocityFloat = idleVelocity;
        inputDirection.Normalize(CastPlayer()->GetDirectionVec());

      } else if (!teamHasBestPossession && CastPlayer()->GetManMarkingID() == -1 && ((opp->GetPosition() + opp->GetMovement() * 0.12f) - (CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.04f)).GetLength() < huntDistanceThreshold) { // defend player

          if (player == team->GetDesignatedTeamPossessionPlayer() && possessionAmount > 0.8f) {
            forceMagnet = true; // don't give up battles too easily
            extraHaste = true;
          }

          // or, defend an opponent, if we don't have anything better to do anyway

          Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();

          int huntingPlayersNum = 2; // remember, this includes players with man marking id that are not controlled by this hunting code
          std::vector<Player*> closestPlayers;
          AI_GetClosestPlayers(team, opp->GetPosition() + opp->GetMovement() * 0.1f, false, closestPlayers, huntingPlayersNum);
          bool close = false;
          for (unsigned int i = 0; i < closestPlayers.size(); i++) {
            if (closestPlayers[i] == player) {
              close = true;
              break;
            }
          }

          if (close) {

            /*
            // 'easy off' method
            Vector3 defendPosition_easy = CastPlayer()->GetPosition();
            AddDefensiveComponent(defendPosition_easy, 1.0f, opp->GetID());
            inputDirection = (defendPosition - CastPlayer()->GetPosition()).GetNormalized(inputDirection);
            inputVelocityFloat = clamp((defendPosition - CastPlayer()->GetPosition()).GetLength() * distanceToVelocityMultiplier, 0.0f, sprintVelocity);
            //forceMagnet = true; // more aggressive!
            */

            // more 'hunting' method
            Vector3 defendPosition = GetDefendPosition(opp);
            if (NeedDefendingMovement(team->GetSide(), player->GetPosition(), defendPosition)) {
              inputDirection = (defendPosition - CastPlayer()->GetPosition()).GetNormalized(inputDirection);
              inputVelocityFloat = clamp((defendPosition - CastPlayer()->GetPosition()).GetLength() * distanceToVelocityMultiplier, 0.0f, sprintVelocity);
              forceMagnet = true; // more aggressive!
            }

          } // close

      } // else (!forceMagnet)

    } // !designated

    inputVelocityFloat = RangeVelocity(inputVelocityFloat); // emulate controller quantization

    _MovementCommand(commandQueue, forceMagnet, extraHaste);

  } else { // keeper?

    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Movement;
    command.useDesiredMovement = true;
    command.desiredDirection = manualMovementDirection;
    assert(command.desiredDirection.coords[2] == 0.0f);
    command.desiredVelocityFloat = manualMovementVelocityFloat;
    command.useDesiredLookAt = true;
    command.desiredLookAt = player->GetPosition() + (_mentalImage->GetBallPrediction(40).Get2D() - player->GetPosition()).GetNormalized(0) * 10.0f;
    commandQueue.push_back(command);

  }
}

void ElizaController::Process() {
  PlayerController::Process();
}

Vector3 ElizaController::GetDirection() {
  return lastDesiredDirection;
}

float ElizaController::GetFloatVelocity() {
  return lastDesiredVelocity;
}

void ElizaController::LoadStrategies() {
  defenseStrategy = new DefaultDefenseStrategy(this);
  midfieldStrategy = new DefaultMidfieldStrategy(this);
  offenseStrategy = new DefaultOffenseStrategy(this);
  goalieStrategy = new GoalieDefaultStrategy(this);
}

float ElizaController::GetLazyVelocity(float desiredVelocityFloat) {

  // input is unclamped! use large values (less lazy, apparently we are more off-position)
  float adaptedDesiredVelocityFloat = desiredVelocityFloat;
  // don't use full overflow though
  if (adaptedDesiredVelocityFloat > sprintVelocity) adaptedDesiredVelocityFloat = sprintVelocity + (adaptedDesiredVelocityFloat - sprintVelocity) * 0.1f;

  float startLazinessDistance = 20.0f * (CastPlayer()->GetFatigueFactorInv() * 0.8f + 0.2f);
  float endLazinessDistance = 65.0f * (CastPlayer()->GetFatigueFactorInv() * 0.5f + 0.5f);

  Vector3 oppPos = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer()->GetPosition();
  float actionDistance = (player->GetPosition() - oppPos).GetLength();
  float teamPossession = clamp(GetFadingTeamPossessionAmount() - 0.5f, 0.0f, 1.0f);
  float mindSet = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);

  // if mindSet is high (offensive), be lazy when teamPossession is low. and the other way round for defenders.
  // (midfielders are always half-lazy by role this way; pow() them a bit if this is undesired; this will of course wear them out more than backs and strikers)
  float lazinessByRole = mindSet + teamPossession * (1.0f - mindSet * 2.0f);
  float lazinessByPosition = NormalizedClamp(actionDistance, startLazinessDistance, endLazinessDistance);

  float lazyFactor = lazinessByPosition * (0.5f + lazinessByRole * 0.5f);
  float resultingVelocityFloat = adaptedDesiredVelocityFloat * (1.0f - lazyFactor);

  bool clampToDribble = false;
  if (desiredVelocityFloat >= dribbleVelocity) clampToDribble = true; // no standing still when initially undesired; it would get us further and further away from our desired position *edit: no longer true with the unclamped input
  if (clampToDribble && resultingVelocityFloat < dribbleVelocity) resultingVelocityFloat = dribbleVelocity;

  // short term fatigue/work rate shortage ;)
  // does not heed dribble clamp above, as to simulate players having to stop to catch their breath
  float breathLeftFactor = 1.0f - NormalizedClamp(CastPlayer()->GetAverageVelocity(10), idleVelocity, sprintVelocity);
  float workRate = CastPlayer()->GetStat(mental_workrate);
  breathLeftFactor = std::pow(breathLeftFactor, 0.8f - workRate * 0.2f);
  breathLeftFactor = clamp(breathLeftFactor * 1.2f, 0.0f, 1.0f); // make sure beginning of sprint is full speed
  breathLeftFactor = breathLeftFactor * lazyFactor + 1.0f * (1.0f - lazyFactor); // sometimes, we really need to force it
  resultingVelocityFloat = std::min(resultingVelocityFloat, sprintVelocity * breathLeftFactor);

  return resultingVelocityFloat;
}

Vector3 ElizaController::GetSupportPosition_ForceField(const MentalImage *mentalImage, const Vector3 &basePosition, bool makeRun) {

  Player *designatedPlayer = team->GetDesignatedTeamPossessionPlayer();

  Vector3 currentPos = player->GetPosition() + CastPlayer()->GetMovement() * 0.1f; //basePosition;
  Vector3 mainManPos = designatedPlayer->GetPosition() + designatedPlayer->GetMovement() * 0.1f;

  std::vector<ForceSpot> forceField;


  // support position

  float dynamicMindSet = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
  float ballDistance = (match->GetBall()->Predict(250).Get2D() - player->GetPosition()).GetLength();
  float ballDistanceX = fabs(match->GetBall()->Predict(250).coords[0] - player->GetPosition().coords[0]);

  bool forceNoOffside = true;

  float basePositionWeight = 0.7f;
  float overallWeight = 1.0f;
  float opponentRepelWeight = 0.3f * overallWeight;
  float teammateRepelWeight = 0.4f * overallWeight;
  float ballRepelWeight = 1.0f * overallWeight;
  float runWeight = 1.0f * overallWeight;
  float flockToPossessionPlayerWeight = 0.45f * overallWeight;

  float webScale = 0.75f;

  switch (CastPlayer()->GetDynamicFormationEntry().role) {
    case e_PlayerRole_CB:
    case e_PlayerRole_LB:
    case e_PlayerRole_RB:
      opponentRepelWeight *= 2.2f;
      break;
    case e_PlayerRole_DM:
      opponentRepelWeight *= 2.0f;
      break;
    case e_PlayerRole_CM:
    case e_PlayerRole_LM:
    case e_PlayerRole_RM:
      opponentRepelWeight *= 1.6f;
      break;
    case e_PlayerRole_AM:
      opponentRepelWeight *= 1.2f;
      break;
    case e_PlayerRole_CF:
      opponentRepelWeight *= 1.0f;
      break;
    default:
      break;
  }

  float offsideX = AI_GetOffsideLine(match, _mentalImage, abs(team->GetID() - 1), 240);
  float adaptedMakeRun = makeRun;

  // actual base position
  {
    ForceSpot spot;
    spot.origin = basePosition;

    // let left and right flank flow forwards and backwards every period_sec seconds, for some dynamics
    // except for one close player, who should always run forward for support
    Player *forwardSupportPlayer = team->GetController()->GetForwardSupportPlayer();
    if (player == forwardSupportPlayer) {
      spot.origin.coords[0] += -team->GetSide() * (0.3f + 0.7f * dynamicMindSet) * 12.0f;
    } else {
      /* sine version
      float amount = 8.0f;//14.0f;//8.0f;
      float period_sec = 7.0f;
      float phase_offset = NormalizedClamp(currentPos.coords[1], -pitchHalfH, pitchHalfH);
      amount *= sin(match->GetActualTime_ms() * 0.001f * ((1.0f / period_sec) * 2.0f * pi) + (phase_offset * 2.0f * pi)) * 0.5f + 0.5f;
      float delta = -team->GetSide() * curve(dynamicMindSet, 0.5f) * (1.0f - pow(NormalizedClamp(ballDistanceX, 1.0f, 30.0f), 1.5f)) * amount; // offensive crew
      spot.origin.coords[0] += delta;
      */
      // lane version
      float amount = 22.0f;
      float laneY = -signSide(mainManPos.coords[1]) * 8.0f;
      amount *= curve(1.0f - NormalizedClamp(fabs(laneY - currentPos.coords[1]), 0.0f, 30.0f), 1.0f);
      float delta = -team->GetSide() * std::pow(dynamicMindSet, 1.5f) * amount;
      spot.origin.coords[0] += delta;
    }

    spot.magnetType = e_MagnetType_Attract;
    spot.decayType = e_DecayType_Constant;
    spot.power = 1.0f * basePositionWeight;
    // the farther away from this position, the more we are attracted to it
    spot.power *= 0.3f + 0.7f * NormalizedClamp((spot.origin - currentPos).GetLength(), 0.0f, 20.0f);
    forceField.push_back(spot);
  }

  if (adaptedMakeRun) {
    ForceSpot spot;
    spot.origin = Vector3(-team->GetSide() * pitchHalfW, currentPos.coords[1] * 0.5f, 0.0f);//player->GetDirectionVec() * 100.0f;// / distanceToVelocityMultiplier;
    spot.magnetType = e_MagnetType_Attract;
    spot.decayType = e_DecayType_Constant;
    spot.power = 2.0f * runWeight;
    forceField.push_back(spot);
  }

  // stay away from opponents
  std::vector<Player*> opponents;
  AI_GetClosestPlayers(match->GetTeam(abs(team->GetID() - 1)), mainManPos * 0.3f + currentPos + 0.7f, false, opponents, 3);
  for (unsigned int i = 0; i < opponents.size(); i++) {
    const PlayerImage &oppImg = mentalImage->GetPlayerImage(opponents[i]->GetID());
    ForceSpot spot;
    Vector3 oppPos = oppImg.position + oppImg.movement * 0.1f;
    spot.origin = oppPos + (oppPos - mainManPos).GetNormalized(0) * 2.0f; // anti-magnet behind opponent, because the pass-way must be cleared
    spot.magnetType = e_MagnetType_Repel;
    spot.decayType = e_DecayType_Variable;
    spot.power = 1.0f * opponentRepelWeight;
    spot.scale = 5.0f;
    if (adaptedMakeRun) {
      spot.scale = 2.0f;
      spot.power *= 0.5f;
    }
    spot.exp = 0.7f;
    forceField.push_back(spot);
  }

  // stay away from teammates
  if (team->GetFadingTeamPossessionAmount() >= 1.02f) {
    std::vector<Player*> players;
    AI_GetClosestPlayers(team, currentPos, false, players, 6);
    for (unsigned int i = 0; i < players.size(); i++) {
      if (players[i] != CastPlayer()) {
        const PlayerImage &mateImg = mentalImage->GetPlayerImage(players[i]->GetID());
        ForceSpot spot;
        spot.origin = mateImg.position + mateImg.movement * 0.1f;
        spot.magnetType = e_MagnetType_Repel;
        spot.decayType = e_DecayType_Variable;
        spot.power = 1.0f * teammateRepelWeight;
        spot.scale = 14.0f * webScale;
        spot.exp = 1.0f;
        forceField.push_back(spot);
      }
    }
  }

  // stay away from ball (to not get in the way of passes and possessionplayer)
  if (CastPlayer() != designatedPlayer && team->GetFadingTeamPossessionAmount() >= 1.06f) {
    ForceSpot spot;
    spot.magnetType = e_MagnetType_Repel;
    spot.decayType = e_DecayType_Variable;
    spot.power = 1.0f * ballRepelWeight;
    spot.scale = 2.0f;
    spot.exp = 0.5f;
    spot.origin = mentalImage->GetBallPrediction(200).Get2D();
    forceField.push_back(spot);
    spot.origin = mentalImage->GetBallPrediction(350).Get2D();
    forceField.push_back(spot);
    spot.origin = mentalImage->GetBallPrediction(500).Get2D();
    forceField.push_back(spot);
    spot.origin = mentalImage->GetBallPrediction(650).Get2D();
    forceField.push_back(spot);
  }

  if (CastPlayer() != designatedPlayer) {

    // attract to teammate in possession
    {
      ForceSpot spot;
      spot.origin = mainManPos;
      spot.magnetType = e_MagnetType_Attract;
      spot.decayType = e_DecayType_Variable;
      spot.power = 1.0f * flockToPossessionPlayerWeight;
      spot.scale = 28.0f * webScale;
      spot.exp = 1.0f;
      forceField.push_back(spot);
    }

    // ..yet not too close
    {
      ForceSpot spot;
      spot.origin = mainManPos;
      spot.magnetType = e_MagnetType_Repel;
      spot.decayType = e_DecayType_Variable;
      spot.power = 1.0f * flockToPossessionPlayerWeight;
      spot.scale = 16.0f * webScale;
      spot.exp = 1.0f;
      forceField.push_back(spot);
    }
  }

  Vector3 forceFieldPosition = currentPos + AI_GetForceFieldMovement(forceField, currentPos, 7);//8);

  float margin = 0.08f;
  if (forceNoOffside) if (forceFieldPosition.coords[0] * -team->GetSide() > (offsideX * -team->GetSide()) - margin) forceFieldPosition.coords[0] = offsideX - (margin * -team->GetSide());

  forceFieldPosition.coords[0] = clamp(forceFieldPosition.coords[0], -pitchHalfW, pitchHalfW);
  forceFieldPosition.coords[1] = clamp(forceFieldPosition.coords[1], -pitchHalfH, pitchHalfH);

  return forceFieldPosition;
}

void ElizaController::Reset() {
  lastDesiredDirection = Vector3(0);
  lastDesiredVelocity = 0;
}

void ElizaController::GetOnTheBallCommands(std::vector<PlayerCommand> &commandQueue, Vector3 &rawInputDirection, float &rawInputVelocityFloat) {

  float oneTouchIsHard = 0.0f;
  float movementDiff = NormalizedClamp((match->GetBall()->GetMovement() - CastPlayer()->GetMovement()).GetLength(), 0.0f, 10.0f);
  oneTouchIsHard = movementDiff - CastPlayer()->GetStat(technical_shortpass) * movementDiff * 0.8f;

  auto opponentPlayerImages = _mentalImage->GetTeamPlayerImages(abs(team->GetID() - 1));


  // DECIDE WHAT TO DO

  float longPossessionFactor = pow(NormalizedClamp(CastPlayer()->GetPossessionDuration_ms(), 0, 5000), 2.0f);

  // first selection
  float forwardSpaceWeight = 0.4f;
  float spaceWeight = 0.3f;
  float forwardWeight = 2.0f + AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role) * 6.0f;

  float totalWeight1 = forwardSpaceWeight + spaceWeight + forwardWeight;
  float tacticalImprovementThreshold = 0.06f * (1.0f - AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role)); // only go on with pass selection if recipient has this much tactical advantage over current player

  // second selection
  float tacticalDiffWeight =
      1.0f +
      std::pow(AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role),
               2.0f) *
          10.0f;
  float passWeight = 1.0f;
  float passMinimum = 0.2f * (1.0f - AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role)) - longPossessionFactor * 0.1f;

  float totalWeight2 = tacticalDiffWeight + passWeight;

  // name says it all
  float passThreshold = 0.1f - longPossessionFactor * 0.05f;

  // self rating
  const TacticalPlayerSituation &sit = CastPlayer()->GetTacticalSituation();
  float tacticalRating = sit.forwardSpaceRating * forwardSpaceWeight +
                         sit.spaceRating * spaceWeight +
                         sit.forwardRating * forwardWeight;

  tacticalRating /= totalWeight1;

  // collect pass target candidates
  std::vector<Player*> mates;
  team->GetActivePlayers(mates);

  struct MateRating {
    Player *player;
    float tacticalRating = 0.0f;
    float tacticalDiffRating = 0.0f;
    float passRating = 0.0f;
    float proximityRating = 0.0f;
    e_FunctionType passType;
  };

  float bestTotalRating = 0.0f;
  MateRating bestMateRating;
  TacticalPlayerSituation bestMateSit;
  bestMateRating.player = 0;
  bestMateRating.passRating = 0.0f;
  bestMateRating.passType = e_FunctionType_ShortPass;
  for (unsigned int i = 0; i < mates.size(); i++) {

    if (mates[i] != CastPlayer()) {

      const TacticalPlayerSituation &mateSit = mates[i]->GetTacticalSituation();

      float mateTacticalRating = mateSit.forwardSpaceRating * forwardSpaceWeight +
                                 mateSit.spaceRating * spaceWeight +
                                 mateSit.forwardRating * forwardWeight;

      mateTacticalRating /= totalWeight1;
      if (mates[i]->GetFormationEntry().role == e_PlayerRole_GK) mateTacticalRating *= 0.7f; // don't like playing back to goalie

      MateRating mateRating;
      mateRating.player = mates[i];
      mateRating.tacticalRating = mateTacticalRating;

      if (mateTacticalRating > tacticalRating + tacticalImprovementThreshold) {

        float tacticalDiffRating = mateRating.tacticalRating - tacticalRating;

        mateRating.tacticalDiffRating = tacticalDiffRating;
        float passingOddsShort = _GetPassingOdds(mates[i], e_FunctionType_ShortPass, opponentPlayerImages);
        float passingOddsLong  = _GetPassingOdds(mates[i], e_FunctionType_LongPass,  opponentPlayerImages);
        float passingOddsHigh  = _GetPassingOdds(mates[i], e_FunctionType_HighPass,  opponentPlayerImages);
        if (passingOddsShort >= passingOddsLong && passingOddsShort >= passingOddsHigh) {
          mateRating.passRating = passingOddsShort;
          mateRating.passType = e_FunctionType_ShortPass;
        } else if (passingOddsLong >= passingOddsHigh) {
          mateRating.passRating = passingOddsLong;
          mateRating.passType = e_FunctionType_LongPass;
        } else {
          mateRating.passRating = passingOddsHigh;
          mateRating.passType = e_FunctionType_HighPass;
        }

        float totalRating = mateRating.tacticalDiffRating * tacticalDiffWeight +
                            mateRating.passRating * passWeight -
                            oneTouchIsHard;

        totalRating /= totalWeight2;

        if (totalRating > bestTotalRating && totalRating > passThreshold && mateRating.passRating > passMinimum) {
          bestTotalRating = totalRating;
          bestMateRating = mateRating;
          bestMateSit = mateSit;
        }
      }

    } // !self
  }

  // panic
  float mindSet = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
  if (mindSet < 0.25f) {
    float panicProneness = 1.0f - mindSet * 2.0f;
    float goalCloseness = 1.0f - NormalizedClamp((CastPlayer()->GetPosition() - Vector3(pitchHalfW * CastPlayer()->GetTeam()->GetSide(), 0, 0)).GetLength(), 2.0f, 16.0f);//8.0f, 32.0f);
    if (CastPlayer()->GetDynamicFormationEntry().role != e_PlayerRole_GK) {
      if ((bestMateRating.player == 0 || bestMateRating.passRating < panicProneness * goalCloseness) && possessionAmount < 0.9f + panicProneness * goalCloseness * 0.8f) {
        _AddPanicPass(commandQueue);
      }
    } else { // keeper
      if (possessionAmount < 3.0f) {
        _AddPanicPass(commandQueue);
      }
    }
  }

  if (bestMateRating.player != 0) {
    _AddPass(commandQueue, bestMateRating.player, bestMateRating.passType);
  }

  // shoot?
  float goalDist = NormalizedClamp((Vector3(pitchHalfW * -team->GetSide(), 0, 0) - player->GetPosition()).GetLength(), 0.0f, 32.0f);
  float idealShotPosFactor = 1.0f - NormalizedClamp((Vector3((pitchHalfW - 7.0f) * -team->GetSide(), 0, 0) - player->GetPosition()).GetLength(), 0.0f, 16.0f);
  idealShotPosFactor = curve(idealShotPosFactor, 1.0f);
  if (idealShotPosFactor > 0.1f) {
    float odds1 = _GetPassingOdds(Vector3((pitchHalfW + 1.0f) * -team->GetSide(), -3.6f, 0), e_FunctionType_Shot, opponentPlayerImages, 3.0f);
    float odds2 = _GetPassingOdds(Vector3((pitchHalfW + 1.0f) * -team->GetSide(),  0.0f, 0), e_FunctionType_Shot, opponentPlayerImages, 3.0f);
    float odds3 = _GetPassingOdds(Vector3((pitchHalfW + 1.0f) * -team->GetSide(),  3.6f, 0), e_FunctionType_Shot, opponentPlayerImages, 3.0f);
    float odds = odds2; float y = 0.0f;
    if (odds1 > odds) { odds = odds1; y = -3.5f; }
    if (odds3 > odds) { odds = odds3; y =  3.5f; }

    odds = std::pow(odds, 0.5f);

    if (odds + random(0.0f, 0.5f) > 0.5f) {
      PlayerCommand command;
      command.desiredFunctionType = e_FunctionType_Shot;
      command.useDesiredMovement = false;
      command.useDesiredLookAt = false;
      command.desiredVelocityFloat = rawInputVelocityFloat; // this is so we can use sprint/dribble buttons as shot modifiers
      command.touchInfo.desiredDirection = (Vector3((pitchHalfW + 1.0f) * -team->GetSide(), y + random(-1.0f + player->GetStat(technical_shot), 1.0f - player->GetStat(technical_shot)), 0) - (CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.2f)).GetNormalized(Vector3(-team->GetSide(), 0, 0));
      command.touchInfo.desiredDirection = (command.touchInfo.desiredDirection * 0.7f + -CastPlayer()->GetDirectionVec() * (CastPlayer()->GetFloatVelocity() / sprintVelocity) * 0.3f).GetNormalized();
      command.touchInfo.autoDirectionBias = 1.0f;
      command.touchInfo.desiredPower = random(0.7f * (0.6f + goalDist * 0.4f), 1.0f * (0.6f + goalDist * 0.4f));
      commandQueue.push_back(command);
    }
  }


  e_Velocity enumVelocity = e_Velocity_Idle;
  AI_GetBestDribbleMovement(match, player->GetID(), _mentalImage, rawInputDirection, rawInputVelocityFloat, team->GetTeamData()->GetTactics());
}

void ElizaController::_AddPass(std::vector<PlayerCommand> &commandQueue, Player *target, e_FunctionType passType) {
  PlayerCommand command;
  command.desiredFunctionType = passType;
  command.useDesiredMovement = false;
  command.useDesiredLookAt = false;
  command.touchInfo.targetPlayer = 0;
  command.touchInfo.forcedTargetPlayer = target;
  command.touchInfo.inputDirection = Vector3(0);
  command.touchInfo.inputPower = 0;
  command.touchInfo.autoDirectionBias = 1.0f;
  command.touchInfo.autoPowerBias = 1.0f;
  AI_GetPass(CastPlayer(), passType, command.touchInfo.inputDirection, command.touchInfo.inputPower, command.touchInfo.autoDirectionBias, command.touchInfo.autoPowerBias, command.touchInfo.desiredDirection, command.touchInfo.desiredPower, command.touchInfo.targetPlayer, command.touchInfo.forcedTargetPlayer);
  commandQueue.push_back(command);
}

void ElizaController::_AddPanicPass(std::vector<PlayerCommand> &commandQueue) {

  int yside = signSide(player->GetDirectionVec().coords[1]); // > 0 ? 1 : -1;
  Vector3 sensibleAwayDir = ((player->GetDirectionVec() * Vector3(0.8f, 1.0f, 0.0f)).GetNormalized() + Vector3(-team->GetSide() * 0.7f, yside * 0.5f, 0)).GetNormalized(0) + Vector3(0, 0, 0.3f);
  sensibleAwayDir.Normalize(player->GetDirectionVec());

  PlayerCommand command;
  command.useDesiredMovement = false;
  command.useDesiredLookAt = false;

  command.touchInfo.inputDirection = sensibleAwayDir;
  command.touchInfo.desiredDirection = sensibleAwayDir;
  command.touchInfo.autoDirectionBias = 0.0f;
  command.touchInfo.autoPowerBias = 0.0f;

  command.desiredFunctionType = e_FunctionType_HighPass;
  command.touchInfo.inputPower = 0.7f;
  command.touchInfo.desiredPower = 0.7f;
  commandQueue.push_back(command);

  command.desiredFunctionType = e_FunctionType_Shot;
  command.touchInfo.inputPower = 0.6f;
  command.touchInfo.desiredPower = 0.6f;
  commandQueue.push_back(command);

  command.desiredFunctionType = e_FunctionType_LongPass;
  command.touchInfo.inputPower = 0.8f;
  command.touchInfo.desiredPower = 0.8f;
  commandQueue.push_back(command);
}

float ElizaController::_GetPassingOdds(Player *targetPlayer, e_FunctionType passType, const std::vector<PlayerImagePosition> &opponentPlayerImages, float ballVelocityMultiplier) {

  float initialTargetDistance = (targetPlayer->GetPosition() - player->GetPosition()).GetLength();
  if (passType == e_FunctionType_HighPass && initialTargetDistance < 10.0f) return 0.0f;
  float estimatedTime_sec = 0.7f + initialTargetDistance * 0.03f;

  Vector3 target = targetPlayer->GetPosition() + targetPlayer->GetMovement() * clamp(estimatedTime_sec, 0.0f, 0.5f); // time needed to brake
  if (passType == e_FunctionType_LongPass) target += Vector3(-team->GetSide() * initialTargetDistance * 0.2f, 0, 0);

  return _GetPassingOdds(target, passType, opponentPlayerImages, ballVelocityMultiplier);
}

float ElizaController::_GetPassingOdds(const Vector3 &target, e_FunctionType passType, const std::vector<PlayerImagePosition> &opponentPlayerImages, float ballVelocityMultiplier) {

  float secondScale = 1.0f; // how many seconds of range to measure danger in

  Vector3 origin = player->GetPosition() + player->GetMovement() * 0.12f;

  // draw imaginary line between this and target player
  Line line(origin, target);

  float danger = 0.0f;
  for (unsigned int opp = 0; opp < opponentPlayerImages.size(); opp++) {
    Vector3 oppPos = opponentPlayerImages.at(opp).position + opponentPlayerImages.at(opp).movement * 0.2f; // + time needed to brake
    float u = 0.0f; // % of line opp is closest to (0 .. 1)
    float oppDistance = 0.0f;
    oppDistance = line.GetDistanceToPoint(oppPos, u);

    if (u >= 0.0f && u <= 1.0f + 0.2f) { // opp is dangerous in the first place
      if ((passType == e_FunctionType_HighPass && (u < 0.2f || u > 0.65f)) ||
           passType != e_FunctionType_HighPass) {
        float clampedU = clamp(u, 0.0f, 1.0f);
        Vector3 intersect = origin * (1.0f - clampedU) + target * clampedU; // where opp is most likely to intercept ball

        float oppToIntersect_sec = (oppDistance + 1.0f) / sprintVelocity; // add some distance to correct for acceleration

        Vector3 originToBallPos = (intersect - origin);
        float penaltyTime = (passType == e_FunctionType_HighPass && u > 0.5f) ? 2.5f : 0.0f; // trapping high balls takes time
        float ballToIntersect_sec = 0.7f + originToBallPos.GetLength() * u * 0.03f + penaltyTime;
        ballToIntersect_sec *= 1.0f / ballVelocityMultiplier;

        danger += clamp(ballToIntersect_sec - oppToIntersect_sec + (secondScale * 0.5f), 0.0f, secondScale); // add some seconds because 0 seconds doesn't mean 0 danger
      }
    }
  }

  if (passType == e_FunctionType_HighPass) danger += 0.4f; // just don't prefer high passing when low passing is applicable as well

  danger = NormalizedClamp(danger, 0.0f, secondScale); // 1 super dangerous dude is basically the same as 100% danger
  float odds = 1.0f - danger;

  return odds;
}

void ElizaController::_AddCelebration(std::vector<PlayerCommand> &commandQueue) {

  signed int xSide = (match->GetBall()->Predict(0).Get2D().coords[0] > 0) ? 1 : -1;
  signed int ySide = team->GetSide();
  if (team->GetLastTouchPlayer()) ySide = (team->GetLastTouchPlayer()->GetPosition().coords[1] > 0) ? 1 : -1;
  Vector3 celebrationPosition = Vector3(pitchHalfW * xSide, pitchHalfH * ySide, 0);

  Vector3 desiredDirection = (celebrationPosition - player->GetPosition()).GetNormalized();
  float desiredVelocityFloat = ClampVelocity((celebrationPosition - player->GetPosition()).GetLength() / 4.0f);
  Vector3 desiredLookAt = player->GetPosition() + player->GetDirectionVec() * 1000;

  int celebrationType = 1;
  if (match->GetLastGoalTeamID() != team->GetID()) {
    celebrationType = 2;
    desiredVelocityFloat = idleVelocity;
  } else {
    celebrationType = 1;
  }

  int madeGoal = 1;
  if (celebrationType == 1) {
    if (team->GetLastTouchPlayer() == player) {
      madeGoal = 2;
      desiredVelocityFloat = sprintVelocity;
    } else {
      madeGoal = 1;
      if (team->GetLastTouchPlayer() != 0) {
        if ((team->GetLastTouchPlayer()->GetPosition() - player->GetPosition()).GetLength() < 20) {
          celebrationPosition = team->GetLastTouchPlayer()->GetPosition();
          desiredDirection = ((team->GetLastTouchPlayer()->GetPosition() * 0.5 + celebrationPosition * 0.5) - player->GetPosition()).GetNormalized();
          desiredVelocityFloat = ClampVelocity((team->GetLastTouchPlayer()->GetPosition() - player->GetPosition()).GetLength() / 2.0);
        } else {
          desiredVelocityFloat = idleVelocity;
        }
        desiredLookAt = player->GetPosition() + team->GetLastTouchPlayer()->GetDirectionVec() * 100;
      }
    }
  }


  // celebration

  if (match->GetActualTime_ms() - match->GetReferee()->GetBuffer().stopTime > 2000 && match->GetActualTime_ms() - match->GetReferee()->GetBuffer().stopTime < 4000) {
    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Special;
    command.useSpecialVar1 = true;
    command.specialVar1 = celebrationType;
    command.useSpecialVar2 = true;
    command.specialVar2 = madeGoal;
    command.useDesiredMovement = false;
    command.useDesiredLookAt = false;
    commandQueue.push_back(command);
  }


  // movement

  {
    PlayerCommand command;
    command.desiredFunctionType = e_FunctionType_Movement;
    command.useDesiredMovement = true;
    command.desiredDirection = desiredDirection;
    command.desiredVelocityFloat = desiredVelocityFloat;
    command.useDesiredLookAt = true;
    command.desiredLookAt = desiredLookAt;
    commandQueue.push_back(command);
  }

}
