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

#include "humancontroller.hpp"
#include <math.h>
#include <cmath>

#include "../../AIsupport/AIfunctions.hpp"

#include "../../../main.hpp"

HumanController::HumanController(Match *match, IHIDevice *hid) : PlayerController(match), hid(hid) {
  Reset();
}

HumanController::~HumanController() {
}

void HumanController::SetPlayer(PlayerBase *player) {
  lastSwitchTime_ms = match->GetActualTime_ms();

  PlayerController::SetPlayer(player);
}

void HumanController::RequestCommand(PlayerCommandQueue &commandQueue) {

  CastPlayer()->SetDesiredTimeToBall_ms(0);

  _Preprocess(); // calculate some variables


  // human input

  Vector3 rawInputDirection;
  float rawInputVelocityFloat = 0;
  _GetHidInput(rawInputDirection, rawInputVelocityFloat);
  _SetInput(rawInputDirection, rawInputVelocityFloat);


  // clear buffer?

  e_FunctionType functionType = CastPlayer()->GetCurrentFunctionType();
  if (actionMode == 2 &&
      (functionType == e_FunctionType_ShortPass ||
       functionType == e_FunctionType_LongPass ||
       functionType == e_FunctionType_HighPass ||
       functionType == e_FunctionType_Shot) && !CastPlayer()->TouchPending()) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }

  if (actionMode == 1 &&
      (functionType == e_FunctionType_Sliding ||
       functionType == e_FunctionType_Interfere) && !CastPlayer()->TouchPending()) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }


  // cancels

  // shot cancel
  if (actionMode == 2 && actionButton == e_ButtonFunction_Shot && hid->GetButton(e_ButtonFunction_ShortPass) && !match->IsInSetPiece()) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }

  // high pass cancel
  if (actionMode == 2 && actionButton == e_ButtonFunction_HighPass && hid->GetButton(e_ButtonFunction_ShortPass) && !match->IsInSetPiece()) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }

  // cancel action buffer
  if (actionMode == 2 && !match->IsInSetPiece() &&
      (actionBufferTime_ms > 2000 ||
       CastPlayer()->GetTimeNeededToGetToBall_ms() > CastPlayer()->GetTimeNeededToGetToBall_previous_ms() + 700 ||
      (CastPlayer()->GetCurrentFunctionType() == e_FunctionType_Interfere))) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }

  // cancel pressure and such
  if (actionMode == 1 && actionBufferTime_ms > 1000) {
    actionMode = 0;
    gauge_ms = 0;
    actionBufferTime_ms = 0;
  }


  // execute buffer?

  if (actionMode == 2) {

    if (!hid->GetButton(actionButton) ||
        (hid->GetButton(actionButton) && gauge_ms > 500) || // allow anim to kick in before queue is complete (before button is released), it will usually touch ball after the remaining time anyway, so we still have time to add more power, yet still respond as fast as possible
        (!CastPlayer()->HasPossession() && !match->IsInSetPiece() && actionBufferTime_ms > 0)) {

      int baseTime_ms = 60; // substract a little because we can't really press a button shorter than this
      float gaugeFactor = (gauge_ms - baseTime_ms) * (1.0f / float(1000 - baseTime_ms));
      gaugeFactor = clamp(gaugeFactor, 0.0f, 1.0f);

      // action button released!

      // force set piece methods
      if (match->IsInSetPiece() && team->GetController()->GetPieceTaker() == player && team->GetController()->GetSetPieceType() == e_GameMode_KickOff) {

        PlayerCommand command;

        command.desiredFunctionType = e_FunctionType_ShortPass;
        command.touchInfo.autoDirectionBias = 1.0f;
        command.touchInfo.autoPowerBias = 1.0f;
        command.touchInfo.inputDirection = player->GetDirectionVec(); // dud
        command.touchInfo.inputPower = 0.1f; // dud

        Vector3 desiredTargetPosition = player->GetPosition() + player->GetDirectionVec() * 1.0f;
        command.touchInfo.forcedTargetPlayer = AI_GetClosestPlayer(team, desiredTargetPosition, false, CastPlayer());

        AI_GetPass(CastPlayer(), command.desiredFunctionType, command.touchInfo.inputDirection, command.touchInfo.inputPower, command.touchInfo.autoDirectionBias, command.touchInfo.autoPowerBias, command.touchInfo.desiredDirection, command.touchInfo.desiredPower, command.touchInfo.targetPlayer, command.touchInfo.forcedTargetPlayer);

        commandQueue.push_back(command);


      } else if (actionButton == e_ButtonFunction_ShortPass) {

        PlayerCommand command;
        command.desiredFunctionType = e_FunctionType_ShortPass;
        command.useDesiredMovement = false;
        command.useDesiredLookAt = false;

        float inputPower = clamp(std::pow(gaugeFactor, 0.7f), 0.01f, 1.0f);
        command.touchInfo.inputDirection = inputDirection;
        command.touchInfo.inputPower = inputPower;
        command.touchInfo.autoDirectionBias = GetConfiguration()->GetReal("gameplay_shortpass_autodirection", _default_ShortPass_AutoDirection);
        command.touchInfo.autoPowerBias = GetConfiguration()->GetReal("gameplay_shortpass_autopower", _default_ShortPass_AutoPower);
        AI_GetPass(CastPlayer(), command.desiredFunctionType, command.touchInfo.inputDirection, command.touchInfo.inputPower, command.touchInfo.autoDirectionBias, command.touchInfo.autoPowerBias, command.touchInfo.desiredDirection, command.touchInfo.desiredPower, command.touchInfo.targetPlayer);

        commandQueue.push_back(command);


      } else if (actionButton == e_ButtonFunction_LongPass) {

        PlayerCommand command;
        command.desiredFunctionType = e_FunctionType_LongPass;
        command.useDesiredMovement = false;
        command.useDesiredLookAt = false;

        float inputPower = clamp(std::pow(gaugeFactor, 0.65f), 0.01f, 1.0f);
        command.touchInfo.inputDirection = inputDirection;
        command.touchInfo.inputPower = inputPower;
        command.touchInfo.autoDirectionBias = GetConfiguration()->GetReal("gameplay_throughpass_autodirection", _default_ThroughPass_AutoDirection);
        command.touchInfo.autoPowerBias = GetConfiguration()->GetReal("gameplay_throughpass_autopower", _default_ThroughPass_AutoPower);
        AI_GetPass(CastPlayer(), command.desiredFunctionType, command.touchInfo.inputDirection, command.touchInfo.inputPower, command.touchInfo.autoDirectionBias, command.touchInfo.autoPowerBias, command.touchInfo.desiredDirection, command.touchInfo.desiredPower, command.touchInfo.targetPlayer);

        commandQueue.push_back(command);


      } else if (actionButton == e_ButtonFunction_HighPass) {

        PlayerCommand command;
        command.desiredFunctionType = e_FunctionType_HighPass;
        command.useDesiredMovement = false;
        command.useDesiredLookAt = false;

        float inputPower = clamp(std::pow(gaugeFactor, 0.55f), 0.01f, 1.0f);
        command.touchInfo.inputDirection = inputDirection;
        command.touchInfo.inputPower = inputPower;
        command.touchInfo.autoDirectionBias = GetConfiguration()->GetReal("gameplay_highpass_autodirection", _default_HighPass_AutoDirection);
        command.touchInfo.autoPowerBias = GetConfiguration()->GetReal("gameplay_highpass_autopower", _default_HighPass_AutoPower);
        AI_GetPass(CastPlayer(), command.desiredFunctionType, command.touchInfo.inputDirection, command.touchInfo.inputPower, command.touchInfo.autoDirectionBias, command.touchInfo.autoPowerBias, command.touchInfo.desiredDirection, command.touchInfo.desiredPower, command.touchInfo.targetPlayer);

        commandQueue.push_back(command);


      } else if (actionButton == e_ButtonFunction_Shot) {

        PlayerCommand command;
        command.desiredFunctionType = e_FunctionType_Shot;
        command.useDesiredMovement = false;
        command.useDesiredLookAt = false;
        command.desiredVelocityFloat = inputVelocityFloat; // this is so we can use sprint/dribble buttons as shot modifiers
        command.touchInfo.inputDirection = inputDirection;
        command.touchInfo.autoDirectionBias = GetConfiguration()->GetReal("gameplay_shot_autodirection", _default_Shot_AutoDirection);
        if (GetHIDevice()->GetDeviceType() == e_HIDeviceType_Keyboard) command.touchInfo.autoDirectionBias = 1.0f;
        command.touchInfo.desiredDirection = AI_GetShotDirection(CastPlayer(), command.touchInfo.inputDirection, command.touchInfo.autoDirectionBias);
        command.touchInfo.desiredPower =
            clamp(std::pow(gaugeFactor, 0.6f), 0.01f, 1.0f);

        commandQueue.push_back(command);

      }

    }


  } else if (actionMode == 1) {

    if (hid->GetButton(actionButton)) {

      if (actionButton == e_ButtonFunction_Sliding) {

        PlayerCommand command;
        command.desiredFunctionType = e_FunctionType_Sliding;
        command.useDesiredMovement = true;
        command.desiredDirection = inputDirection;
        command.desiredVelocityFloat = inputVelocityFloat;
        command.useDesiredLookAt = true;
        command.desiredLookAt = CastPlayer()->GetPosition() + CastPlayer()->GetMovement() * 0.1f + command.desiredDirection * 10.0f;
        commandQueue.push_back(command);

      }

      if (actionButton == e_ButtonFunction_TeamPressure) {

        team->GetController()->ApplyTeamPressure();

      }

      if (actionButton == e_ButtonFunction_KeeperRush) {

        team->GetController()->ApplyKeeperRush();

      }

    } else {

      // action button released!
      actionMode = 0;

    }
  }

  // set piece?
  if ((match->IsInSetPiece() && team->GetController()->GetPieceTaker() == player && (actionMode != 2 || (actionMode == 2 && hid->GetButton(actionButton)) || match->GetBallRetainer() == player)) ||
      (match->IsInSetPiece() && team->GetController()->GetPieceTaker() != player && match->GetBallRetainer() == 0)) {
    _SetPieceCommand(commandQueue);
    //if (team->GetController()->GetPieceTaker() == player) printf("waiting to take set piece!\n");
    //if (team->GetController()->GetPieceTaker() != player) printf("waiting for teammate to take set piece!\n");
    return;
  }

  // delay direction input until we have chosen a steady direction.
  // this is because humans can only move an analog stick so fast, and we don't want requeues to happen mid-analogstick-movement.
  Vector3 inputDirectionSaveNonsteady = inputDirection;
  //SetGreenDebugPilon(player->GetPosition() + inputDirection * (inputVelocityFloat * 0.5f + 0.5f));
  inputDirection = steadyDirection;
  //SetBlueDebugPilon(player->GetPosition() + inputDirection * (inputVelocityFloat * 0.5f + 0.6f));

  if (match->IsInPlay() && !match->IsInSetPiece()) {

    bool idleTurnToOpponentGoal = false;
    bool knockOn = false;
    if (hid->GetButton(e_ButtonFunction_Dribble)) idleTurnToOpponentGoal = true;
    if (hid->GetButton(e_ButtonFunction_Dribble) && hid->GetButton(e_ButtonFunction_Sprint)) knockOn = true;

    // special adapted input for ballcontrol and trap, when we have shoot/pass buffers
    Vector3 inputDirectionSave2 = inputDirection;
    float inputVelocitySave2 = inputVelocityFloat;
    if (actionMode == 2) {
      float dot = CastPlayer()->GetDirectionVec().GetDotProduct(inputDirection) * 0.5f + 0.5f;
      if (dot < 0) {
        dot = 0;
      }
      if (CastPlayer()->GetEnumVelocity() != e_Velocity_Idle) {
        inputDirection = (CastPlayer()->GetDirectionVec() * 1.0f + inputDirection * 0.0f).GetNormalized(inputDirection); // want inputdirection to be biggest, so we won't stubbornly fail to do 180s
      }
      dot = std::pow(dot, 1.5f);  // prefer braking, even on slight angles
      dot = dot * 0.8f + 0.2f;
      dot *= 0.9f; // else, <=walk-anims may never work (backheels and such)
      inputVelocityFloat = CastPlayer()->GetFloatVelocity() * dot;
    }

    // ball control?
    bool keepCurrentBodyDirection = false;
    // sidestep dribble disabled for now, too quirky: if (hid->GetButton(e_ButtonFunction_Dribble)) keepCurrentBodyDirection = true;
    _BallControlCommand(commandQueue, idleTurnToOpponentGoal, knockOn, true, keepCurrentBodyDirection);

    // trap?
    _TrapCommand(commandQueue, idleTurnToOpponentGoal, knockOn);

    // reload original input
    if (actionMode == 2) {
      inputDirection = inputDirectionSave2;
      inputVelocityFloat = inputVelocitySave2;
    }

    // interfere?
    bool byAnyMeans = false;
    if (hid->GetButton(e_ButtonFunction_Pressure)) byAnyMeans = true;
    _InterfereCommand(commandQueue, byAnyMeans);
  }

  // movement
  bool forceMagnet = false;
  bool extraHaste = false;
  if (actionMode != 2 && hid->GetButton(e_ButtonFunction_Pressure)) {
    forceMagnet = true;
    extraHaste = true;
  }
  if (actionMode == 2) {
    forceMagnet = true;
    extraHaste = true;
  }
  _MovementCommand(commandQueue, forceMagnet, extraHaste);

  if (commandQueue.size() > 0) {
    PlayerCommand &command = commandQueue.at(commandQueue.size() - 1);
    assert(command.desiredFunctionType == e_FunctionType_Movement); // make sure this is the movement command (is probably guaranteed, check out _MovementCommand)

    if (!match->GetUseMagnet()) {
      // no magnet
      command.desiredDirection = inputDirection;
      command.desiredVelocityFloat = inputVelocityFloat;
      if (command.desiredVelocityFloat < idleDribbleSwitch) command.desiredDirection = (_mentalImage->GetBallPrediction(500).Get2D() - player->GetPosition()).GetNormalized(inputDirection);
      //command.desiredLookAt = CastPlayer()->GetPosition() + inputDirection * 10;
      command.desiredLookAt = _mentalImage->GetBallPrediction(500).Get2D();
    }

    // super cancel
    if (match->IsInPlay() && !match->IsInSetPiece() &&
        hid->GetButton(e_ButtonFunction_Dribble) &&
        hid->GetButton(e_ButtonFunction_Sprint)) {
      if (!hasBestPossession) {
        command.desiredDirection = inputDirection;
        command.desiredVelocityFloat = inputVelocityFloat;
      }
    }
  }

  // reload original input
  inputDirection = inputDirectionSaveNonsteady;
}

