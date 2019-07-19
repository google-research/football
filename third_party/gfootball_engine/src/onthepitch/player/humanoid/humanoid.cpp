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

#include "humanoid.hpp"

#include <cmath>

#include "humanoid_utils.hpp"

#include "../player.hpp"
#include "../../team.hpp"
#include "../../match.hpp"

#include "../../../main.hpp"

#include "../../AIsupport/AIfunctions.hpp"

#include "../../../utils/animationextensions/footballanimationextension.hpp"

#include "../../../systems/graphics/objects/graphics_geometry.hpp"
#include "../../../systems/graphics/graphics_scene.hpp"
#include "../../../systems/graphics/graphics_system.hpp"

#include "../../../managers/resourcemanagerpool.hpp"

const bool animSmoothing = true;
const float cheatFactor = 0.5f;
const bool useContinuousBallCheck = true;
const bool enableMovementSmuggle = true;
const float cheatDiscardDistance = 0.02f; // don't 'display' this distance of cheat (looks better for small distances, but will look funny when too large, players 'missing' the ball and all. also has big influence on gameplay, since this influences player collisions etc)
const float cheatDistanceBonus = 0.02f; // add extra allowed cheat distance (in meters). don't 'display' this distance of cheat (looks better for small distances, but will look funny when too large, players 'missing' the ball and all. also has big influence on gameplay, since this influences player collisions etc)
const float cheatDiscardDistanceMultiplier = 0.4f; // lower == more snappy
const float maxSmuggleDiscardDistance = 0.2f;
const bool enableActionSmuggleDiscard = true;
const bool forceFullActionSmuggleDiscard = false;
const bool discardForwardSmuggle = true;
const bool discardSidewaysSmuggle = false;
const float bodyRotationSmoothingFactor = 1.0f;
const float bodyRotationSmoothingMaxAngle = animSmoothing ? 0.25f * pi : 0.0f;
const int initialReQueueDelayFrames = 22;
const int minRemainingMovementReQueueFrames = 6; // after this # of frames remaining, just let anim finish; we're almost there anyway
const int minRemainingTrapReQueueFrames = 6; // after this # of frames remaining to touchframe, just let anim finish; we're almost there anyway
const int maxBallControlReQueueFrame = 8;
const bool allowReQueue = true;
const bool allowMovementReQueue = true;
const bool allowBallControlReQueue = true;
const bool allowTrapReQueue = true;
const bool allowPreTouchRotationSmuggle = false;
const bool enableControlledBallCollisions = true;

Humanoid::Humanoid(Player *player, boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::shared_ptr<AnimCollection> animCollection, boost::intrusive_ptr<Node> fullbodyTargetNode, boost::intrusive_ptr < Resource<Surface> > kit, int bodyUpdatePhaseOffset) :
                  HumanoidBase(player, player->GetTeam()->GetMatch(), humanoidSourceNode, fullbodySourceNode, colorCoords, animCollection, fullbodyTargetNode, kit, bodyUpdatePhaseOffset) {

  team = CastPlayer()->GetTeam();

  stat_GetBodyBallDistanceAdvantage_RadiusDeny = 0;
  stat_GetBodyBallDistanceAdvantage_DistanceDeny = 0;
}

Humanoid::~Humanoid() {
}

Player *Humanoid::CastPlayer() const { return static_cast<Player*>(player); }

bool _PassFiddlingEnabled() {
  return true;
}

void Humanoid::Process() {

  _cache_AgilityFactor = GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor);
  _cache_AccelerationFactor = GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor);

  // this might be the solution to long-term inbalance
  decayingPositionOffset *= 0.95f;
  if (decayingPositionOffset.GetLength() < 0.005) decayingPositionOffset.Set(0);
  decayingDifficultyFactor = clamp(decayingDifficultyFactor - 0.002f, 0.0f, 1.0f);

  assert(match);

  if (!currentMentalImage) {
    currentMentalImage = match->GetMentalImage(0); // first-time run (todo: ehh, why this specific branch?)
  } else {
    bool instaDoorheb = false;
    if (match->GetLastTouchTeamID() == team->GetID()) instaDoorheb = true;
    currentMentalImage = match->GetMentalImage(instaDoorheb ? 0 : CastPlayer()->GetController()->GetReactionTime_ms());
  }

  CalculateSpatialState();
  spatialState.positionOffsetMovement = Vector3(0);

  currentAnim->frameNum++;
  previousAnim->frameNum++;

  assert(team);
  int teamID = team->GetID();

/*

  if (currentAnim->positionOffset.GetLength() > 0.1f && interruptAnim == e_InterruptAnim_None) {
    interruptAnim = e_InterruptAnim_Bump;
  }
*/

  if (currentAnim->frameNum == currentAnim->anim->GetFrameCount() - 1 && interruptAnim == e_InterruptAnim_None) {
    interruptAnim = e_InterruptAnim_Switch;
  }

  bool mayReQueue = allowReQueue;


  // already some anim interrupt waiting?

  if (mayReQueue) {
    if (interruptAnim != e_InterruptAnim_None) {
      mayReQueue = false;
    }
  }


  // may requeue on this frame?