void HumanController::Process() {

  // just doesn't work so well (fixes the 'humans can't change stick pos instantly' problem, but introduces too much lag). maybe revisit/update later
  bool enableSteadyDirectionSystem = false;

  PlayerController::Process();

  Vector3 currentDirection;
  float dud = 0.0f;
  _GetHidInput(currentDirection, dud);
  radian angle = fabs(currentDirection.GetAngle2D(previousDirection));
  previousDirection = currentDirection;

  // only set steadydirection if angle is small (= human probably reaching his intended direction)
  // or very large (= maybe the stick has been in deadzone space; humans can't move this fast)
  if (enableSteadyDirectionSystem) {
    if (angle < 0.01f * pi || angle > 0.65f * pi || (match->GetActualTime_ms() - lastSteadyDirectionSnapshotTime_ms) > 100) {
      steadyDirection = currentDirection;
      lastSteadyDirectionSnapshotTime_ms = match->GetActualTime_ms();
    }
  } else {
    steadyDirection = currentDirection;
    lastSteadyDirectionSnapshotTime_ms = match->GetActualTime_ms();
  }

  _CalculateSituation();

  // action?

  if (actionMode == 0 && (!match->IsInSetPiece() || team->GetController()->GetPieceTaker() == player)) {



    // what is the context: do we want defend buttons or pass/shot buttons?
    float possessionContext = possessionAmount - 1.0f;
    if (match->GetDesignatedPossessionPlayer() == player) {
      possessionContext = 1.0f; // new (keeper was allowed doing slidings before free kick sometimes lol, sign something was wrong)
    } else {
      // in situations where we aren't the designated player, we sometimes still want to do ball stuff, because we could try to extend our leg to pass, for example
      if (hid->GetButton(e_ButtonFunction_ShortPass)) possessionContext += 0.15f;
      if (hid->GetButton(e_ButtonFunction_LongPass)) possessionContext += 0.15f;
      if (hid->GetButton(e_ButtonFunction_Shot)) possessionContext += 0.15f;

      if (hid->GetButton(e_ButtonFunction_Pressure)) possessionContext -= 0.15f;
      if (hid->GetButton(e_ButtonFunction_Sliding)) possessionContext -= 0.15f;
      if (hid->GetButton(e_ButtonFunction_TeamPressure)) possessionContext -= 0.15f;

      if (match->GetBall()->Predict(0).coords[2] > 1.5f) possessionContext += 0.2f;
    }

    if (possessionContext < 0.0f) {
      bool allowPressure = true;
      bool allowSliding = true;
      bool allowTeamPressure = true;
      bool allowKeeperRush = true;

      if (match->IsInSetPiece()) {
        allowPressure = false;
        allowSliding = false;
        allowTeamPressure = false;
        allowKeeperRush = false;
      }

      if (hid->GetButton(e_ButtonFunction_Pressure) && !hid->GetPreviousButtonState(e_ButtonFunction_Pressure) && allowPressure) {
        actionMode = 1;
        actionButton = e_ButtonFunction_Pressure;
      }

      if (hid->GetButton(e_ButtonFunction_Sliding) && !hid->GetPreviousButtonState(e_ButtonFunction_Sliding) && allowSliding) { // we don't want high passes to turn into slidings
        actionMode = 1;
        actionButton = e_ButtonFunction_Sliding;
      }

      if (hid->GetButton(e_ButtonFunction_TeamPressure) && !hid->GetPreviousButtonState(e_ButtonFunction_TeamPressure) && allowTeamPressure) {
        actionMode = 1;
        actionButton = e_ButtonFunction_TeamPressure;
      }

      if (hid->GetButton(e_ButtonFunction_KeeperRush) && !hid->GetPreviousButtonState(e_ButtonFunction_KeeperRush) && allowKeeperRush) {
        actionMode = 1;
        actionButton = e_ButtonFunction_KeeperRush;
      }

    } else {

      bool allowShortPass = true;
      bool allowLongPass = true;
      bool allowHighPass = true;
      bool allowShot = true;

      if (team->GetController()->GetPieceTaker() == player && team->GetController()->GetSetPieceType() == e_GameMode_ThrowIn) {
        allowHighPass = false;
        allowShot = false;
      }

      if (hid->GetButton(e_ButtonFunction_ShortPass) && !hid->GetPreviousButtonState(e_ButtonFunction_ShortPass) && allowShortPass) {
        actionMode = 2;
        actionButton = e_ButtonFunction_ShortPass;
      }

      if (hid->GetButton(e_ButtonFunction_LongPass) && !hid->GetPreviousButtonState(e_ButtonFunction_LongPass) && allowLongPass) {
        actionMode = 2;
        actionButton = e_ButtonFunction_LongPass;
      }

      if (hid->GetButton(e_ButtonFunction_HighPass) && !hid->GetPreviousButtonState(e_ButtonFunction_HighPass) && allowHighPass) {
        actionMode = 2;
        actionButton = e_ButtonFunction_HighPass;
      }

      if (hid->GetButton(e_ButtonFunction_Shot) && !hid->GetPreviousButtonState(e_ButtonFunction_Shot) && allowShot) {
        actionMode = 2;
        actionButton = e_ButtonFunction_Shot;
      }

    }

  }

  if (actionMode == 2) {
    if (hid->GetButton(actionButton)) {
      gauge_ms += 10;
      gauge_ms = clamp(gauge_ms, 10, 1000);
      actionBufferTime_ms = 0;
    } else {
      // button released, stay in this actionMode until actionBufferTime_ms becomes too big
      actionBufferTime_ms += 10;
    }
  }

  if (hid->GetButton(e_ButtonFunction_Switch) && hasPossession) team->GetController()->ApplyAttackingRun();

}