/* can't do this: requeued movement anims should be requeueable into touch anims
  if (mayReQueue) { // never requeue a requeued anim
    if (currentAnim->originatingInterrupt != e_Interrupt_None &&
        currentAnim->originatingInterrupt != e_Interrupt_Switch) {
    mayRequeue = false;
  }*/

  if (mayReQueue) {
    bool frameNumPredicate = false;
    float actionDistance = ((spatialState.position + spatialState.movement * 0.1f) - match->GetBall()->Predict(100).Get2D()).GetLength();//(spatialState.position - match->GetDesignatedPossessionPlayer()->GetPosition()).GetLength();

    if (match->GetDesignatedPossessionPlayer() == player && actionDistance < 3.0f) {
      frameNumPredicate = ((match->GetActualTime_ms() + team->GetID() * 10) % 20) == 0; // .. 1 .. 2 .. 1 .. 2 ..

    } else if (match->GetDesignatedPossessionPlayer() == player) {
      frameNumPredicate = ((match->GetActualTime_ms() + team->GetID() * 10) % 30) == 0; // .. 1 .. 2 .. x .. 1 .. 2 .. x ..

    } else if (team->GetDesignatedTeamPossessionPlayer() == player) {
      frameNumPredicate = ((match->GetActualTime_ms() + team->GetID() * 20) % 40) == 0; // .. 1 .. x .. 2 .. x .. 1 .. x .. 2 ..

    } else if (actionDistance < 5.0f) {
      frameNumPredicate = ((match->GetActualTime_ms() + team->GetID() * 20) % 50) == 0; // .. 1 .. x .. 2 .. x .. x ..

    } else if (actionDistance < 10.0f) {
      frameNumPredicate = ((match->GetActualTime_ms() + team->GetID() * 40) % 80) == 0; // .. 1 .. x .. x .. x .. 2 .. x .. x .. x ..

    }

    if (!frameNumPredicate) mayReQueue = false;
  }


  // right anim to requeue?

  if (mayReQueue) {

    float ballDistance = (currentMentalImage->GetBallPrediction(500).Get2D() - spatialState.position).GetLength();
    if (( (currentAnim->functionType == e_FunctionType_Movement && !CastPlayer()->HasPossession() && ballDistance < 16.0f) ||
          (currentAnim->functionType == e_FunctionType_Movement && CastPlayer()->HasPossession()) || // passes / shot
          (currentAnim->functionType == e_FunctionType_Trap && TouchPending()) ||
          (currentAnim->functionType == e_FunctionType_BallControl && TouchPending()) ) &&
          /* now done later on, else we can't requeue to pass/shot during trap/ballcontrol
          (currentAnim->functionType == e_FunctionType_Trap && TouchPending() && allowTrapReQueue && currentAnim->frameNum <= maxTrapReQueueFrame) ||
          (currentAnim->functionType == e_FunctionType_BallControl && TouchPending() && allowBallControlReQueue && currentAnim->frameNum <= maxBallControlReQueueFrame) ) &&
          */
        currentAnim->anim->GetVariableCache().incoming_special_state().empty() && currentAnim->anim->GetVariableCache().outgoing_special_state().empty()) {
      mayReQueue = true;
    } else {
      mayReQueue = false;
    }
  }


  // okay, see if we need to requeue

  if (mayReQueue) {
    interruptAnim = e_InterruptAnim_ReQueue;
  }

  if (interruptAnim != e_InterruptAnim_None) {
    PlayerCommandQueue commandQueue;

    if (interruptAnim == e_InterruptAnim_Trip && tripType != 0) {
      AddTripCommandToQueue(commandQueue, tripDirection, tripType);
      tripType = 0;
      commandQueue.push_back(GetBasicMovementCommand(tripDirection, spatialState.floatVelocity)); // backup, if there's no applicable trip anim
    } else {
      CastPlayer()->RequestCommand(commandQueue);
    }


    // iterate through the command queue and pick the first that is applicable

    bool found = false;
    bool preferPassAndShot = false; // pass/shot and such; in that case we want trap/ballcontrol anims to be less prefered
    for (unsigned int i = 0; i < commandQueue.size(); i++) {

      const PlayerCommand &command = commandQueue[i];

      if (command.desiredFunctionType == e_FunctionType_ShortPass ||
          command.desiredFunctionType == e_FunctionType_LongPass ||
          command.desiredFunctionType == e_FunctionType_HighPass ||
          command.desiredFunctionType == e_FunctionType_Shot) {
        preferPassAndShot = true;
      }
      found = SelectAnim(command, interruptAnim, preferPassAndShot);
      if (found) break;
    }

    if (interruptAnim == e_InterruptAnim_Switch && !found) {
      Log(e_Warning, "Humanoid", "Process", "RED ALERT! NO APPLICABLE ANIM FOUND! NOOOO!");
      Log(e_Warning, "Humanoid", "Process", "currentanimtype: " + currentAnim->anim->GetVariable("type"));
      for (unsigned int i = 0; i < commandQueue.size(); i++) {
        Log(e_Warning, "Humanoid", "Process", "desiredanimtype:" + int_to_str(commandQueue[i].desiredFunctionType));
        Log(e_Warning, "Humanoid", "Process", "desired velo: " + real_to_str(commandQueue[i].desiredVelocityFloat));
        Log(e_Warning, "Humanoid", "Process", "desired direction: " + real_to_str(commandQueue[i].desiredDirection.coords[0]) + ", " + real_to_str(commandQueue[i].desiredDirection.coords[1]) + ", " + real_to_str(commandQueue[i].desiredDirection.coords[2]));
      }
      Log(e_Warning, "Humanoid", "Process", "current velo: " + real_to_str(spatialState.floatVelocity));
      Log(e_Warning, "Humanoid", "Process", "current body angle: abs: " + real_to_str(spatialState.bodyAngle) + ", rel: " + real_to_str(spatialState.relBodyAngle));
      Log(e_Warning, "Humanoid", "Process", "current position: " + real_to_str(spatialState.position.coords[0]) + ", " + real_to_str(spatialState.position.coords[1]) + ", " + real_to_str(spatialState.position.coords[2]));
      Log(e_Warning, "Humanoid", "Process", "special state: " + currentAnim->anim->GetVariableCache().outgoing_special_state());
      print_stacktrace();
      exit(1);
    }

    if (found) {
      startPos = spatialState.position;
      startAngle = spatialState.angle;

      CalculatePredictedSituation(nextStartPos, nextStartAngle);

      animApplyBuffer.anim = currentAnim->anim;
      animApplyBuffer.smooth = animSmoothing;
      animApplyBuffer.smoothFactor = (interruptAnim == e_InterruptAnim_Switch && previousAnim->functionType == e_FunctionType_Movement && currentAnim->functionType == e_FunctionType_Movement) ? 0.0f : 1.0f; // more smoothing for mid-anim requeues
      if (currentAnim->functionType == e_FunctionType_Shot) animApplyBuffer.smoothFactor = 0.8f;
      if (currentAnim->functionType == e_FunctionType_ShortPass ||
          currentAnim->functionType == e_FunctionType_HighPass) animApplyBuffer.smoothFactor = 0.8f;
      if (currentAnim->functionType == e_FunctionType_Deflect ||
          currentAnim->functionType == e_FunctionType_Sliding) animApplyBuffer.smoothFactor = 0.8f;
      if (currentAnim->functionType == e_FunctionType_BallControl ||
          currentAnim->functionType == e_FunctionType_Trap) animApplyBuffer.smoothFactor = 0.8f;
      //printf("smoothfac: %f, interrupt: %i\n", animApplyBuffer.smoothFactor, interruptAnim);
      //animApplyBuffer.offsets.clear();

      // decaying difficulty
      float animDiff = atof(currentAnim->anim->GetVariable("animdifficultyfactor").c_str());
      if (animDiff > decayingDifficultyFactor) decayingDifficultyFactor = animDiff;

      // if we just requeued, for example, from movement to ballcontrol, there's no reason we can not immediately requeue to another ballcontrol again (next time). only apply the initial requeue delay on subsequent anims of the same type
      // (so we can have a fast ballcontrol -> ballcontrol requeue, but after that, use the initial delay)
      if (interruptAnim == e_InterruptAnim_ReQueue && previousAnim->functionType == currentAnim->functionType) {
        reQueueDelayFrames = initialReQueueDelayFrames; // don't try requeueing (some types of anims, see selectanim()) too often
      }

    }

  }
  reQueueDelayFrames = std::max(reQueueDelayFrames - 1, 0);

  interruptAnim = e_InterruptAnim_None;

  if (startPos.coords[2] != 0.f) {
    Log(e_FatalError, "Humanoid", "Process", "BWAAAAAH FLYING PLAYERS!! height: " + real_to_str(startPos.coords[2]));
  }

  float ballDistanceNow = (match->GetBall()->Predict(0).Get2D() - spatialState.position).GetLength();
  float ballDistanceFuture = (match->GetBall()->Predict(200).Get2D() - (spatialState.position + spatialState.movement * 0.2f)).GetLength();
  float lastTouchBias = CastPlayer()->GetLastTouchBias(1500);
  float oppLastTouchBias = match->GetTeam(abs(team->GetID() - 1))->GetLastTouchBias(240);

  if (CastPlayer() == match->GetDesignatedPossessionPlayer() &&
    (
      (lastTouchBias <= 0.01f && oppLastTouchBias <= 0.01f && currentAnim->functionType == e_FunctionType_Movement &&
       ballDistanceNow < 0.6f && ballDistanceFuture > 0.65f && ballDistanceFuture > ballDistanceNow) // 0.5 / 0.6

      ||

      (lastTouchBias <= 0.7f && CastPlayer()->HasPossession() && currentAnim->functionType == e_FunctionType_Trip && ballDistanceNow < 0.4f)

    ) && match->GetBall()->Predict(0).coords[2] < 1.6f) {

    CastPlayer()->TriggerControlledBallCollision();
    //SetGreenDebugPilon(spatialState.position);
  }

  // ------------------------ EXPERIMENTAL ------------------------------------------------
  bool controlledBallCollision = CastPlayer()->IsControlledBallCollisionTriggered();
  if (controlledBallCollision) CastPlayer()->ResetControlledBallCollisionTrigger();
  if (enableControlledBallCollisions && controlledBallCollision && currentAnim->touchFrame == -1) {
    Vector3 currentBallVec = match->GetBall()->GetMovement();
    radian nextBodyAngle = startAngle + currentAnim->anim->GetOutgoingAngle() + currentAnim->anim->GetOutgoingBodyAngle() + currentAnim->rotationSmuggle.end;

    radian xRot = 0;
    radian yRot = 0;
    Vector3 touchVec = GetTrapVector(match, CastPlayer(), nextStartPos, nextStartAngle, nextBodyAngle, CalculateOutgoingMovement(currentAnim->positions), currentAnim, currentAnim->frameNum, spatialState, decayingPositionOffset, xRot, yRot);
    if (currentAnim->originatingCommand.modifier & e_PlayerCommandModifier_KnockOn) {
      touchVec *= 1.35f;//1.2f;
    }

    float bumpyRideBias = 0.0f;
    touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

    match->GetBall()->Touch(touchVec);
    match->GetBall()->SetRotation(xRot, yRot, 0, 0.2f * (1.0f - bumpyRideBias)); // 0.9
    team->SetLastTouchPlayer(CastPlayer(), GetTouchTypeForBodyPart(currentAnim->anim->GetVariable("touch_bodypart")));//, e_TouchType_Accidental);
    CastPlayer()->UpdatePossessionStats(false);
  }
  // ---------------------- / EXPERIMENTAL ------------------------------------------------

  if (currentAnim->touchFrame == currentAnim->frameNum) {

    Vector3 desiredBallPosition;
    boost::static_pointer_cast<FootballAnimationExtension>(currentAnim->anim->GetExtension("football"))->GetTouchPos(currentAnim->touchFrame, desiredBallPosition);
    float desiredBallHeight = desiredBallPosition.coords[2];

    float touchableDistance = 0.4f;

    float fullBallDistance = (match->GetBall()->Predict(0) - (currentAnim->touchPos + currentAnim->positionOffset)).GetLength();

    if (!currentAnim->anim->GetVariableCache().incoming_retain_state().empty()) {
      fullBallDistance = 0.0f;
      touchableDistance = 1.0f;
    }

    float bumpyRideBias = fullBallDistance / touchableDistance;
    bumpyRideBias = clamp(bumpyRideBias - 0.001f, 0.0f, 1.0f);
    bumpyRideBias = curve(bumpyRideBias, 1.0f);
    bumpyRideBias = curve(bumpyRideBias, 0.5f);
    Vector3 currentBallVec = match->GetBall()->GetMovement();

    if (fullBallDistance < touchableDistance && fabs(desiredBallHeight - match->GetBall()->Predict(0).coords[2]) < 1.0f) {

      radian nextBodyAngle = startAngle + currentAnim->anim->GetOutgoingAngle() + currentAnim->anim->GetOutgoingBodyAngle() + currentAnim->rotationSmuggle.end;

      if (currentAnim->functionType == e_FunctionType_Trap || (currentAnim->functionType == e_FunctionType_BallControl && CastPlayer()->HasPossession() == false)) {
        //printf("trap!\n");
        radian xRot = 0;
        radian yRot = 0;
        Vector3 touchVec = GetTrapVector(match, CastPlayer(), nextStartPos, nextStartAngle, nextBodyAngle, CalculateOutgoingMovement(currentAnim->positions), currentAnim, currentAnim->frameNum, spatialState, decayingPositionOffset, xRot, yRot);
        if (currentAnim->originatingCommand.modifier & e_PlayerCommandModifier_KnockOn) {
          touchVec *= 1.35f;
        }

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);
        match->GetBall()->SetRotation(xRot, yRot, 0, 0.5f * (1.0f - bumpyRideBias));

        team->SetLastTouchPlayer(CastPlayer(), GetTouchTypeForBodyPart(currentAnim->anim->GetVariable("touch_bodypart")));
        CastPlayer()->UpdatePossessionStats(false);
      }

      else if (currentAnim->functionType == e_FunctionType_BallControl) {
        radian xRot = 0;
        radian yRot = 0;
        Vector3 touchVec = GetBallControlVector(match->GetBall(), CastPlayer(), nextStartPos, nextStartAngle, nextBodyAngle, CalculateOutgoingMovement(currentAnim->positions), currentAnim, currentAnim->frameNum, spatialState, decayingPositionOffset, xRot, yRot);
        if (currentAnim->originatingCommand.modifier & e_PlayerCommandModifier_KnockOn) {
          touchVec *= 1.35f;
        }

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);
        match->GetBall()->SetRotation(xRot, yRot, 0, 0.6f * (1.0f - bumpyRideBias)); // 1.0

        team->SetLastTouchPlayer(CastPlayer(), GetTouchTypeForBodyPart(currentAnim->anim->GetVariable("touch_bodypart")));
        CastPlayer()->UpdatePossessionStats(false);
      }

      else if (currentAnim->functionType == e_FunctionType_ShortPass ||
               currentAnim->functionType == e_FunctionType_LongPass ||
               currentAnim->functionType == e_FunctionType_HighPass) {

        Vector3 ballDirection = currentAnim->originatingCommand.touchInfo.desiredDirection;
        float ballPower = currentAnim->originatingCommand.touchInfo.desiredPower;
        Player *targetPlayer = currentAnim->originatingCommand.touchInfo.targetPlayer;
        Vector3 inputDirection = currentAnim->originatingCommand.touchInfo.inputDirection;
        if (CastPlayer()->GetExternalController()) inputDirection = CastPlayer()->GetExternalController()->GetDirection();


        // refine/change target, if new target is close enough to old target

        //targetPlayer = 0;//currentAnim->originatingCommand.touchInfo.targetPlayer;
        Vector3 tmpBallDirection = ballDirection;
        float tmpBallPower = ballPower;
        Player *tmpTargetPlayer = 0;
        Player *forcedTargetPlayer = 0;
        AI_GetPass(CastPlayer(), currentAnim->originatingCommand.desiredFunctionType, inputDirection, currentAnim->originatingCommand.touchInfo.inputPower, currentAnim->originatingCommand.touchInfo.autoDirectionBias, currentAnim->originatingCommand.touchInfo.autoPowerBias, tmpBallDirection, tmpBallPower, tmpTargetPlayer, currentAnim->originatingCommand.touchInfo.forcedTargetPlayer);
        float maxDeviationAngle = 0.15f * pi;
        radian angleDiff = tmpBallDirection.Get2D().GetAngle2D(ballDirection.Get2D());
        if (fabs(angleDiff) <= maxDeviationAngle) {
          ballDirection = tmpBallDirection;
          ballPower = tmpBallPower;
          targetPlayer = tmpTargetPlayer;
        } else if (fabs(angleDiff) < 2.0f * maxDeviationAngle) {
          // get as close as possible
          float clampedAngleDiff = clamp(angleDiff, -maxDeviationAngle, maxDeviationAngle);
          ballDirection = ballDirection.GetRotated2D(clampedAngleDiff);

          if (tmpTargetPlayer != targetPlayer) {
            // if we can't make it to our refined target at all, just stick with original ballpower (think about refined target at ~180 deg, would be weird to pass forward with the power of that (unreachable) target)
            float refinedBias = NormalizedClamp(fabs(clampedAngleDiff), 0.0f, fabs(angleDiff));
            ballPower = ballPower * (1.0f - refinedBias) + tmpBallPower * refinedBias;
            targetPlayer = tmpTargetPlayer; // new, and removed line below
          } else {
            ballPower = tmpBallPower;
          }

        } // else: just stick to original


        if (targetPlayer) team->SelectPlayer(targetPlayer);
        //if (targetPlayer) SetGreenDebugPilon(targetPlayer->GetPosition());

        float zcurve = 0.0f;
        Vector3 touchVec = ballDirection * 36 * (ballPower + 0.3f);

        if (_PassFiddlingEnabled()) {
          //SetGreenDebugPilon(match->GetBall()->Predict(0).Get2D() + touchVec.Get2D() * 0.4f);

          touchVec = GetBestPossibleTouch(touchVec, currentAnim->functionType);

          // add a little curve for aesthetics & realism
          radian bodyTouchAngle = spatialState.bodyDirectionVec.GetAngle2D(touchVec) / pi;
          if (fabs(bodyTouchAngle) > 0.5f) bodyTouchAngle = (1.0f - fabs(bodyTouchAngle)) * signSide(bodyTouchAngle);
          bodyTouchAngle *= 2.0f;
          //printf("bodyTouchAngle: %f\n", bodyTouchAngle);
          radian amount = bodyTouchAngle * 0.25f;
          if (currentAnim->functionType == e_FunctionType_HighPass) amount *= 0.2f;
          touchVec.Rotate2D(amount * (0.4f + 0.6f * NormalizedClamp(touchVec.GetLength(), 0.0f, 70.0f)));
          zcurve = amount * -340;//-600;

          //SetRedDebugPilon(match->GetBall()->Predict(0).Get2D() + touchVec.Get2D() * 0.4f);
        }

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);
        float forwardness = 3.5f;
        if (currentAnim->functionType == e_FunctionType_HighPass) forwardness = -1.3f;
        radian xRot = touchVec.GetNormalized(0).coords[1] * (clamp(touchVec.GetLength(), 0.0, 15.0) * forwardness);
        radian yRot = touchVec.GetNormalized(0).coords[0] * (clamp(touchVec.GetLength(), 0.0, 15.0) * forwardness);
        match->GetBall()->SetRotation(xRot, yRot, zcurve, 0.9f * (1.0f - bumpyRideBias));

        team->SetLastTouchPlayer(CastPlayer(), GetTouchTypeForBodyPart(currentAnim->anim->GetVariable("touch_bodypart")));
        CastPlayer()->UpdatePossessionStats(false);
        if (targetPlayer) targetPlayer->UpdatePossessionStats(false);
      }

      else if (currentAnim->functionType == e_FunctionType_Shot) {

        // alter direction, if neeeded
        Vector3 ballDirection = currentAnim->originatingCommand.touchInfo.desiredDirection;
        Vector3 inputDirection = currentAnim->originatingCommand.touchInfo.inputDirection;
        if (CastPlayer()->GetExternalController()) inputDirection = CastPlayer()->GetExternalController()->GetDirection();
        Vector3 ballDirectionAltered = AI_GetShotDirection(CastPlayer(), inputDirection, currentAnim->originatingCommand.touchInfo.autoDirectionBias);

        float maxDeviationAngle = 0.1f * pi;
        radian angleDiff = ballDirectionAltered.Get2D().GetAngle2D(ballDirection.Get2D());
        if (fabs(angleDiff) > maxDeviationAngle) {
          // get as close as possible
          float clampedAngleDiff = clamp(angleDiff, -maxDeviationAngle, maxDeviationAngle);
          ballDirection = ballDirection.GetRotated2D(clampedAngleDiff);
        } else {
          ballDirection = ballDirectionAltered;
        }

        radian xRot = 0;
        radian yRot = 0;
        radian zRot = 0;
        Vector3 touchVec = GetShotVector(match, CastPlayer(), nextStartPos, nextStartAngle, nextBodyAngle, CalculateOutgoingMovement(currentAnim->positions), currentAnim, currentAnim->frameNum, spatialState, decayingPositionOffset, xRot, yRot, zRot, currentAnim->originatingCommand.touchInfo.autoDirectionBias);

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);
        match->GetBall()->SetRotation(xRot, yRot, zRot, 0.7f * (1.0f - bumpyRideBias));
        team->SetLastTouchPlayer(CastPlayer(), GetTouchTypeForBodyPart(currentAnim->anim->GetVariable("touch_bodypart")));
        match->GetMatchData()->AddShot(team->GetID());
      }

      else if (currentAnim->functionType == e_FunctionType_Interfere) {
        radian xRot = 0;
        radian yRot = 0;
        Vector3 touchVec = GetTrapVector(match, CastPlayer(), nextStartPos, nextStartAngle, nextBodyAngle, CalculateOutgoingMovement(currentAnim->positions), currentAnim, currentAnim->frameNum, spatialState, decayingPositionOffset, xRot, yRot);
        touchVec = touchVec * 0.5f + (match->GetBall()->Predict(0).Get2D() - spatialState.position).GetNormalized() * 4.0f + Vector3(0, 0, random(0.5f, 1.5f)); // was 1 .. 6

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);
        match->GetBall()->SetRotation(xRot, yRot, 0.3f * (1.0f - bumpyRideBias));
        team->SetLastTouchPlayer(CastPlayer(), e_TouchType_Accidental); // it's not truly accidental, but the resulting direction somewhat is, so goalies may fetch these balls
      }

      else if (currentAnim->functionType == e_FunctionType_Deflect) {
        bool canRetain = true; // can we grab hold of the ball?
        if (currentAnim->anim->GetVariable("outgoing_retain_state").compare("") == 0) canRetain = false; // not the right anim, hopeless!
        if (match->GetBallRetainer() != 0) canRetain = false; // somebody is already holding the ball :( (dafuq, this should not happen, right?)

        float veloDifficulty = NormalizedClamp((match->GetBall()->GetMovement() - player->GetMovement()).GetLength(), 0.0f, 40.0f);
        float reactionDifficulty = 0.0f;
        Player *lastTouchPlayer = match->GetTeam(abs(team->GetID() - 1))->GetLastTouchPlayer();
        if (lastTouchPlayer) {
          reactionDifficulty =
              std::pow(lastTouchPlayer->GetLastTouchBias(
                           1200 - player->GetStat(physical_reaction) * 400),
                       0.6f);
        }
        if ((1.0f - veloDifficulty) * (1.0f - reactionDifficulty) < 0.3f) canRetain = false; // too hard!

        if (canRetain) {
          match->SetBallRetainer(CastPlayer());
        } else {
          Vector3 currentBallMovement = match->GetBall()->GetMovement().Get2D();
          Vector3 playerMovement = spatialState.movement;
          Vector3 touchVec = (-currentBallMovement * 0.1f + playerMovement * 2.0f + Vector3(-team->GetSide(), 0, 0) * 4.0f + Vector3(0, random(-1, 1), 0)).GetNormalized(0) * (currentBallMovement.GetLength() * 0.3f + playerMovement.GetLength() * 2.5f);
          touchVec.coords[2] += 1.2f;

          touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

          match->GetBall()->Touch(touchVec);
          match->GetBall()->SetRotation(0, 0, 0, 0.2f * (1.0f - bumpyRideBias));
        }
        team->SetLastTouchPlayer(CastPlayer(), e_TouchType_Accidental);
      }

      else if (currentAnim->functionType == e_FunctionType_Sliding) {
        Vector3 touchVec = GetVectorFromString(currentAnim->anim->GetVariable("balldirection")).GetRotated2D(spatialState.angle);
        touchVec = touchVec * 6.0f + match->GetBall()->GetMovement() * -0.28f;
        touchVec += Vector3(0, 0, 6);

        touchVec = touchVec * (1.0f - bumpyRideBias) + currentBallVec * bumpyRideBias;

        match->GetBall()->Touch(touchVec);

        team->SetLastTouchPlayer(CastPlayer(), e_TouchType_Accidental);
      }

    }
  }

  if (match->GetBallRetainer() == player) {
    if ((currentAnim->touchFrame <= currentAnim->frameNum && currentAnim->anim->GetVariable("outgoing_retain_state") != "") ||
        (currentAnim->touchFrame >  currentAnim->frameNum && currentAnim->anim->GetVariableCache().incoming_retain_state() != "") ||
        (currentAnim->anim->GetVariable("incoming_retain_state") != "" && currentAnim->anim->GetVariable("outgoing_retain_state") != "")) {
      // find body part the ball is stuck to (superglue powers)
      auto outgoing = currentAnim->anim->GetVariable("outgoing_retain_state");
      auto bodyPart = outgoing.empty() ? nullptr : nodeMap[BodyPartFromString(outgoing)];
      if (!bodyPart) {
        auto incoming = currentAnim->anim->GetVariable("incoming_retain_state");
        bodyPart = incoming.empty() ? nullptr : nodeMap[BodyPartFromString(incoming)];
      }
      assert(bodyPart);
      match->GetBall()->Touch(Vector3(0));
      match->GetBall()->SetRotation(0, 0, 0, 1.0);
      match->GetBall()->SetPosition(bodyPart->GetDerivedPosition() + bodyPart->GetDerivedRotation() * Vector3(0, 0, -0.36f));
      team->SetLastTouchPlayer(CastPlayer(), e_TouchType_Intentional_Nonkicked);
    } else {
      // no longer retaining
      match->SetBallRetainer(0);
    }
  }


  // action smuggle

  // start with +1, because we want to influence the first frame as well
  // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
  // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
  float frameBias = (currentAnim->frameNum + 1) / (float)(currentAnim->anim->GetEffectiveFrameCount() + 1);

  if (currentAnim->touchFrame != -1 && currentAnim->frameNum <= currentAnim->touchFrame) {
    // linear version *outdated*
    // spatialState.actionSmuggleMovement = currentAnim->actionSmuggle / (float)(currentAnim->touchFrame - 1.0f);
    // currentAnim->actionSmuggleOffset += spatialState.actionSmuggleMovement;

    // smooth version
    assert(currentAnim->touchFrame > 0.0f);
    float value =
        std::cos((currentAnim->frameNum / (float)(currentAnim->touchFrame + 1) -
                  0.5f) *
                 pi * 2.0f) +
        1.0f;
    // add some linearity
    value = value * 0.1f + 0.9f;
    spatialState.actionSmuggleMovement = (currentAnim->actionSmuggle / (float)(currentAnim->touchFrame + 1)) * value * 100.0f;
    currentAnim->actionSmuggleOffset += spatialState.actionSmuggleMovement / 100.0f;

  } else {
    spatialState.actionSmuggleMovement = Vector3(0);
  }


  // movement smuggle

  if (currentAnim->touchFrame == -1 && currentAnim->frameNum <= currentAnim->anim->GetEffectiveFrameCount()) { // omit one frame, or balltouch will be influenced because of velo
    // linear version *outdated*
    // spatialState.movementSmuggleMovement = currentAnim->movementSmuggle / (float)(currentAnim->anim->GetEffectiveFrameCount());
    // currentAnim->movementSmuggleOffset += spatialState.movementSmuggleMovement;

    // smooth version
    float value =
        std::cos((currentAnim->frameNum /
                      (float)(currentAnim->anim->GetEffectiveFrameCount() + 1) -
                  0.5f) *
                 pi * 2.0f) +
        1.0f;
    // add some linearity
    value = value * 0.1f + 0.9f;
    spatialState.movementSmuggleMovement = (currentAnim->movementSmuggle / (float)(currentAnim->anim->GetEffectiveFrameCount() + 1)) * value * 100.0f;
    currentAnim->movementSmuggleOffset += spatialState.movementSmuggleMovement / 100.0f;
  } else {
    spatialState.movementSmuggleMovement = Vector3(0);
  }


  // rotation smuggle

  int beginRotationFrameCount = 16; // after this amount of frames, be ready with 'ease-in' rotation smuggle
  float cappedFrameBias = std::min(1.0f, (currentAnim->frameNum + 1) / (float)std::min(beginRotationFrameCount, currentAnim->anim->GetEffectiveFrameCount() + 1));
  float beginFrameBias = cappedFrameBias;
  float endFrameBias = cappedFrameBias;
  if (currentAnim->touchFrame != -1) {
    // beginFrameBias ranges from 0 to 1 during frame 0 to (touchframe OR beginRotationFrameCount) (depending on which comes first)
    beginFrameBias = std::min(1.0f, (currentAnim->frameNum + 1) / (float)std::min(beginRotationFrameCount, currentAnim->touchFrame + 1));
    if (!allowPreTouchRotationSmuggle) {
      if (currentAnim->frameNum > currentAnim->touchFrame) {
        // end rotation smuggle starts after touch
        endFrameBias = (currentAnim->frameNum - currentAnim->touchFrame) / (float)(currentAnim->anim->GetEffectiveFrameCount() - currentAnim->touchFrame);
      } else {
        // no smuggle before touch
        endFrameBias = 0.0f;
      }
    }
  }
  currentAnim->rotationSmuggleOffset = currentAnim->rotationSmuggle.begin * (1.0f - beginFrameBias) +
                                       currentAnim->rotationSmuggle.end   * endFrameBias;


  // ballretainer should not get out of 16 meter box

  if (match->GetBallRetainer() == player && CastPlayer()->GetFormationEntry().role == e_PlayerRole_GK && (match->IsInSetPiece() == false && match->IsInPlay() == true)) {
    if (match->GetBall()->Predict(0).coords[1] > 20.05f) {
      OffsetPosition(Vector3(0, clamp(20.05f - match->GetBall()->Predict(0).coords[1], -0.5f, 0.5f), 0) * 0.3f);
    }
    if (match->GetBall()->Predict(0).coords[1] < -20.05f) {
      OffsetPosition(Vector3(0, clamp(-20.05f - match->GetBall()->Predict(0).coords[1], -0.5f, 0.5f), 0) * 0.3f);
    }
    if (match->GetBall()->Predict(0).coords[0] * -team->GetSide() > -pitchHalfW + 16.4f) {
      OffsetPosition(Vector3(clamp((-pitchHalfW + 16.4f) - match->GetBall()->Predict(0).coords[0] * -team->GetSide(), -0.5f, 0.5f), 0, 0) * -team->GetSide() * 0.3f);
    }
    if (match->GetBall()->Predict(0).coords[0] * -team->GetSide() < -pitchHalfW + 0.1f) {
      OffsetPosition(Vector3(clamp((-pitchHalfW + 0.1f) - match->GetBall()->Predict(0).coords[0] * -team->GetSide(), -0.5f, 0.5f), 0, 0) * -team->GetSide() * 0.4f);
    }
  }


  // next frame

  animApplyBuffer.frameNum = currentAnim->frameNum;

  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    //printf("size: %i\n", currentAnim->positions.size());
    animApplyBuffer.position = startPos + currentAnim->positions.at(currentAnim->frameNum) + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
    animApplyBuffer.orientation = startAngle + currentAnim->rotationSmuggleOffset;
    animApplyBuffer.noPos = true;
  } else {
    animApplyBuffer.position = startPos + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
    animApplyBuffer.orientation = startAngle;
    animApplyBuffer.noPos = false;
  }
  animApplyBuffer.offsets = offsets;
}

void Humanoid::CalculateGeomOffsets() {
  SetOffset(middle, 0.0, QUATERNION_IDENTITY);
  SetOffset(neck, 0.0, QUATERNION_IDENTITY);
  SetOffset(left_thigh, 0.0, QUATERNION_IDENTITY);
  SetOffset(right_thigh, 0.0, QUATERNION_IDENTITY);
  SetOffset(left_knee, 0.0, QUATERNION_IDENTITY);
  SetOffset(right_knee, 0.0, QUATERNION_IDENTITY);
  SetOffset(left_ankle, 0.0, QUATERNION_IDENTITY);
  SetOffset(right_ankle, 0.0, QUATERNION_IDENTITY);
  SetOffset(left_shoulder, 0.0, QUATERNION_IDENTITY);
  SetOffset(right_shoulder, 0.0, QUATERNION_IDENTITY);
  SetOffset(left_elbow, 0.0, QUATERNION_IDENTITY);
  SetOffset(right_elbow, 0.0, QUATERNION_IDENTITY);
  SetOffset(body, 0.0, QUATERNION_IDENTITY);

  bool adaptLegsToTrueVelocity = true;
  float adaptLegsToTrueVelocity_influence = 0.7f;
  bool adaptBodyToBallPosition = true;
  float adaptBodyToBallPosition_influence = 0.5f;
  bool adaptLegToTouchPos = false;
  float adaptLegToTouchPos_influence = 0.5f;
  bool adaptArmsToOpp = true;
  float adaptArmsToOpp_influence = 0.9f;

  if (match->IsInPlay() && match->GetBallRetainer() != player) {

    if (currentAnim->functionType == e_FunctionType_Movement) {


      // slow down legs when we're going slower than the anim (or speed up if we're going faster)

      if (adaptLegsToTrueVelocity) {
        float actualVelo = spatialState.actualMovement.GetLength();
        float animVelo = spatialState.animMovement.GetLength();
        float veloFactor = (actualVelo + 0.2f) / (animVelo + 0.2f); // avoid div by zero

        float allowFasterFactor = 0.0f;
        if (spatialState.actualMovement.GetLength() > 7.0f) allowFasterFactor += 0.1;
        veloFactor = clamp(veloFactor, 0.1f, 1.0f + allowFasterFactor);

        float bendFactor = 0.8f;

        Quaternion defaultLeftHipOrientation = Quaternion(QUATERNION_IDENTITY);
        defaultLeftHipOrientation.SetAngleAxis(-0.15f * pi * bendFactor, Vector3(1, 0.06, -0.12).GetNormalized());
        Quaternion defaultRightHipOrientation = Quaternion(QUATERNION_IDENTITY);
        defaultRightHipOrientation.SetAngleAxis(-0.15f * pi * bendFactor, Vector3(1, -0.06, 0.12).GetNormalized());
        Quaternion defaultKneeOrientation = Quaternion(QUATERNION_IDENTITY);
        defaultKneeOrientation.SetAngleAxis(0.3f * pi * bendFactor, Vector3(1, 0, 0));
        Quaternion defaultAnkleOrientation = Quaternion(QUATERNION_IDENTITY);
        defaultAnkleOrientation.SetAngleAxis(-0.1f * pi * bendFactor, Vector3(1, 0, 0));

        SetOffset(left_thigh,  (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultLeftHipOrientation);
        SetOffset(right_thigh, (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultRightHipOrientation);
        SetOffset(left_knee,   (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultKneeOrientation);
        SetOffset(right_knee,  (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultKneeOrientation);
        SetOffset(left_ankle,  (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultAnkleOrientation);
        SetOffset(right_ankle, (1.0f - veloFactor) * adaptLegsToTrueVelocity_influence, defaultAnkleOrientation);

      }


      // aim a bit towards ball

      if (adaptBodyToBallPosition && !CastPlayer()->HasPossession()) {
        Vector3 toBall = (currentMentalImage->GetBallPrediction(10).Get2D() - spatialState.position).GetNormalized(spatialState.directionVec);

        radian angle = toBall.GetAngle2D(spatialState.relBodyDirectionVecNonquantized.GetRotated2D(spatialState.angle));//FixAngle(toBall.GetAngle2D() - spatialState.bodyAngle);
        angle = clamp(fabs(angle) - 0.05f * pi, 0.0f, pi) * signSide(angle); // less influence
        float lookAtBallBias = std::pow(1.0f - fabs(angle / pi), 0.3f);
        lookAtBallBias = clamp(lookAtBallBias * adaptBodyToBallPosition_influence, 0.0f, 1.0f);
        float ballDistance = match->GetBall()->Predict(100).Get2D().GetDistance(spatialState.position);
        float ballProximityFalloff = 1.6f;
        if (ballDistance < ballProximityFalloff) { // close ball can be a problem
          lookAtBallBias *= NormalizedClamp(ballDistance, ballProximityFalloff * 0.4f, ballProximityFalloff);
        }
        lookAtBallBias *=
            1.0f -
            std::pow(NormalizedClamp(spatialState.floatVelocity, idleVelocity,
                                     sprintVelocity - 0.5f),
                     2.0f) *
                0.3f;  // less effect on high velo

        //Quaternion middleOrientation; middleOrientation.SetAngleAxis(FixAngle(toBall.GetAngle2D()), Vector3(0, 0, 1));
        Quaternion middleOrientation; middleOrientation.SetAngleAxis(angle, Vector3(0, 0, 1));

/*        // look up (does not work since the changes in amount of (main) effect breaks it)
        float heightFactor = clamp(toBall3D.coords[2] * 1.1f - 0.1f, 0.0f, 1.0f);
        Quaternion rotX; rotX.SetAngleAxis(heightFactor * 3.5f * pi, Vector3(1, 0, 0));
        middleOrientation = middleOrientation * rotX;
*/

        middleOrientation.Normalize();

        // correct for body orientation
        SetOffset(middle, lookAtBallBias * 0.3f, middleOrientation, true);

        // this is an incorrect guesstimation
        Quaternion headOrientation; headOrientation.SetAngleAxis(angle, Vector3(0, 0, 1));
        // correct for body orientation
        headOrientation.Normalize();
        SetOffset(neck, lookAtBallBias * 0.7f, headOrientation, true);
      }


      // use arms to keep opponents away

      if (adaptArmsToOpp) {
        Player *opp = match->GetPlayer( match->GetTeam(abs(team->GetID() - 1))->GetBestPossessionPlayerID() );
        if ((opp->GetPosition() - spatialState.position).GetLength() < 1.4 && opp->GetDirectionVec().GetDotProduct(spatialState.directionVec) > 0.0) {

          Vector3 baseRotVec = Vector3(0, -1, 0).GetRotated2D(spatialState.angle).GetRotated2D(spatialState.relBodyAngle);
          Vector3 oppVec = opp->GetPosition() - spatialState.position;
          radian angle = baseRotVec.GetAngle2D(oppVec.GetNormalized(baseRotVec));

          Quaternion shoulder;
          Quaternion elbow;

          // player is behind us, somewhat to the left or right. hold up arm to keep him back
          if (fabs(angle) > 0.6 * pi && fabs(angle) < 0.85 * pi) {
            if (angle > 0) {

              shoulder.SetAngleAxis(0.4 * pi, Vector3(0, 1, 0));
              elbow.SetAngleAxis(0, Vector3(0, 1, 0));
              SetOffset(right_shoulder, 0.8f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(right_elbow, 0.7f * adaptArmsToOpp_influence, elbow.GetNormalized());

            } else {

              shoulder.SetAngleAxis(-0.4 * pi, Vector3(0, 1, 0));
              elbow.SetAngleAxis(0, Vector3(0, 1, 0));
              SetOffset(left_shoulder, 0.8f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(left_elbow, 0.7f * adaptArmsToOpp_influence, elbow.GetNormalized());

            }

            // bend forwards
            Quaternion middle_q;
            middle_q.SetAngleAxis(0.2f * pi, Vector3(1, 0, 0));
            SetOffset(middle, 0.3f * adaptArmsToOpp_influence, middle_q.GetNormalized());
          }

          // player is next to us, use arm to protect our position
          if (fabs(angle) > 0.2 * pi && fabs(angle) <= 0.6 * pi) {
            if (angle > 0) {

              shoulder.SetAngleAxis(0.4 * pi, Vector3(0, 1, 0));
              elbow.SetAngleAxis(-0.5 * pi, Vector3(1, 0, 0));
              SetOffset(right_shoulder, 0.7f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(right_elbow, 0.8f * adaptArmsToOpp_influence, elbow.GetNormalized());

              /* todo: fix relative body rotation
              Quaternion body; body.SetAngleAxis(-0.3 * pi, Vector3(0, 1, 0));
              Quaternion middle; middle.SetAngleAxis(0.3 * pi, Vector3(0, 1, 0));
              SetOffset("body", 0.8, body);
              SetOffset("middle", 0.8, middle);
              */

            } else {

              shoulder.SetAngleAxis(-0.4 * pi, Vector3(0, 1, 0));
              elbow.SetAngleAxis(-0.5 * pi, Vector3(1, 0, 0));
              SetOffset(left_shoulder, 0.7f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(left_elbow, 0.8f * adaptArmsToOpp_influence, elbow.GetNormalized());

              /*
              Quaternion body; body.SetAngleAxis(0.3 * pi, Vector3(0, 1, 0));
              Quaternion middle; middle.SetAngleAxis(-0.3 * pi, Vector3(0, 1, 0));
              SetOffset("body", 0.8, body);
              SetOffset("middle", 0.8, middle);
              */

            }
          }

          // player is in front of us, pull shirt :p
          if (fabs(angle) < 0.2) {
            if (angle > 0) {

              shoulder.SetAngleAxis(-0.4 * pi, Vector3(1, 0, 0));
              elbow.SetAngleAxis(-0.2 * pi, Vector3(1, 0, 0));
              SetOffset(right_shoulder, 0.6f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(right_elbow, 0.8f * adaptArmsToOpp_influence, elbow.GetNormalized());

            } else {

              shoulder.SetAngleAxis(-0.4 * pi, Vector3(1, 0, 0));
              elbow.SetAngleAxis(-0.2 * pi, Vector3(1, 0, 0));
              SetOffset(left_shoulder, 0.6f * adaptArmsToOpp_influence, shoulder.GetNormalized());
              SetOffset(left_elbow, 0.8f * adaptArmsToOpp_influence, elbow.GetNormalized());

            }
          }
        }

      }

    }

    else if (currentAnim->touchFrame != -1) {

      if (adaptLegToTouchPos) {

        // dynamic legs so we can reach the ball

        int smoothFrames = 0;

        std::string bodypart = currentAnim->anim->GetVariable("touch_bodypart");
        int leftOrRightLeg = 0; // -1 == left, 1 == right
        if (bodypart.find("left_foot") != std::string::npos || bodypart.find("left_leg") != std::string::npos) leftOrRightLeg = -1;
        if (bodypart.find("right_foot") != std::string::npos || bodypart.find("right_leg") != std::string::npos) leftOrRightLeg = 1;

        if (leftOrRightLeg != 0) { // wrong bodypart? don't do anything

          int influenceFrames = 8;
          float frameFactor = curve(NormalizedClamp(influenceFrames - fabs(currentAnim->frameNum - (currentAnim->touchFrame - smoothFrames)), 0.0f, (float)influenceFrames), 0.5f);
          float ignoreDistance = 0.0f; // only start using this effect after this amount of desired- vs. actual ball pos offset
          float neededFactor = 1.0f;

          Vector3 hipJointPos;
          if (leftOrRightLeg == -1) hipJointPos = nodeMap[left_thigh]->GetDerivedPosition();
          else                     hipJointPos = nodeMap[right_thigh]->GetDerivedPosition();

          Quaternion bodyOrientationRel = nodeMap[body]->GetRotation();

          //SetRedDebugPilon(currentAnim->touchPos);
          //SetBlueDebugPilon(hipJointPos);

          Vector3 autoTouchOffsetRel = currentAnim->touchPos - hipJointPos;
          autoTouchOffsetRel.Rotate2D(-spatialState.angle - spatialState.relBodyAngleNonquantized);

          // make this position shift dynamically, making it relative to the body instead of it being a static world position
          Vector3 bodyPosTouch = currentAnim->positions.at(currentAnim->touchFrame - smoothFrames);
          Vector3 bodyPosNow = currentAnim->positions.at(currentAnim->frameNum);
          autoTouchOffsetRel -= (bodyPosTouch - bodyPosNow).GetRotated2D(-spatialState.angle - spatialState.relBodyAngleNonquantized);

          // if ball is further away, stretch more. if close, bend knee and such
          radian bendAngle = (1.0f - NormalizedClamp(autoTouchOffsetRel.GetLength(), 0.5f, 1.0f)) * 0.34f * pi;

          float insideFactor = 0.4f;
          Quaternion hipTwist; hipTwist.SetAngleAxis(0.5f * pi, Vector3(0, 0, -leftOrRightLeg * insideFactor));
          Quaternion defaultHipOrientation; defaultHipOrientation.SetAngleAxis(-0.03f * pi - bendAngle, Vector3(1, 0, 0)); defaultHipOrientation = hipTwist * defaultHipOrientation;
          Quaternion defaultKneeOrientation; defaultKneeOrientation.SetAngleAxis(0.1f * pi + bendAngle * 2.2f, Vector3(1, 0, 0));
          Quaternion defaultAnkleOrientation; defaultAnkleOrientation.SetAngleAxis(0.4f * pi - bendAngle * 1.2f, Vector3(1, 0, 0));

          // calculate the desired forward/backward swing angle of the leg
          // first, convert z, y into y, x coords, so we can use Vector3's 2D functions
          Vector3 zy = Vector3(autoTouchOffsetRel.coords[2], autoTouchOffsetRel.coords[1], 0.0f);
          radian angle_X = zy.GetAngle2D(Vector3(-1, 0, 0));

          // now decide on the sideways sway angle
          Vector3 zx = Vector3(autoTouchOffsetRel.coords[2], autoTouchOffsetRel.coords[0], 0.0f);
          radian angle_Y = zx.GetAngle2D(Vector3(-1, 0, 0));

          // create rotation quaternions
          Quaternion sway_X; sway_X.SetAngleAxis(angle_X, Vector3(-1,  0, 0));
          Quaternion sway_Y; sway_Y.SetAngleAxis(angle_Y, Vector3( 0,  1, 0));

          // use em!
          defaultHipOrientation = sway_X * defaultHipOrientation;
          defaultHipOrientation.Normalize();
          defaultHipOrientation = sway_Y * defaultHipOrientation;
          defaultHipOrientation.Normalize();

          // overcorrect Z so we can rotate with the inverse of the body quaternion afterwards
          Quaternion quatZ; quatZ.SetAngleAxis(-spatialState.angle - spatialState.relBodyAngleNonquantized, Vector3(0, 0, -1));
          defaultHipOrientation = quatZ * defaultHipOrientation;
          defaultHipOrientation.Normalize();

          // finally, correct for body movements
          defaultHipOrientation = bodyOrientationRel.GetInverse() * defaultHipOrientation;
          defaultHipOrientation.Normalize();




          if (leftOrRightLeg == -1) {
            SetOffset(left_thigh,  frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultHipOrientation);
            SetOffset(left_knee,   frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultKneeOrientation);
            SetOffset(left_ankle,  frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultAnkleOrientation);
          } else {
            SetOffset(right_thigh, frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultHipOrientation);
            SetOffset(right_knee,  frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultKneeOrientation);
            SetOffset(right_ankle, frameFactor * neededFactor * adaptLegToTouchPos_influence, defaultAnkleOrientation);
          }

        }

      }

    }

  }

}

void Humanoid::SelectRetainAnim() {
  CrudeSelectionQuery query;
  query.byFunctionType = true;
  query.functionType = e_FunctionType_Movement;
  query.byIncomingVelocity = true;
  query.incomingVelocity = e_Velocity_Idle;
  query.byOutgoingVelocity = true;
  query.outgoingVelocity = e_Velocity_Idle;
  query.properties.set("incoming_retain_state", "right_elbow");
  query.properties.set("outgoing_retain_state", "right_elbow");

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);

  assert(dataSet.size() != 0);

  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareMovementSimilarity, this, _1, _2));

  startAngle = FixAngle((Vector3(0) - startPos).GetAngle2D());//0.5 * pi; (facing right)

  currentAnim->positions.clear();
  currentAnim->anim = anims->GetAnim(*dataSet.begin());
  currentAnim->id = *dataSet.begin();
  currentAnim->frameNum = 0;
  currentAnim->touchFrame = -1;
  currentAnim->fullActionSmuggle = Vector3(0);
  currentAnim->actionSmuggle = Vector3(0);
  currentAnim->actionSmuggleOffset = Vector3(0);
  currentAnim->actionSmuggleSustain = Vector3(0);
  currentAnim->actionSmuggleSustainOffset = Vector3(0);
  currentAnim->movementSmuggle = Vector3(0);
  currentAnim->movementSmuggleOffset = Vector3(0);
  currentAnim->rotationSmuggle.begin = 0;
  currentAnim->rotationSmuggle.end = 0;
  currentAnim->rotationSmuggleOffset = 0;
  currentAnim->functionType = e_FunctionType_Movement;

  animApplyBuffer.anim = currentAnim->anim;
  animApplyBuffer.smooth = false;
  animApplyBuffer.position = startPos;
  animApplyBuffer.orientation = startAngle;
  buf_animApplyBuffer = animApplyBuffer;

  match->SetBallRetainer(CastPlayer());
}

void Humanoid::ResetSituation(const Vector3 &focusPos) {
  HumanoidBase::ResetSituation(focusPos);
  //printf("humanoid reset\n");
}

bool Humanoid::SelectAnim(const PlayerCommand &command, e_InterruptAnim localInterruptAnim, bool preferPassAndShot) { // returns false on no applicable anim found
  assert(command.desiredDirection.coords[2] == 0.0f);


  // optimizations

  if (command.desiredFunctionType != e_FunctionType_Movement &&
      command.desiredFunctionType != e_FunctionType_Trip &&
      command.desiredFunctionType != e_FunctionType_Special &&
      command.desiredFunctionType != e_FunctionType_Sliding) {
    if ((currentMentalImage->GetBallPrediction(200).Get2D() - spatialState.position).GetLength() > ballDistanceOptimizeThreshold) {
      return false;
    }
    if ((currentMentalImage->GetBallPrediction(defaultTouchOffset_ms).Get2D() - spatialState.position).GetLength() > 2.0f &&
    //    match->GetBall()->GetMovement().GetNormalized(0).GetDotProduct(player->GetMovement().GetNormalizedMax(1.0f)) < 0) { // ball and player going the other way
        (currentMentalImage->GetBallPrediction(defaultTouchOffset_ms).Get2D() - (player->GetPosition() + player->GetMovement() * defaultTouchOffset_ms * 0.001)).GetLength() >
        (currentMentalImage->GetBallPrediction(0).Get2D()                     - (player->GetPosition())).GetLength()) { // ball moving away from player
      return false;
    }
  }

  // this stops these anims from happening when opp has touched the ball, deflecting the ball too far away
  if (command.desiredFunctionType != e_FunctionType_Movement &&
      command.desiredFunctionType != e_FunctionType_Trip &&
      command.desiredFunctionType != e_FunctionType_Special &&
      command.desiredFunctionType != e_FunctionType_Sliding &&
      command.desiredFunctionType != e_FunctionType_Deflect && match->GetBallRetainer() != player) {
    if ((currentMentalImage->GetBallPrediction(1000) - match->GetBall()->Predict(1000)).GetLength() > 2.0f) return false;
  }

  // /optimizations


  if (localInterruptAnim == e_InterruptAnim_ReQueue) {

    float focusDistance = (match->GetDesignatedPossessionPlayer()->GetPosition() - spatialState.position).GetLength();

    if (currentAnim->functionType != e_FunctionType_Movement && command.desiredFunctionType == e_FunctionType_Movement) return false;
    if (currentAnim->functionType == e_FunctionType_Movement && command.desiredFunctionType == e_FunctionType_Movement && (CastPlayer()->HasPossession()/* || team->GetTeamPossessionAmount() >= 1.0f*/ || focusDistance > 12.0f)) return false;
    if (currentAnim->functionType == e_FunctionType_Movement && command.desiredFunctionType == e_FunctionType_Movement && currentAnim->frameNum + minRemainingMovementReQueueFrames > currentAnim->anim->GetEffectiveFrameCount()) return false;
    if (currentAnim->functionType == e_FunctionType_Movement && command.desiredFunctionType == e_FunctionType_Movement && (!allowMovementReQueue || reQueueDelayFrames > 0)) return false;
    if (currentAnim->functionType == e_FunctionType_BallControl && command.desiredFunctionType == e_FunctionType_BallControl && (!allowBallControlReQueue || currentAnim->frameNum > maxBallControlReQueueFrame || reQueueDelayFrames > 0)) return false;
    if (currentAnim->functionType == e_FunctionType_BallControl && command.desiredFunctionType == e_FunctionType_Trap) return false;
    if (currentAnim->functionType == e_FunctionType_Trap && command.desiredFunctionType == e_FunctionType_Trap && (!allowTrapReQueue || currentAnim->frameNum + minRemainingTrapReQueueFrames > currentAnim->touchFrame || reQueueDelayFrames > 0)) return false;
    if (currentAnim->functionType == e_FunctionType_Trap && command.desiredFunctionType == e_FunctionType_BallControl && (!allowTrapReQueue || currentAnim->frameNum + minRemainingTrapReQueueFrames > currentAnim->touchFrame || reQueueDelayFrames > 0)) return false;

    // too similar to what we are already trying to accomplish
    if (currentAnim->originatingCommand.desiredFunctionType == command.desiredFunctionType &&
        ((currentAnim->originatingCommand.desiredDirection * currentAnim->originatingCommand.desiredVelocityFloat) - (command.desiredDirection * command.desiredVelocityFloat)).GetLength() < 1.5f) {
      return false;
    }

    // requeue not needed?
    if ((currentAnim->functionType == e_FunctionType_Movement && command.desiredFunctionType == e_FunctionType_Movement) ||
        (currentAnim->functionType == e_FunctionType_BallControl && command.desiredFunctionType == e_FunctionType_BallControl) ||
        (currentAnim->functionType == e_FunctionType_Trap && command.desiredFunctionType == e_FunctionType_BallControl) ||
        (currentAnim->functionType == e_FunctionType_Trap && command.desiredFunctionType == e_FunctionType_Trap)) {

      // current change in momentum
      Vector3 plannedMomentumChange = currentAnim->outgoingMovement - currentAnim->incomingMovement;
      Vector3 desiredMomentumChange = (command.desiredDirection * command.desiredVelocityFloat) - spatialState.movement;

      if ((desiredMomentumChange.GetDotProduct(plannedMomentumChange) > 0.0f && desiredMomentumChange.GetDistance(plannedMomentumChange) < 4.0f) ||
          desiredMomentumChange.GetDotProduct(plannedMomentumChange) > 0.8f || desiredMomentumChange.GetDistance(plannedMomentumChange) < 2.0f) {
        return false;
      }

    }

    // don't requeue movement to ballcontrol halfway movement anims, unless there's a serious change of movement desired
    if ((currentAnim->functionType == e_FunctionType_Movement) && command.desiredFunctionType == e_FunctionType_BallControl && (match->GetActualTime_ms() - CastPlayer()->GetLastTouchTime_ms() < 600 && CastPlayer()->GetLastTouchType() == e_TouchType_Intentional_Kicked) && CastPlayer()->HasPossession()) {// && !CastPlayer()->AllowLastDitch()) {
      float desiredMovementChange = (spatialState.movement - (command.desiredDirection * command.desiredVelocityFloat)).GetLength();
      if (desiredMovementChange < 1.0f) return false;
    }

  }

  if (localInterruptAnim != e_InterruptAnim_ReQueue || currentAnim->frameNum > 12) CalculateFactualSpatialState();

  assert(command.desiredLookAt.coords[2] == 0.0f);


  // CREATE A CRUDE SET OF POTENTIAL ANIMATIONS

  CrudeSelectionQuery query;

  query.byFunctionType = true;
  query.functionType = command.desiredFunctionType;

  query.byFoot = false;
  query.foot = spatialState.foot == e_Foot_Left ? e_Foot_Right : e_Foot_Left;

  // query.heedForcedFoot = true;
  // query.strongFoot = e_Foot_Right;

  // hax: long pass uses same anims as short pass
  if (query.functionType == e_FunctionType_LongPass) query.functionType = e_FunctionType_ShortPass;

  if (command.touchInfo.desiredPower != 0.0f) {
    query.byOutgoingBallDirection = true;
    query.outgoingBallDirection = command.touchInfo.desiredDirection.GetRotated2D(-spatialState.angle);
  }

  query.byIncomingVelocity = true;
  query.incomingVelocity = spatialState.enumVelocity;

  if (query.functionType != e_FunctionType_Movement && query.incomingVelocity == e_Velocity_Dribble) query.incomingVelocity = e_Velocity_Walk;

  query.incomingVelocity_Strict = false;
  if (query.functionType != e_FunctionType_Movement && query.functionType != e_FunctionType_BallControl) {
    query.incomingVelocity_ForceLinearity = false;
    if (query.functionType != e_FunctionType_Deflect) {
      query.incomingVelocity_NoDribbleToSprint = true;
      if (query.functionType != e_FunctionType_ShortPass && query.functionType != e_FunctionType_LongPass && query.functionType != e_FunctionType_HighPass && query.functionType != e_FunctionType_Shot) {
        query.incomingVelocity_ForceLinearity = true;
        query.incomingVelocity_NoDribbleToIdle = true;
      } else { // passes and such
        query.incomingVelocity_ForceLinearity = false;
        query.incomingVelocity_NoDribbleToIdle = false;
      }
    } else { // deflect
      query.incomingVelocity_NoDribbleToSprint = false;
      query.incomingVelocity_NoDribbleToIdle = false;
    }
  } else {
    query.incomingVelocity_Strict = true;
  }

  query.byIncomingBodyDirection = true;

  query.incomingBodyDirection = spatialState.relBodyDirectionVec;
  if (query.functionType != e_FunctionType_Movement) {
    query.incomingBodyDirection_Strict = false;
    if (query.functionType != e_FunctionType_Deflect) {
      if (query.functionType != e_FunctionType_BallControl) {
        query.incomingBodyDirection_ForceLinearity = true;
      } else {
        query.incomingBodyDirection_ForceLinearity = false; // new, we want to be able to use ballcontrol anims as trap more often to stop ball from rolling past us
      }
    } else { // deflect
      query.incomingBodyDirection_ForceLinearity = false;
    }
  } else {
    query.incomingBodyDirection_Strict = true;
  }

  query.bySide = false;
  if (command.useDesiredLookAt && currentAnim->anim->GetVariableCache().outgoing_special_state().empty() && match->GetBallRetainer() != CastPlayer()) {
    Vector3 playerLookAtVec = (command.desiredLookAt - spatialState.position).GetNormalized(spatialState.directionVec);
    query.lookAtVecRel = playerLookAtVec.GetRotated2D(-spatialState.angle);
    query.bySide = true;
  }

  if (command.onlyDeflectAnimsThatPickupBall == true) {
    query.byPickupBall = true;
    query.pickupBall = true;
  }

  if (command.desiredFunctionType == e_FunctionType_Trap ||
      command.desiredFunctionType == e_FunctionType_Interfere ||
      command.desiredFunctionType == e_FunctionType_Deflect) {
    query.byIncomingBallDirection = true;
    query.incomingBallDirection = (currentMentalImage->GetBallPrediction(180) - currentMentalImage->GetBallPrediction(120)).GetRotated2D(-spatialState.angle).GetNormalized(Vector3(0));
  }

  if (CastPlayer()->AllowLastDitch()) {
    query.allowLastDitchAnims = true;
  } else {
    query.allowLastDitchAnims = false;
  }

  if (command.desiredFunctionType == e_FunctionType_Trip) {
    query.byTripType = true;
    query.tripType = command.tripType;
  }

  query.properties.set("incoming_special_state", currentAnim->anim->GetVariableCache().outgoing_special_state());
  if (match->GetBallRetainer() == player) query.properties.set("incoming_retain_state", currentAnim->anim->GetVariable("outgoing_retain_state"));
  if (command.useSpecialVar1) query.properties.set_specialvar1(command.specialVar1);
  if (command.useSpecialVar2) query.properties.set_specialvar2(command.specialVar2);

  if (!currentAnim->anim->GetVariableCache().outgoing_special_state().empty()) query.incomingVelocity = e_Velocity_Idle; // standing up anims always start out idle

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);
  if (dataSet.size() == 0) {
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      dataSet.push_back(GetIdleMovementAnimID()); // do with idle anim (should not happen too often, only after weird bumps when there's for example a need for a sprint anim at an impossible body angle, after a trip of whatever)
    } else return false;
  }

  //printf("dataset size after crude selection: %i\n", dataSet.size());

  if (command.useDesiredMovement) {

    Vector3 relDesiredDirection = command.desiredDirection.GetRotated2D(-spatialState.angle);
    float desiredAnimationVelocityFloat = command.desiredVelocityFloat;

    // // special case: 90 degrees cornering is tough with weak foot [hax version]
    // radian angle = command.desiredDirection.GetAngle2D(spatialState.directionVec);
    // if (command.desiredFunctionType == e_FunctionType_BallControl && adaptedDesiredVelocityFloat > dribbleVelocity && spatialState.floatVelocity > dribbleVelocity &&
    //     fabs(angle) > 0.3 * pi && fabs(angle) < 0.8 * pi && angle > 0) adaptedDesiredVelocityFloat = idleVelocity;

    SetMovementSimilarityPredicate(relDesiredDirection, FloatToEnumVelocity(desiredAnimationVelocityFloat));
    SetBodyDirectionSimilarityPredicate(command.desiredLookAt);

    if (command.desiredFunctionType == e_FunctionType_Movement) {
      // this makes body dirs lots better, at the cost of less correct movement anims. todo: maybe it's an idea to actually use this, and then allow more deviation in the physics code to fix the incorrect movement.
      //if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command, false, 0.5f * pi);

      // now strict-select from the remainder
      _KeepBestDirectionAnims(dataSet, command, true);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command, true);
    }

    else if (command.desiredFunctionType == e_FunctionType_BallControl) {
      bool strict = true;
      if (CastPlayer()->AllowLastDitch()) {
        strict = false;
      }
      float allowedBaseAngle = 0.0f * pi;
      int allowedVelocitySteps = 0; // last ditch anims are always allowed > 0 velocity steps, as long as strict is false
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command, strict, allowedBaseAngle); // needed for idle outgoing velo, probably
      _KeepBestDirectionAnims(dataSet, command, strict, allowedBaseAngle, allowedVelocitySteps);
    }

    else if (command.desiredFunctionType == e_FunctionType_Trap) {

      /*
      bool haste = false;
      // doesn't work well for long anims: they may be disregarded early on as panicky, yet then missed when we are starting to panic because they have a long 'fadein'
      if (currentAnim->functionType != e_FunctionType_Trap) {
        float hasteFactor = GetHasteFactor(false);
        if (hasteFactor > 0.5f) hasteFactor = true;
        std::string hasteString = hasteFactor ? "YES! PANIC!" : "nah relax bro";
      }*/


      bool strict = true;
      if (CastPlayer()->AllowLastDitch(false) || _HighOrBouncyBall()) strict = false;
      float allowedBaseAngle = 0.3f * pi;
      int allowedVelocitySteps = 2;
      int bestBallControlQuadrantID = -1;
      _KeepBestDirectionAnims(dataSet, command, strict, allowedBaseAngle, allowedVelocitySteps, bestBallControlQuadrantID);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command, strict, allowedBaseAngle);

      // when too unlike command's desired movement, just don't go for it (and hope for another ballcontrol/trap anim to save us later on)
      if (!_HighOrBouncyBall() && query.allowLastDitchAnims == false) {
        assert(!dataSet.empty());
        Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
        Animation *bestWeGot = anims->GetAnim(*dataSet.begin());
        Vector3 bestWeGotMovement = bestWeGot->GetOutgoingMovement().GetRotated2D(spatialState.angle);
        float currentDesiredDot = command.desiredDirection.GetDotProduct(spatialState.directionVec);

        bool allowAnim = true;

        radian angleDiff = fabs(bestWeGot->GetOutgoingDirection().GetRotated2D(spatialState.angle).GetAngle2D(command.desiredDirection));
        if (angleDiff > 0.375f * pi) { // so we accept at least either 000 or 135 deg anims, which are two common anim types that are often available
          allowAnim = false;
        }

        Vector3 desiredBestDiff = bestWeGotMovement - desiredMovement;
        if ( (desiredBestDiff.GetLength() > walkVelocity + 0.5f && currentDesiredDot > 0.0f) ||
             (desiredBestDiff.GetLength() > sprintVelocity + 0.5f && currentDesiredDot <= 0.0f)    ) { // + margin
          allowAnim = false;
        }

        if (!allowAnim) {
          return false;
        }

      }

    }

    else if (command.desiredFunctionType == e_FunctionType_Interfere) {

      bool strict = false;
      float allowedAngle = 0.3f * pi;
      int allowedVelocitySteps = 1;
      if (command.strictMovement == e_StrictMovement_True) strict = true;

      _KeepBestDirectionAnims(dataSet, command, strict, allowedAngle, allowedVelocitySteps);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command, strict, allowedAngle);
    }

  }

  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::ComparePriorityVariable, this, _1, _2));

  int desiredIdleLevel = 0;
  if (!match->IsInPlay()) desiredIdleLevel = 2;
  if (match->IsInSetPiece()) desiredIdleLevel = 1;
  else if ((match->GetBall()->Predict(200) - spatialState.position).GetLength() > 16.0f) desiredIdleLevel = 1;
  SetIdlePredicate(desiredIdleLevel);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIdleVariable, this, _1, _2));

  SetFootSimilarityPredicate(spatialState.foot);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareFootSimilarity, this, _1, _2));

  if (command.desiredFunctionType != e_FunctionType_BallControl) {
    SetIncomingBodyDirectionSimilarityPredicate(spatialState.relBodyDirectionVec);
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingBodyDirectionSimilarity, this, _1, _2));
  }

  // moved down
  SetIncomingVelocitySimilarityPredicate(spatialState.enumVelocity);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingVelocitySimilarity, this, _1, _2));

  // OLD METHOD
  if (command.useDesiredTripDirection) {
    Vector3 relDesiredTripDirection = command.desiredTripDirection.GetRotated2D(-spatialState.angle);
    SetTripDirectionSimilarityPredicate(relDesiredTripDirection);
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareTripDirectionSimilarity, this, _1, _2));
  }

  // OLD METHOD
  if (command.desiredFunctionType != e_FunctionType_Movement) {
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
  }

  if (command.desiredFunctionType == e_FunctionType_Deflect) {
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareCatchOrDeflect, this, _1, _2));
  }

  int selectedAnimID = -1;
  std::vector<Vector3> positions_tmp;
  int touchFrame_tmp = -1;
  float radiusOffset_tmp = 0.0f;
  Vector3 touchPos_tmp;
  Vector3 fullActionSmuggle_tmp;
  Vector3 actionSmuggle_tmp;
  radian rotationSmuggle_tmp = 0;

  if (dataSet.size() == 0 && command.desiredFunctionType == e_FunctionType_Movement) {
    dataSet.push_back(GetIdleMovementAnimID()); // do with idle anim (should not happen too often, only after weird bumps when there's for example a need for a sprint anim at an impossible body angle, after a trip of whatever)
  }

  Vector3 desiredBodyDirectionRel = Vector3(0, -1, 0);
  if (command.useDesiredLookAt) desiredBodyDirectionRel = (command.desiredLookAt - (spatialState.position + spatialState.movement * 0.1f)).GetNormalized(Vector3(0, -1, 0)).GetRotated2D(-spatialState.angle);

  if (command.desiredFunctionType == e_FunctionType_Movement ||
      command.desiredFunctionType == e_FunctionType_Trip ||
      command.desiredFunctionType == e_FunctionType_Special) {

    selectedAnimID = *dataSet.begin();
    Animation *nextAnim = anims->GetAnim(selectedAnimID);
    Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
    assert(desiredMovement.coords[2] == 0.0f);
    Vector3 physicsVector = CalculatePhysicsVector(nextAnim, command.useDesiredMovement, desiredMovement, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, rotationSmuggle_tmp);
  }
  else if (command.desiredFunctionType == e_FunctionType_BallControl) {
    if (NeedTouch(*dataSet.begin(), command)) {
      selectedAnimID = GetBestCheatableAnimID(dataSet, command.useDesiredMovement, command.desiredDirection, command.desiredVelocityFloat, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, touchFrame_tmp, radiusOffset_tmp, touchPos_tmp, fullActionSmuggle_tmp, actionSmuggle_tmp, rotationSmuggle_tmp, localInterruptAnim, preferPassAndShot);
    }
  }
  else if (command.desiredFunctionType == e_FunctionType_Trap ||
           command.desiredFunctionType == e_FunctionType_Interfere ||
           command.desiredFunctionType == e_FunctionType_Deflect) {
    selectedAnimID = GetBestCheatableAnimID(dataSet, command.useDesiredMovement, command.desiredDirection, command.desiredVelocityFloat, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, touchFrame_tmp, radiusOffset_tmp, touchPos_tmp, fullActionSmuggle_tmp, actionSmuggle_tmp, rotationSmuggle_tmp, localInterruptAnim, preferPassAndShot);
  }
  else if (command.desiredFunctionType == e_FunctionType_ShortPass ||
           command.desiredFunctionType == e_FunctionType_LongPass ||
           command.desiredFunctionType == e_FunctionType_HighPass ||
           command.desiredFunctionType == e_FunctionType_Shot) {

    selectedAnimID = GetBestCheatableAnimID(dataSet, command.useDesiredMovement, command.desiredDirection, command.desiredVelocityFloat, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, touchFrame_tmp, radiusOffset_tmp, touchPos_tmp, fullActionSmuggle_tmp, actionSmuggle_tmp, rotationSmuggle_tmp, localInterruptAnim);
  }
  else if (command.desiredFunctionType == e_FunctionType_Sliding) {
    selectedAnimID = GetBestCheatableAnimID(dataSet, command.useDesiredMovement, command.desiredDirection, command.desiredVelocityFloat, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, touchFrame_tmp, radiusOffset_tmp, touchPos_tmp, fullActionSmuggle_tmp, actionSmuggle_tmp, rotationSmuggle_tmp, localInterruptAnim);
    if (selectedAnimID == -1) {
      if (dataSet.size() > 0) {
        selectedAnimID = *dataSet.begin();
        Animation *nextAnim = anims->GetAnim(selectedAnimID);
        Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
        assert(desiredMovement.coords[2] == 0.0f);
        Vector3 physicsVector = CalculatePhysicsVector(nextAnim, command.useDesiredMovement, desiredMovement, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, rotationSmuggle_tmp);
      }
    }
  }


  // check if we really want to requeue; if the anim we dug up is actually better than the current

  if (localInterruptAnim == e_InterruptAnim_ReQueue && selectedAnimID != -1 && currentAnim->positions.size() > 1 && positions_tmp.size() > 1) {

    // don't requeue to same quadrant
    if (currentAnim->functionType == command.desiredFunctionType &&

        ((FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) != e_Velocity_Idle &&
          currentAnim->anim->GetVariableCache().quadrant_id() == anims->GetAnim(selectedAnimID)->GetVariableCache().quadrant_id())
          ||
         ((FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle && FloatToEnumVelocity(anims->GetAnim(selectedAnimID)->GetOutgoingVelocity()) == e_Velocity_Idle) &&
          fabs((ForceIntoPreferredDirectionAngle(currentAnim->anim->GetOutgoingAngle()) - ForceIntoPreferredDirectionAngle(anims->GetAnim(selectedAnimID)->GetOutgoingAngle()))) < 0.06f * pi))
       ) {

      selectedAnimID = -1;
    }
  }


  // make it so

  if (selectedAnimID != -1) {
    *previousAnim = *currentAnim;

    currentAnim->anim = anims->GetAnim(selectedAnimID);
    currentAnim->id = selectedAnimID;
    currentAnim->functionType = command.desiredFunctionType;//StringToFunctionType(currentAnim->anim->GetVariable("type"));
    currentAnim->frameNum = 0;
    currentAnim->touchFrame = touchFrame_tmp;
    currentAnim->originatingInterrupt = localInterruptAnim;
    currentAnim->radiusOffset = radiusOffset_tmp;
    currentAnim->touchPos = touchPos_tmp;
    currentAnim->rotationSmuggle.begin = clamp(ModulateIntoRange(-pi, pi, spatialState.relBodyAngleNonquantized - currentAnim->anim->GetIncomingBodyAngle()) * bodyRotationSmoothingFactor, -bodyRotationSmoothingMaxAngle * (currentAnim->functionType == e_FunctionType_Movement ? 1.0f : 0.5f), bodyRotationSmoothingMaxAngle * (currentAnim->functionType == e_FunctionType_Movement ? 1.0f : 0.5f));
    currentAnim->rotationSmuggle.end = rotationSmuggle_tmp;
    currentAnim->rotationSmuggleOffset = 0;
    currentAnim->fullActionSmuggle = fullActionSmuggle_tmp;
    currentAnim->actionSmuggle = actionSmuggle_tmp;
    currentAnim->actionSmuggleOffset = Vector3(0);
    currentAnim->actionSmuggleSustain = Vector3(0); // calculated below
    currentAnim->actionSmuggleSustainOffset = Vector3(0);
    currentAnim->movementSmuggle = Vector3(0); // needs to be reset here, else the previous calc is used in upcoming 'calculatemovementsmuggle'
    currentAnim->movementSmuggleOffset = Vector3(0);
    currentAnim->incomingMovement = spatialState.movement;
    currentAnim->outgoingMovement = CalculateOutgoingMovement(positions_tmp);
    currentAnim->positions.clear();
    currentAnim->positions.assign(positions_tmp.begin(), positions_tmp.end());
    currentAnim->positionOffset = Vector3(0);
    currentAnim->originatingCommand = command;
    currentAnim->movementSmuggle = CalculateMovementSmuggle(command.desiredDirection, command.desiredVelocityFloat);
    currentAnim->movementSmuggleOffset = Vector3(0);
    return true;
  }

  return false;
}