Vector3 HumanController::GetDirection() {
  Vector3 direction = CastPlayer()->GetDirectionVec();
  return hid->GetDirection().GetNormalized(direction);
}

float HumanController::GetFloatVelocity() {
  Vector3 rawInputDirection;
  float rawInputVelocityFloat = 0;
  _GetHidInput(rawInputDirection, rawInputVelocityFloat);
  return rawInputVelocityFloat;
}

int HumanController::GetReactionTime_ms() {
  return IController::GetReactionTime_ms(); // already have human reaction time to contend with
}

void HumanController::Reset() {
  actionMode = 0;
  gauge_ms = 0;
  actionButton = e_ButtonFunction_ShortPass;
  actionBufferTime_ms = 0;

  lastSwitchTime_ms = -10000;
  lastSwitchTimeDuration_ms = 300;

  lastSteadyDirectionSnapshotTime_ms = 0;
  steadyDirection = Vector3(0, -1, 0);
  previousDirection = Vector3(0, -1, 0);

  fadingTeamPossessionAmount = 1.0;
}

void HumanController::_GetHidInput(Vector3 &rawInputDirection, float &rawInputVelocityFloat) {
  rawInputDirection = hid->GetDirection();
  if (CastPlayer()->GetPosition().coords[0] > pitchHalfW) {
    rawInputDirection.coords[0] = std::min(0.0f, rawInputDirection.coords[0]);
  }
  if (CastPlayer()->GetPosition().coords[0] < -pitchHalfW) {
    rawInputDirection.coords[0] = std::max(0.0f, rawInputDirection.coords[0]);
  }
  if (CastPlayer()->GetPosition().coords[1] > pitchHalfH) {
    rawInputDirection.coords[1] = std::min(0.0f, rawInputDirection.coords[1]);
  }
  if (CastPlayer()->GetPosition().coords[1] < -pitchHalfH) {
    rawInputDirection.coords[1] = std::max(0.0f, rawInputDirection.coords[1]);
  }
  if (rawInputDirection.GetLength() < analogStickDeadzone) {
    rawInputDirection = CastPlayer()->GetDirectionVec();
    rawInputVelocityFloat = idleVelocity;
  } else {
    if (hid->GetButton(e_ButtonFunction_Sprint)) rawInputVelocityFloat = sprintVelocity;
    else if (hid->GetButton(e_ButtonFunction_Dribble)) rawInputVelocityFloat = dribbleVelocity;
    else if (hid->GetButton(e_ButtonFunction_Switch) && match->GetDesignatedPossessionPlayer() == CastPlayer()) rawInputVelocityFloat = idleVelocity;
    else rawInputVelocityFloat = walkVelocity;
    assert(rawInputDirection.GetLength() > 0.001f);
    rawInputDirection.Normalize(); // hid should do this, but still
  }

  if (GetLastSwitchBias() > 0.0f) {
    float switchInfluence = 0.5f;
    float switchBias = std::pow(GetLastSwitchBias(), 0.7f);
    Vector3 currentMovement = player->GetDirectionVec() * player->GetFloatVelocity();
    Vector3 manualMovement = rawInputDirection * rawInputVelocityFloat;
    Vector3 resultMovement = currentMovement * switchBias * switchInfluence +
                             manualMovement * (1.0f - switchBias * switchInfluence);
    rawInputDirection = resultMovement.GetNormalized(rawInputDirection);
    rawInputVelocityFloat = resultMovement.GetLength();
  }

}