bool Humanoid::NeedTouch(int animID, const PlayerCommand &command) {

  // when idle (and desiredvelo is idle as well), don't want to touch the ball every frame

  Animation *anim = anims->GetAnim(animID);

  if (FloatToEnumVelocity(anim->GetOutgoingVelocity() != e_Velocity_Idle)) return true;
  if (command.desiredVelocityFloat > idleDribbleSwitch) return true;
  if (fabs(match->GetBall()->GetMovement().GetLength()) > 2.0f) return true;

  Vector3 animMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle) * 0.3f + spatialState.movement * 0.7f;

  float animVelo = animMovement.GetLength();
  animMovement.Normalize(spatialState.directionVec);
  animMovement *= spatialState.movement.GetLength() * 0.8f + animVelo * 0.2f;

  Vector3 ballMovement = (currentMentalImage->GetBallPrediction(250).Get2D() - currentMentalImage->GetBallPrediction(240).Get2D()) * 100;

  if (fabs(anim->GetOutgoingAngle()) > 0.125f * pi) return true;

  float distanceDeviation = (animMovement - ballMovement).GetLength();
  if (distanceDeviation >= 2.0) return true;

  float velocityDeviation = animMovement.GetLength() - ballMovement.GetLength(); // negative == ball is faster
  if (velocityDeviation < -1.4 || velocityDeviation >= 0.7) return true;

  if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle) {
    float angleDeviation = animMovement.GetNormalized(spatialState.directionVec).GetDotProduct(ballMovement.GetNormalized(spatialState.directionVec));
    if (angleDeviation < 0.975) return true;
  }

  return false;
}

float Humanoid::GetBodyBallDistanceAdvantage(const Animation *anim, e_FunctionType functionType, const Vector3 &animTouchMovement, const Vector3 &touchMovement, const Vector3 &incomingMovement, const Vector3 &outgoingMovement, radian outgoingAngle, /*const Vector3 &animBallToBall2D, */const Vector3 &bodyPos, const Vector3 &FFO, const Vector3 &animBallPos2D, const Vector3 &actualBallPos2D, const Vector3 &ballMovement2D, float radiusFactor, float radiusCheatDistance, float decayPow, bool debug) const {

  assert(touchMovement.coords[2] == 0.0f);
  assert(bodyPos.coords[2] == 0.0f);

  // super simplistic debug version
  //if (animBallPos2D.GetDistance(actualBallPos2D) < radiusFactor * 1.0) return 1.0f; else return 0.0f;

  float touchVelocity = touchMovement.GetLength();
  float animTouchVelocity = animTouchMovement.GetLength();
  float highestTouchVelocity = (animTouchVelocity > touchVelocity) ? animTouchVelocity : touchVelocity;

  float incomingVelocity = incomingMovement.GetLength();
  float outgoingVelocity = outgoingMovement.GetLength();
  float averageInOutVelocity = (incomingMovement + outgoingMovement).GetLength() * 0.5f;
  float highestInOutVelocity = (incomingVelocity > outgoingVelocity) ? incomingVelocity : outgoingVelocity;

  float highestVelocity = (highestTouchVelocity > highestInOutVelocity) ? highestTouchVelocity : highestInOutVelocity;

  float velocityChange = outgoingVelocity - incomingVelocity;
  float velocityChange_mps = velocityChange / (anim->GetFrameCount() * 0.01f);


  float bodyAnimBallBonus = 1.0f - curve(NormalizedClamp(((bodyPos + FFO.GetNormalized(0) * 0.1f) - animBallPos2D).GetLength(), 0.0f, 0.7f), 0.7f); // less FFO feels better
  float bodyActualBallBonus = 1.0f - curve(NormalizedClamp(((bodyPos + FFO.GetNormalized(0) * 0.1f) - actualBallPos2D).GetLength(), 0.0f, 0.7f), 0.4f);
  float velocityBonus = 1.0f - NormalizedClamp(averageInOutVelocity, idleVelocity, sprintVelocity);
  float velocityChangeBonus = 1.0f - NormalizedClamp(velocityChange_mps / 20.0f, -1.0f, 1.0f);


  float radius = radiusFactor;
  radius *= 1.0f +
            1.0f * bodyAnimBallBonus +
            0.6f * bodyActualBallBonus +
            0.8f * bodyAnimBallBonus * bodyActualBallBonus +
            0.0f * velocityChangeBonus +
            1.0f * velocityBonus;

  // more cheat for faster balls. rationale: with the current system, points in time are used as 'check if legal ball touch', while in reality, this is a continuum.
  // in other words, faster balls have an unrealistic disadvantage in this system. this tries to restore this balance. however; this widens the ball touch area, while
  // a better solution would be to actually somehow increase the number of checked points (or rather, to just check against the continuum instead of against points)
  //radius *= 0.5f + clamp((ballMovement2D - touchMovement).GetLength() * 0.1f, 0.0f, 1.5f);

  float effectiveRadiusCheatDistance = radiusCheatDistance;

  Vector3 outgoingDirection;
  if (FloatToEnumVelocity(outgoingMovement.GetLength()) == e_Velocity_Idle) {
    outgoingDirection = Vector3(0, -1, 0).GetRotated2D(outgoingAngle);
  } else {
    outgoingDirection = outgoingMovement.GetNormalized();
  }


  Vector3 behindVectorUnscaled = -(incomingMovement * 0.1f + touchMovement * 0.2f + outgoingMovement * 0.7f);
  Vector3 behindVector =
      behindVectorUnscaled.GetNormalized(0) *
      std::pow(
          NormalizedClamp(behindVectorUnscaled.GetLength(), 0, sprintVelocity),
          0.5f);

  // cornering with a more centered behindvector for 'richer' range
  float dot = Vector3(0, -1, 0).GetDotProduct(outgoingDirection);
  dot = 0.5f + clamp(dot * 0.5f + 0.5f, 0.0f, 1.0f) * 0.5f;
  behindVector *= dot;

  Vector3 animToActualBall = actualBallPos2D - animBallPos2D;

  bool deformArea = true;
  if (deformArea) {
    Vector3 straightAngleVectorUnscaled = incomingMovement;
    Vector3 straightAngleVector = straightAngleVectorUnscaled.GetNormalized(outgoingDirection);
    radian toStraightAngle = Vector3(0, -1, 0).GetAngle2D(straightAngleVector);

    // rotate animball to actualball vec into outgoing direction so we can do funky stuff with it
    animToActualBall.Rotate2D(toStraightAngle);

    // cheating to the side (lateral) (as seen from the outgoing direction vector) is harder at high velos (effectively changes cheat circle into ellipse)
    float lateralRadiusFactor =
        0.6f -
        0.3f * std::pow(NormalizedClamp(straightAngleVectorUnscaled.GetLength(),
                                        idleVelocity, sprintVelocity),
                        0.7f);
    animToActualBall.coords[0] /= lateralRadiusFactor; // coords[0] is now the lateral component
    radius *= std::pow(
        1.0f / lateralRadiusFactor,
        0.5f);  // make sure we stick with the same surface area (= pi * r ^ 2,
                // so just take the sqrt of lateralradiusfactor, which is
                // effectively a surface area multiplier)
    /*
    // brick wall: if balls are beyond animtouchpos in outgoingDirection
    territory, cut off at higher speeds and such if (animToActualBall.coords[1]
    < 0.0f) {
      //float brickWallDistanceFactor = 1.0f -
    NormalizedClamp(averageInOutVelocity, 0.0f, sprintVelocity);
      //float brickWallDistance = radiusFactor * brickWallDistanceFactor * 2.0f;
      //animToActualBall.coords[1] *= 1.0f + pow(averageInOutVelocity, 1.5f)
    * radiusFactor * 5.0f; animToActualBall.coords[1] *= 1.0f +
    pow(averageInOutVelocity, 1.5f) * radiusFactor * 0.4f;
    }
    */
    // rotate back and act like nothing happened
    animToActualBall.Rotate2D(-toStraightAngle);
  }


  // do some magic

  Vector3 adaptedActualBallPos2D = animBallPos2D + animToActualBall;

  float radiusCheatBehindBias = 0.6f; // how much the radiuscheat heeds behindvec
  Vector3 behindCenter = animBallPos2D + behindVector * (radius + (effectiveRadiusCheatDistance * radiusCheatBehindBias)) * cheatFactor;

  float result = 1.0f;
  float allowedRadius = (radius + effectiveRadiusCheatDistance) * cheatFactor + cheatDistanceBonus;
  if (adaptedActualBallPos2D.GetDistance(behindCenter) > allowedRadius) { result = 0.0f; stat_GetBodyBallDistanceAdvantage_RadiusDeny++; }

  return result;
}

signed int Humanoid::GetBestCheatableAnimID(const DataSet &sortedDataSet, bool useDesiredMovement, const Vector3 &desiredDirection, float desiredVelocityFloat, bool useDesiredBodyDirection, const Vector3 &desiredBodyDirectionRel, std::vector<Vector3> &positions_ret, int &animTouchFrame_ret, float &radiusOffset_ret, Vector3 &touchPos_ret, Vector3 &fullActionSmuggle_ret, Vector3 &actionSmuggle_ret, radian &rotationSmuggle_ret, e_InterruptAnim localInterruptAnim, bool preferPassAndShot) const {

  // never allow touchanims when someone else is holding the ball in his/her hands
  if (match->GetBallRetainer() != 0 && match->GetBallRetainer() != player) return -1;

  Vector3 incomingMovement = spatialState.movement.GetRotated2D(-spatialState.angle);

  signed int bestAnimID = -1;
  Vector3 bestActionSmuggleVec2D;
  Vector3 bestTouchMovementAbs;

  DataSet::const_iterator iter = sortedDataSet.begin();

  Vector3 desiredMovement = desiredDirection * desiredVelocityFloat;

  float playerHeight = player->GetPlayerData()->GetHeight();

  e_FunctionType functionType = StringToFunctionType(anims->GetAnim(*iter)->GetAnimTypeStr());

  radian rotationSmuggle_ret_tmp = 0;
  radian predictedAngle = 0;
  Vector3 adaptedOutgoingMovement;

  Vector3 dud;

  bool found = false;
  while (iter != sortedDataSet.end() && found == false) {

    Animation *anim = anims->GetAnim(*iter);
    bool isBase = anim->GetVariableCache().baseanim();

    const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(anim);

    Vector3 physicsVector = CalculatePhysicsVector(anim, useDesiredMovement, desiredMovement, useDesiredBodyDirection, desiredBodyDirectionRel, positions_ret, rotationSmuggle_ret_tmp);

    // anim space!
    predictedAngle = anim->GetOutgoingAngle() + rotationSmuggle_ret_tmp;
    predictedAngle = ModulateIntoRange(-pi, pi, predictedAngle);

    // iterate all possible touches of this anim
    int touchNum = 0;
    Vector3 animBallPos;
    int animTouchFrame = 0;

    Quaternion animBodyRot;
    Vector3 animBodyPos;
    Vector3 bodyPos;
    Vector3 prevBodyPos;

    Vector3 outgoingMovement = CalculateOutgoingMovement(positions_ret).GetRotated2D(-spatialState.angle);
    adaptedOutgoingMovement = outgoingMovement; // may be changed into touchMovement later on, when an anim is found

    int frameCount = anim->GetEffectiveFrameCount();

    boost::shared_ptr<FootballAnimationExtension> footballExtension = boost::static_pointer_cast<FootballAnimationExtension>(anim->GetExtension("football"));

    int totalTouches = footballExtension->GetTouchCount();
    int touchIDs[totalTouches];
    int count = 0;

    int defaultTouchFrame = atoi(anim->GetVariable("touchframe").c_str());
    assert(defaultTouchFrame >= 0 && defaultTouchFrame < frameCount);

    // first the middle one down to the first
    for (int i = totalTouches / 2; i > -1; i--) {
      touchIDs[count] = i;
      count++;
    }
    // then the 1-after-middle one and upwards
    for (int i = totalTouches / 2 + 1; i < totalTouches; i++) {
      touchIDs[count] = i;
      count++;
    }

    while (touchNum < totalTouches && found == false) {

      bool exists = footballExtension->GetTouch(touchIDs[touchNum], animBallPos, animTouchFrame);
      assert(exists);

      // out of bounds?
      if (match->GetBallRetainer() != player) {
        Vector3 absBallPos = match->GetBall()->Predict(animTouchFrame * 10);
        if (fabs(absBallPos.coords[0]) > pitchHalfW + lineHalfW + 0.11f ||
            fabs(absBallPos.coords[1]) > pitchHalfH + lineHalfW + 0.11f) {
          touchNum++;
          continue;
        }
      }

      Vector3 touchMovement = CalculateMovementAtFrame(positions_ret, animTouchFrame).GetRotated2D(-spatialState.angle);
      Vector3 animTouchMovement = CalculateMovementAtFrame(origPositionCache, animTouchFrame);// already anim space so no: .GetRotated2D(-spatialState.angle);

      Vector3 ballPos, ballMovement;
      ballPos = currentMentalImage->GetBallPrediction(animTouchFrame * 10);
      ballMovement = (currentMentalImage->GetBallPrediction(animTouchFrame * 10 + 10) - currentMentalImage->GetBallPrediction(animTouchFrame * 10)) * 100.0f;
      ballPos = (ballPos - spatialState.position).GetRotated2D(-spatialState.angle);
      ballMovement = ballMovement.GetRotated2D(-spatialState.angle);

      bodyPos = positions_ret.at(animTouchFrame).GetRotated2D(-spatialState.angle);
      bodyPos.coords[2] = 0;

      anim->GetKeyFrame(BodyPart::player, animTouchFrame, animBodyRot, animBodyPos);

      radian x, y, z;
      animBodyRot.GetAngles(x, y, z);

      float animBallHeight = animBallPos.coords[2];
      if (allowPreTouchRotationSmuggle) {
        animBallPos = (animBallPos - animBodyPos).GetRotated2D(rotationSmuggle_ret_tmp * ((float)animTouchFrame / (float)frameCount)) + positions_ret.at(animTouchFrame).GetRotated2D(-spatialState.angle);
      } else {
        animBallPos = (animBallPos - animBodyPos) + positions_ret.at(animTouchFrame).GetRotated2D(-spatialState.angle);
      }
      animBallPos.coords[2] = animBallHeight * (playerHeight / defaultPlayerHeight);


      // now pick the ballPos from the previous 9ms that is closest to animBallPos. this is to emulate a continuous 'close enough?'-check instead of a 'single moment' check.
      if (useContinuousBallCheck) {
        Line ballLine(ballPos - ballMovement * 0.006f, ballPos + ballMovement * 0.003f);
        float u = clamp(ballLine.GetClosestToPoint(animBallPos), 0.0f, 1.0f);
        ballPos = ballLine.GetVertex(0) + (ballLine.GetVertex(1) - ballLine.GetVertex(0)) * u;
      }

      Vector3 actionSmuggleVec3D = ballPos - animBallPos;
      Vector3 actionSmuggleVec2D = actionSmuggleVec3D.Get2D();


      // ball height

      float ballDistanceZ = fabs(actionSmuggleVec3D.coords[2]);

      ballDistanceZ *= 1.0f - clamp((animBallPos.coords[2] - 0.11) * 0.3f, 0.0f, 0.2f); // higher balls == cheat more Z (else we would have to make 100000000 anims for high balls on different heights)
      ballDistanceZ *= 1.0f - clamp((ballPos.coords[2] - 0.11) * 0.4f, 0.0f, 0.3f);
      // enforce maximum height though
      if (ballPos.coords[2] > 1.8f && ballPos.coords[2] > animBallPos.coords[2] + 0.12f) ballDistanceZ *= 2.0f;
      if (ballPos.coords[2] > 2.6f && ballPos.coords[2] > animBallPos.coords[2] + 0.08f) ballDistanceZ *= 20.0f;
      if (functionType == e_FunctionType_Deflect) ballDistanceZ *= 0.8f;
      if (match->GetBallRetainer() == player) ballDistanceZ = 0.0f;

      if (ballPos.coords[2] < 0.5f && isBase) ballDistanceZ = std::max(ballDistanceZ - 0.15f, 0.0f); // low balls should be doable with ground level anims, doesn't look that bad :P

      if (ballDistanceZ < 0.22f) {

        // default touch can be 'cheated' towards best, has biggest 'radius'
        float touchFrameAwkwardness = NormalizedClamp(abs(defaultTouchFrame - animTouchFrame), 0.0f, 4.0f);
        touchFrameAwkwardness = std::pow(touchFrameAwkwardness, 2.0f) * 0.5f;

        /* todo: check if this is a possibility in some form
              // add this to desired cheatvec to get more of a flowing feeling to it
              Vector3 flowVector = spatialState.movement - match->GetBall()->GetMovement().Get2D();// - anims->GetAnim(*iter)->GetOutgoingMovement().GetRotated2D(spatialState.angle) * 2.0;
              flowVector.Rotate2D(-spatialState.angle);
              actionSmuggleVec2D += (flowVector).GetNormalized(0) * 0.3f;
        */

        float touchFrameFactor = animTouchFrame / 24.0f; // makes it 0.5 around the 'average touchframe' (educated guess average)
        touchFrameFactor = std::pow(
            touchFrameFactor,
            0.7f);  // advantage for anims that touch the ball earlier.
                    // rationale: more cheating, sure, but for a shorter period
        // even steeper falloff after 1.0
        if (touchFrameFactor > 1.0f) touchFrameFactor = 1.0f + ((touchFrameFactor - 1.0f) * 0.5f);

        float decayPow = 1.0f;
        float radiusCheatOffset = 0.0f;
        float radiusFactor = 0.3f * (1.0f - touchFrameAwkwardness);

        if (functionType == e_FunctionType_Deflect) { radiusFactor *= 1.8f; radiusCheatOffset += 0.4f; }

        if (functionType == e_FunctionType_Sliding) { radiusFactor *= 0.2f; radiusCheatOffset = 0.0f; } // prefer sliding without ball touch
        if (functionType == e_FunctionType_Interfere) { radiusFactor *= 1.4f; radiusCheatOffset += 0.2f; }

        if (functionType == e_FunctionType_ShortPass) { radiusFactor *= 1.3f; radiusCheatOffset += 0.15f; }
        if (functionType == e_FunctionType_LongPass) { radiusFactor *= 1.3f; radiusCheatOffset += 0.15f; }
        if (functionType == e_FunctionType_HighPass) { radiusFactor *= 1.3f; radiusCheatOffset += 0.15f; }
        if (functionType == e_FunctionType_Shot) { radiusFactor *= 1.3f; radiusCheatOffset += 0.15f; }

        if ((functionType == e_FunctionType_Trap || functionType == e_FunctionType_BallControl) && preferPassAndShot == true) {
          radiusFactor *= 0.3f;
        }

        radiusCheatOffset *= (1.0f - touchFrameAwkwardness);

        float touchVelo = touchMovement.GetLength();

        Vector3 FFO = GetFrontOfFootOffsetRel(touchVelo, z, ballPos.coords[2]);
        if (FloatToEnumVelocity(touchVelo) == e_Velocity_Idle) {
          //FFO.Rotate2D(z); // always towards y = -1, right? (hmmm not really)
        } else {
          FFO.Rotate2D(FixAngle(touchMovement.GetNormalized(Vector3(0, -1, 0)).GetAngle2D()));
        }
        // just touched ball
        float lastTouchBias = curve(player->GetLastTouchBias(600, match->GetActualTime_ms() + animTouchFrame * 10), 1.0f);
        if (lastTouchBias > 0.0f) {
          float factor = 1.0f - lastTouchBias * 0.97f * (1.0f - player->GetStat(technical_ballcontrol) * 0.1f);
          radiusFactor *= factor;
          radiusCheatOffset *= factor;
        }


        bool debug = false;

        float touchFramedRadiusFactor = radiusFactor * touchFrameFactor;
        float bodyBallDistanceAdvantage = GetBodyBallDistanceAdvantage(anim, functionType, animTouchMovement, touchMovement, incomingMovement, adaptedOutgoingMovement, predictedAngle, bodyPos, FFO, animBallPos.Get2D(), ballPos.Get2D(), ballMovement.Get2D(), touchFramedRadiusFactor, radiusCheatOffset, 1.0f, debug);

        if (bodyBallDistanceAdvantage >= 1.0f || match->GetBallRetainer() == player) {
          found = true;

          bestAnimID = (signed int)*iter;
          bestActionSmuggleVec2D = actionSmuggleVec2D;
          bestTouchMovementAbs = touchMovement.GetRotated2D(spatialState.angle);
          animTouchFrame_ret = animTouchFrame;
          radiusOffset_ret = 1000.0f;//radiusOffset; todo? is this still in use?
        }

      }
      touchNum++;
    }
    iter++;
  }

  if (found) {

    touchPos_ret = currentMentalImage->GetBallPrediction(animTouchFrame_ret * 10);

    fullActionSmuggle_ret = bestActionSmuggleVec2D.GetRotated2D(spatialState.angle);
    actionSmuggle_ret = fullActionSmuggle_ret;

    if (forceFullActionSmuggleDiscard) {

      actionSmuggle_ret = 0;

    } else if (enableActionSmuggleDiscard) {

      // cheat discard distance (don't show some amount of cheat, like, a cheat-cheat :D CHEATCEPTION)
      float smuggleDistance = actionSmuggle_ret.GetLength();
      float adaptedCheatDiscardDistanceMultiplier = cheatDiscardDistanceMultiplier;
      if (functionType != e_FunctionType_BallControl) adaptedCheatDiscardDistanceMultiplier *= 0.8f;
      if (touchPos_ret.coords[2] > 0.5f) adaptedCheatDiscardDistanceMultiplier *= 0.7f;
      if (touchPos_ret.coords[2] > 1.0f) adaptedCheatDiscardDistanceMultiplier *= 0.7f;
      float cheatDiscardDistanceBonus = 0.0f;
      if (functionType == e_FunctionType_Interfere) cheatDiscardDistanceBonus += 0.1f; // don't break flow


      // smuggle, in this context, is how much we will move to the ball after the physics have been accounted for.
      // this is so it seems like we actually touch the ball. we can discard this smuggle somewhat though to make the physics look better.
      // it always is a trade-off between visually touching the ball and visually moving 'correctly', physics-wise.

      // method 1: always discard a part of the smuggle distance
      smuggleDistance = clamp(smuggleDistance - (cheatDiscardDistance + cheatDiscardDistanceBonus), 0.0f, 100.0);
      smuggleDistance *= 1.0f - adaptedCheatDiscardDistanceMultiplier;

      smuggleDistance = std::min(smuggleDistance, actionSmuggle_ret.GetLength() - maxSmuggleDiscardDistance);

/* deprecated
      // method 2: force maximum smuggle - discard the rest
      float smuggleDistanceMPS = smuggleDistance / (animTouchFrame_ret / 100.0f);
      float maximumSmuggleMPSFactor = 1.0;
      if (functionType == e_FunctionType_Sliding) maximumSmuggleMPSFactor *= 4.0f;
      if (functionType == e_FunctionType_Deflect) maximumSmuggleMPSFactor *= 3.0f;
      //smuggleDistanceMPS = clamp(smuggleDistanceMPS, 0.0f, maximumSmuggleMPS * (1.0f / adaptedCheatDiscardDistanceMultiplier) * maximumSmuggleMPSFactor);
      smuggleDistanceMPS = clamp(smuggleDistanceMPS, 0.0f, maximumSmuggleMPS * maximumSmuggleMPSFactor);
      smuggleDistance = smuggleDistanceMPS * (animTouchFrame_ret / 100.0f);
*/

      if (match->GetBallRetainer() == player) smuggleDistance = 0.0f;
      actionSmuggle_ret = actionSmuggle_ret.GetNormalized(0) * smuggleDistance;


      // lose forward-facing part of smuggle

      if (discardForwardSmuggle || discardSidewaysSmuggle) {

        radian toStraightAngle = spatialState.angle + predictedAngle;
        actionSmuggle_ret.Rotate2D(-toStraightAngle);

        if (discardForwardSmuggle) {
          float shortenForwardDistance = 0.02f;
          float allowForwardDistance =
              0.25f *
              (1.0f -
               std::pow(NormalizedClamp(adaptedOutgoingMovement.GetLength(),
                                        idleVelocity, sprintVelocity - 2.0f),
                        0.6f)) *
              (animTouchFrame_ret * 0.1f);
          if (functionType != e_FunctionType_BallControl && functionType != e_FunctionType_Trap) allowForwardDistance += 0.1f; // allow pass/shot/intefere anims and such to smuggle forwards more, since their follow-up-movement doesn't matter that much
          if (actionSmuggle_ret.coords[1] < 0.0f) actionSmuggle_ret.coords[1] = clamp(actionSmuggle_ret.coords[1] + shortenForwardDistance, -allowForwardDistance, 0.0f);
        }
        if (discardSidewaysSmuggle) {
          actionSmuggle_ret.coords[0] *= 0.7f;
        }

        actionSmuggle_ret.Rotate2D(toStraightAngle);

      }

      // less chaos in micro battles
      actionSmuggle_ret *= 0.7f + 0.3f * NormalizedClamp(CastPlayer()->GetClosestOpponentDistance(), 0.6f, 1.2f);
    }

    assert(actionSmuggle_ret.coords[2] == 0.0f);
  }

  rotationSmuggle_ret = rotationSmuggle_ret_tmp;

  return bestAnimID;
}


Vector3 Humanoid::CalculateMovementSmuggle(const Vector3 &desiredDirection, float desiredVelocityFloat) {

  if (!enableMovementSmuggle) return Vector3(0);

  if (team->GetDesignatedTeamPossessionPlayer() != player || match->GetDesignatedPossessionPlayer() != player ||
      currentAnim->touchFrame != -1 || (currentAnim->functionType == e_FunctionType_Trip && currentAnim->anim->GetVariable("triptype").compare("1") != 0) || currentAnim->anim->GetVariableCache().incoming_special_state().compare("") != 0 || currentAnim->anim->GetVariableCache().outgoing_special_state().compare("") != 0 ||
      !match->IsInPlay() || match->IsInSetPiece() || match->GetBallRetainer() != 0) return Vector3(0);


  Vector3 toDesired;


  // various stuff needed by all

  unsigned int timeToBall_ms = CastPlayer()->GetTimeNeededToGetToBall_ms();
  if (CastPlayer()->GetDesiredTimeToBall_ms() > (signed int)timeToBall_ms) {
    timeToBall_ms = CastPlayer()->GetDesiredTimeToBall_ms();
  }
  unsigned int animTime_ms = currentAnim->anim->GetFrameCount() * 10;
  unsigned int futureTime_ms = std::max(animTime_ms + defaultTouchOffset_ms, timeToBall_ms);


  Vector3 predictedOutgoingMovement = CalculateOutgoingMovement(currentAnim->positions);
  Vector3 predictedPos;
  radian predictedAngle;
  CalculatePredictedSituation(predictedPos, predictedAngle);
  Vector3 ballPos = currentMentalImage->GetBallPrediction(futureTime_ms);
  float ballHeight = ballPos.coords[2];
  Vector3 ffo = GetFrontOfFootOffsetRel(predictedOutgoingMovement.GetLength(), currentAnim->anim->GetOutgoingBodyAngle(), ballHeight).GetRotated2D(predictedAngle);
  Vector3 desiredBallPos = predictedPos + ffo;


  if (!CastPlayer()->HasPossession()) {

    // macro effect: consider a line going in the ball movement direction. consider the spot we want the ball at (in front of us) after this movement anim.
    // now calculate the shortest line between that line and that point. now move over that line from the point towards the line somewhat

    Line ballMovementLine;
    ballMovementLine.SetVertex(0, currentMentalImage->GetBallPrediction(0).Get2D());
    ballMovementLine.SetVertex(1, currentMentalImage->GetBallPrediction(futureTime_ms).Get2D());
    if (ballMovementLine.GetLength() < 0.5f) return Vector3(0); // ball is slow or very close


    float u = ballMovementLine.GetClosestToPoint(desiredBallPos);
    Vector3 closestBallPos = ballMovementLine.GetVertex(0) + (ballMovementLine.GetVertex(1) - ballMovementLine.GetVertex(0)) * u;

    toDesired = closestBallPos - desiredBallPos;

  } else { // if HasPossession

    toDesired = ballPos.Get2D() - desiredBallPos;

  }


  unsigned int maxEffectTimeTreshold_ms = 250 + defaultTouchOffset_ms; // if the ball is this much longer 'farther away' than feasible, rather postpone effect until next anim (else we may overrun)
  if (FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle) maxEffectTimeTreshold_ms = 2000; // no danger of overrunning
  float maxEffectVelocity = dribbleWalkSwitch;
  float maxSmuggleMPS = 1.6f;

  if (futureTime_ms - (animTime_ms + defaultTouchOffset_ms) > maxEffectTimeTreshold_ms) return Vector3(0);

  Vector3 toDesiredMovement = toDesired / (animTime_ms * 0.001f);
  Vector3 resultingMovement = predictedOutgoingMovement + toDesiredMovement;
  float predictedVelocity = predictedOutgoingMovement.GetLength();
  float resultingVelocity = resultingMovement.GetLength();
  if (resultingVelocity > predictedVelocity && resultingVelocity > maxEffectVelocity) return Vector3(0);

  toDesired.NormalizeMax(maxSmuggleMPS * (currentAnim->anim->GetEffectiveFrameCount() * 0.01f));

  //SetGreenDebugPilon(predictedPos);

  float removeDistance = 0.06f; // remove part of the smuggle (allow staying this far away from ideal spot)
  toDesired = toDesired.GetNormalized(0) * std::max(0.0f, toDesired.GetLength() - removeDistance);
  return toDesired;
}

Vector3 Humanoid::GetBestPossibleTouch(const Vector3 &desiredTouch, e_FunctionType functionType) {

  float maxPowerShortPass = 30.0f;
  float maxPowerHighPass  = 42.0f;
  float maxPowerBase = maxPowerShortPass;
  if (functionType == e_FunctionType_HighPass) maxPowerBase = maxPowerHighPass;

  Vector3 resultTouch = desiredTouch;

  // fetch vars

  float maxPowerFactor = atof(currentAnim->anim->GetVariable("touch_maxpowerfactor").c_str());
  if (maxPowerFactor == 0.0f) maxPowerFactor = 1.0f;
  maxPowerFactor = maxPowerFactor * 0.7f + 0.3f;


  // clamp to maximum possible power (from anim vars)

  float maxPower = maxPowerBase * maxPowerFactor * (1.0f - clamp(decayingPositionOffset.GetLength() * 2.5f, 0.0f, 0.25f));
  maxPower += match->GetBall()->GetMovement().GetLength() * 0.5f; // can use some of current ballmomentum
  if (resultTouch.GetLength() > maxPower) {
    float missingPower = resultTouch.GetLength() - maxPower;
    resultTouch = resultTouch.GetNormalized(0) * maxPower;
    resultTouch.coords[2] += clamp(missingPower, 0.0f, 10.0f) * 0.25f;
  }


  // difficulty

  float difficultyFactor = atof(currentAnim->anim->GetVariable("touch_difficultyfactor").c_str());

  // apply stats
  if (functionType == e_FunctionType_ShortPass ||
      functionType == e_FunctionType_LongPass) difficultyFactor *= (1.0f - CastPlayer()->GetStat(technical_shortpass) * 0.5f);
  if (functionType == e_FunctionType_HighPass) difficultyFactor *= (1.0f - CastPlayer()->GetStat(technical_highpass)  * 0.5f);

  float distanceFactor = 0.0f;
  float heightFactor = 0.0f;
  float ballMovementFactor = 0.0f;
  GetDifficultyFactors(match, CastPlayer(), decayingPositionOffset, distanceFactor, heightFactor, ballMovementFactor);

  // difficult balls may go into a more random orientation, or, if the anim has a default outgoing direction, it may converge towards that (since it is the easiest direction for that anim)
  radian randomRotation = 0.0f;
  randomRotation = distanceFactor * 0.15f + heightFactor * 0.15f + ballMovementFactor * 0.3f + difficultyFactor * 0.5f;
  Vector3 animBallDirection = GetVectorFromString(currentAnim->anim->GetVariable("balldirection")).GetRotated2D(startAngle + currentAnim->rotationSmuggleOffset);
  if (animBallDirection.GetLength() > 0.01f) {
    float bias = clamp(randomRotation * 1.5f, 0.0f, 1.0f);
    Vector3 nativeTouch = animBallDirection.GetNormalized(resultTouch).Get2D() * resultTouch.GetLength() + resultTouch * Vector3(0, 0, 1);
    resultTouch = resultTouch * (1.0f - bias) + nativeTouch * bias;
  } else {
    radian rotation = random(-0.5f * pi, 0.5f * pi) * std::min(randomRotation, 0.5f);
    resultTouch.Rotate2D(rotation);
  }

  // ball far away == less power
  resultTouch *= 1.0f - distanceFactor * 0.3f;
  resultTouch.coords[2] += distanceFactor * 1.5f; // try to correct (add power) by playing higher ball (== less ground friction)

  resultTouch.coords[2] += match->GetBall()->GetMovement().coords[2] * heightFactor * 0.5f +
                           heightFactor * 1.0f;

  resultTouch = resultTouch * (1.0f - ballMovementFactor) +
                match->GetBall()->GetMovement() * ballMovementFactor;

  resultTouch.coords[2] += difficultyFactor * 5.0f * random(0.2f, 1.0f);

  return resultTouch;
}
