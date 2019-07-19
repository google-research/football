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

#include "humanoid_utils.hpp"
#include <cmath>

#include "../../../main.hpp"

#include "../../match.hpp"

#include "humanoid.hpp"

#include "animcollection.hpp"
#include "../../../utils/animationextensions/footballanimationextension.hpp"
#include "../../AIsupport/AIfunctions.hpp"
#include "../playerbase.hpp"

e_TouchType GetTouchTypeForBodyPart(const std::string &bodypartname) {
  if (bodypartname.find("foot") != std::string::npos ||
      bodypartname.find("lowerleg") != std::string::npos)
    return e_TouchType_Intentional_Kicked;
  else
    return e_TouchType_Intentional_Nonkicked;
}

float CalculateBiasForFastCornering(const Vector3 &currentMovement, const Vector3 &desiredMovement, float veloPow, float bias) {

  radian angle = desiredMovement.GetNormalized(currentMovement).GetAngle2D(currentMovement);
  // wolfram alpha: sin(x - 0.5 * pi) * 0.5 + 0.5 | from x = 0.0 to pi
  float currentMovementBias = sin(fabs(angle) - 0.5f * pi) * 0.5f + 0.5f; // this one is better for anim selection, else 135 anim is never used (since 135 @ idle is worse than 90 @ dribble then - with this one, 90 @ idle will be preferred (which will allow 135 @ idle as well))
  //float currentMovementBias = sin(2.0f * fabs(angle) - 0.5f * pi) * 0.5f + 0.5f; // this one seems more correct puristically though.. hmm. maybe make a 135 anim that ends in dribble? or would that effectively disable the 180 @ idle?

  // effect is less pronounced at low velocities
  float velocityBias = pow(clamp(currentMovement.GetLength() / (sprintVelocity - 0.5f), 0.0f, 1.0f), veloPow);

  float totalBrakeBias = velocityBias * currentMovementBias * bias;

  return totalBrakeBias;
}

Vector3 CalculateMovementAtFrame(const std::vector<Vector3> &positions, unsigned int frameNum, unsigned int smoothFrames) {

/*
  // simple version for debugging purposes
  unsigned int adaptedFrameNum = std::min(std::max(frameNum, (unsigned int)1), (unsigned int)positions.size() - 1);
  return (positions.at(adaptedFrameNum) - positions.at(adaptedFrameNum - 1)).Get2D() * 100.0f;
*/

  assert(frameNum < positions.size());

  // special case: want the exit movement to be unsmoothed, for we don't want the wrong quantized velocity and such
  if (frameNum == positions.size() - 1) {
    return (positions.at(positions.size() - 1) - positions.at(positions.size() - 2)).Get2D() * 100.0f;
  }
  // special case: if we want the movement at frame 0, we can't get -1 to 0, we need to get 0 to 1 instead
  if (frameNum == 0) {
    return (positions.at(1) - positions.at(0)).Get2D() * 100.0f;
  }

  Vector3 totalMovement;
  unsigned int count = 0;
  for (int frame = (signed int)(frameNum - smoothFrames); frame <= (signed int)(frameNum + smoothFrames); frame++) {
    if (frame > 0 && frame < (signed int)positions.size()) { // was: frame > 1 (i think that was a bug)
      totalMovement += (positions.at(frame) - positions.at(frame - 1)).Get2D() * 100.0f;
      count++;
    }
  }
  if (count > 0) {
    totalMovement /= (float)count;
  }

  return totalMovement;
}

// offset where ball should usually be touched
Vector3 GetFrontOfFootOffsetRel(float velocity, radian bodyAngleRel, float height) {

  float fullDistanceFactor = 1.0f;
  float distance = 0.34f + velocity * defaultTouchOffset_ms * 0.001f * fullDistanceFactor; // must be > 0 (for upcoming dotproduct)

  Vector3 ffo = Vector3(0, -distance * 0.8f, 0);

  radian bodyAngle = bodyAngleRel;
  if (velocity < idleDribbleSwitch) bodyAngle = 0;
  Vector3 angled = Vector3(0, -distance * 0.2f, 0).GetRotated2D(bodyAngle);

  return (ffo + angled) * (1.0f - clamp((height - 0.11f) / 4.0f, 0.0f, 0.5f));
}

bool NeedDefendingMovement(int mySide, const Vector3 &position, const Vector3 &target) {
  // only move if absolutely necessary
  float howDeepIsTarget = std::max((target.coords[0] - position.coords[0]) * -mySide, 0.0f);
  float howWideIsTarget = fabs(target.coords[1] - position.coords[1]);
  howDeepIsTarget -= 0.5f; // some buffer to account for reaction time
  if (howWideIsTarget > howDeepIsTarget * 0.8f) {
    return true;
  } else {
    return false;
  }
}

float StretchSprintTo(const float &inputVelocity, float inputSpaceMaxVelocity, float targetMaxVelocity) {
  assert(targetMaxVelocity > walkSprintSwitch);

  if (inputVelocity < walkSprintSwitch) return inputVelocity;

  float howMuchSprintage = inputVelocity - walkSprintSwitch;

  float oldLength = inputSpaceMaxVelocity - walkSprintSwitch;
  float newLength = targetMaxVelocity - walkSprintSwitch;

  float toNewFactor = newLength / oldLength;

  float resultSprintage = howMuchSprintage * toNewFactor;

  return walkSprintSwitch + resultSprintage;
}

void GetDifficultyFactors(Match *match, Player *player, const Vector3 &positionOffset, float &distanceFactor, float &heightFactor, float &ballMovementFactor) {

  Ball *ball = match->GetBall();

  distanceFactor = 0.0f; // how far ball bounces off feet
  heightFactor = 0.0f; // how high ball bounces off feet
  ballMovementFactor = 0.0f; // how much of the current ball movement is maintained
  // being pushed is tough
  float positionOffsetPenalty = NormalizedClamp(positionOffset.GetLength(), 0.0f, 0.1f) * 2.0f; // was: 7.0f
  // fast balls are harder
  float ballBodyVeloPenalty =
      std::pow(NormalizedClamp(
                   (player->GetMovement() - ball->GetMovement()).GetLength(),
                   10.0f, 50.0f),
               1.5f) *
      5.0f;
  // balls farther away from body are harder
  float fartherAwayPenalty =
      std::pow(NormalizedClamp(
                   ((player->GetPosition() + player->GetDirectionVec() * 0.2f) -
                    ball->Predict(0).Get2D())
                       .GetLength(),
                   0.7f, 1.3f),
               2.0f) *
      2.0f;

  distanceFactor += positionOffsetPenalty * 2.0f;
  distanceFactor += ballBodyVeloPenalty;
  distanceFactor += fartherAwayPenalty * 4.0f;

  heightFactor += positionOffsetPenalty * 0.5f;
  heightFactor += ballBodyVeloPenalty * 2.0f;
  heightFactor += fartherAwayPenalty;

  // make intercepting passes harder
  if (match->GetLastTouchTeamID() != player->GetTeam()->GetID()) {
    Player *lastTouchPlayer = match->GetTeam(abs(player->GetTeam()->GetID() - 1))->GetLastTouchPlayer();
    if (lastTouchPlayer) {
      float lastTouchBiasPenalty =
          std::pow(lastTouchPlayer->GetLastTouchBias(
                       1000 - player->GetStat(physical_reaction) * 500),
                   0.6f) *
          5.0f;
      distanceFactor += lastTouchBiasPenalty;
      heightFactor += lastTouchBiasPenalty;
      ballMovementFactor += lastTouchBiasPenalty * 0.1f;
    }
  }
  ballMovementFactor = clamp(ballMovementFactor, 0.0f, 0.9f);

  float skillPenaltyMultiplier = (1.0f - player->GetStat(technical_ballcontrol) * 0.5f) * random(0.5f, 1.0f);
  distanceFactor *= skillPenaltyMultiplier;
  heightFactor *= skillPenaltyMultiplier;
  ballMovementFactor *= skillPenaltyMultiplier;
  distanceFactor = clamp(distanceFactor, 0.0f, 1.0f);
  heightFactor = clamp(heightFactor, 0.0f, 1.0f);
  ballMovementFactor = clamp(ballMovementFactor, 0.0f, 1.0f);
}

Vector3 GetBallControlVector(Ball *ball, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot, float ffoOffset) {

  // part of the resulting direction is physics, the other part is anim/controller. so originatingBias is only applied on the (1.0 - physicsBias) part of the result
  float physicsBias = 0.7f;
  if (!player->HasPossession()) {
    physicsBias = 0.9f;
  }

  if (FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle) physicsBias = 1.0f;

  float explosivenessFactor = 0.6f;
  float maximumOverdrive_mps = 1.0f * explosivenessFactor;
  if (currentAnim->originatingCommand.desiredVelocityFloat < dribbleWalkSwitch) maximumOverdrive_mps = 0.0f;
  if (currentAnim->originatingCommand.desiredVelocityFloat > walkSprintSwitch) maximumOverdrive_mps = 2.0f * explosivenessFactor;
  float dotFactor1 = spatialState.directionVec.GetDotProduct(currentAnim->originatingCommand.desiredDirection) * 0.5f + 0.5f; // inv deviation from the current direction
  float dotFactor2 = Vector3(0, -1, 0).GetRotated2D(nextStartAngle).GetDotProduct(currentAnim->originatingCommand.desiredDirection) * 0.5f + 0.5f; // inv deviation from the direction we wanted
  float dotFactor = std::min(dotFactor1, dotFactor2);
  maximumOverdrive_mps *= dotFactor;

  if (FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle) maximumOverdrive_mps = 0.0f;

  float originatingBias = 0.7f;

  // using just controller velo
  Vector3 desiredMovement = currentAnim->originatingCommand.desiredDirection * (currentAnim->originatingCommand.desiredVelocityFloat * originatingBias + player->GetController()->GetFloatVelocity() * (1.0f - originatingBias));

  // range velocity, pes6 style
  //desiredMovement = desiredMovement.GetNormalized(0) * RangeVelocity(desiredMovement.GetLength());

  // hold velo to max player velo - else slow players will kick the ball too far away, which is sad
  if (desiredMovement.GetLength() > walkSprintSwitch) desiredMovement = desiredMovement.GetNormalized(0) * StretchSprintTo(desiredMovement.GetLength(), sprintVelocity, player->GetMaxVelocity());

  // don't want to go below physics velo, else we will run over ball and stuff like that
  // don't want to go over physics velo too much either, else ball will be so far away!
  float velocity = clamp(desiredMovement.GetLength(), outgoingMovement.GetLength(), outgoingMovement.GetLength() + maximumOverdrive_mps);
  desiredMovement = desiredMovement.GetNormalized(Vector3(0, -1, 0).GetRotated2D(nextStartAngle)) * velocity;
  Vector3 physicsMovement = Vector3(0, -1, 0).GetRotated2D(nextStartAngle) * velocity;

  float desiredVelocity = desiredMovement.GetLength();
  float physicsVelocity = physicsMovement.GetLength();

  Vector3 FFOsrc = GetFrontOfFootOffsetRel(physicsVelocity, nextBodyAngle - spatialState.angle, ball->Predict(0).coords[2]);
  float annoyanceVeloFactor = curve(NormalizedClamp(currentAnim->anim->GetOutgoingVelocity(), idleVelocity, sprintVelocity), 0.7f); // do not apply effect to low velo's; makes it too chaotic
  float opponentAnnoyanceFactor = (1.0f - NormalizedClamp(player->GetClosestOpponentDistance(), 0.5f, 1.5f)) * (1.0f - (player->GetStat(mental_calmness) * 0.5f + player->GetStat(physical_balance) * 0.3f)) * annoyanceVeloFactor;
  Vector3 FFO = Vector3(0, -1, 0).GetRotated2D(nextBodyAngle) * (FFOsrc.GetLength() + ffoOffset + opponentAnnoyanceFactor * 3.0f); // positionOffset is already in ffoOffset (though only for trap atm)
  float heightFFOOffset = NormalizedClamp(ball->Predict(0).coords[2], 0.5f, 1.0f) * 0.5f; // bounce high balls off body - else they keep colliding inside body and stuff like that
  FFO += FFOsrc * heightFFOOffset * 0.5f +
         player->GetBodyDirectionVec() * heightFFOOffset * 0.5f;

  Vector3 desiredPlannedBallPos = nextStartPos;
  Vector3 physicsPlannedBallPos = nextStartPos;

  // we actually want the ball to be further away than our next position - after all, we want to hit it again a few steps later
  float desiredDelayTime =
      std::pow(NormalizedClamp(desiredVelocity, 0.0f, sprintVelocity), 2.0f) *
          0.60f +
      0.25f;  // how many seconds later do we want to be able to hit it again?
              // (remember: this is minus ffo time and current anim time)
  float physicsDelayTime =
      std::pow(NormalizedClamp(physicsVelocity, 0.0f, sprintVelocity), 2.0f) *
          0.60f +
      0.25f;  // how many seconds later do we want to be able to hit it again?
              // (remember: this is minus ffo time and current anim time)
  // printf("PHYSICS VELO: %f (desiredDelayTime: %f, physicsDelayTime: %f)\n",
  // physicsVelocity, desiredDelayTime, physicsDelayTime);
  desiredPlannedBallPos += desiredMovement * desiredDelayTime + FFO;
  physicsPlannedBallPos += physicsMovement * physicsDelayTime + FFO;

  /*
  if (player->GetDebug()) {
     SetGreenDebugPilon(nextStartPos);
     SetRedDebugPilon(nextStartPos + FFO);
     //SetGreenDebugPilon(desiredPlannedBallPos);
     SetYellowDebugPilon(physicsPlannedBallPos);
  }*/

  Vector3 plannedBallPos = physicsPlannedBallPos * physicsBias + desiredPlannedBallPos * (1.0f - physicsBias);
  Vector3 toPlannedBall = plannedBallPos - ball->Predict(0).Get2D();

  float timeToGo = ((currentAnim->anim->GetEffectiveFrameCount() - frameNum) * 10) * 0.001f;
  timeToGo += physicsDelayTime * physicsBias + desiredDelayTime * (1.0f - physicsBias);
  timeToGo += defaultTouchOffset_ms * 0.001f;//0.08f; // time into next anim where we want to hit the ball

  float divisor = timeToGo * (0.38f + 0.02f * player->GetStat(technical_dribble)); // higher == closer
  divisor *= 1.1f;

  // to get the ball to the planned position in timeToGo seconds, we need to do some pow() magic, since the ball also slows down faster at higher ball velos
  float power = std::pow((toPlannedBall.GetLength() / divisor), 0.7f);
  Vector3 direction = toPlannedBall.GetNormalized(Vector3(0, -1, 0).GetRotated2D(nextBodyAngle));

  // SetYellowDebugPilon(nextStartPos);
  // SetGreenDebugPilon(nextStartPos + FFO);

  //if (player->GetDebug()) printf("power: %f\n", power);
  float height = clamp(0.1f + 1.5f * std::pow(power / 10.0f, 1.6f), 0.0f,
                       1.5f);  // power ~= 0 to 10

  //if (player->GetDebug()) printf("tech ballctrl: %f\n", player->GetStat(technical_ballcontrol));
  float powerMultiplier = 1.2f - (player->GetStat(technical_ballcontrol) * 0.03f); // 1.24 .. * 0.1
  float veloBias = NormalizedClamp(velocity, walkVelocity, sprintVelocity - 0.8f); // this multiplier only applies to high velocities
  powerMultiplier = 1.0f * (1.0f - veloBias) + powerMultiplier * veloBias;

  Vector3 touchVec = direction * power * powerMultiplier + Vector3(0, 0, height);
/*
  if (player->GetDebug()) {
    SetRedDebugPilon(ball->Predict(0).Get2D());
    SetGreenDebugPilon(plannedBallPos);
  }
*/

  float backspinFactor = 30.0f;
  float velo = touchVec.GetLength();
  float veloFactor =
      std::pow(NormalizedClamp(touchVec.GetLength(), 0.0f, 10.0f),
               0.7f);  // extra around walk velocity
  xRot = (FFO.GetNormalized(0) * 0.6f + touchVec.GetNormalized(0) * 0.4f).GetNormalized(0).coords[1] * ((2 * pi * velo) - (backspinFactor * veloFactor));
  yRot = (FFO.GetNormalized(0) * 0.6f + touchVec.GetNormalized(0) * 0.4f).GetNormalized(0).coords[0] * ((2 * pi * velo) - (backspinFactor * veloFactor));

  //if (player->GetDebug()) printf("x: %f, y: %f\n", xRot, yRot);

  return touchVec;
}

Vector3 GetTrapVector(Match *match, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot) {

  Ball *ball = match->GetBall();

  float distanceFactor = 0.0f; // how far ball bounces off feet
  float heightFactor = 0.0f; // how high ball bounces off feet
  float ballMovementFactor = 0.0f; // how much of the current ball movement is maintained
  GetDifficultyFactors(match, player, positionOffset, distanceFactor, heightFactor, ballMovementFactor);

  // base vector to start with
  Vector3 ballControl = GetBallControlVector(ball, player, nextStartPos, nextStartAngle, nextBodyAngle, outgoingMovement, currentAnim, frameNum, spatialState, positionOffset, xRot, yRot, distanceFactor);

  ballControl.coords[2] += ball->GetMovement().coords[2] * heightFactor * 1.0f +
                           heightFactor * 4.0f;

  return ballControl * (1.0f - ballMovementFactor) +
         ball->GetMovement() * ballMovementFactor;

}

Vector3 GetShotVector(Match *match, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot, radian &zRot, float autoDirectionBias) {

  Ball *ball = match->GetBall();

  const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(currentAnim->anim);
  Vector3 touchMovement = CalculateMovementAtFrame(origPositionCache, currentAnim->frameNum).GetRotated2D(spatialState.angle); // spatialState.movement isn't reliable because of smuggles and such
  //SetRedDebugPilon(player->GetPosition() + touchMovement);
  Vector3 touchDirection = touchMovement.GetNormalized(spatialState.directionVec);
  float touchVelocity = touchMovement.GetLength();
  if (FloatToEnumVelocity(touchVelocity) == e_Velocity_Idle) touchDirection = spatialState.directionVec;
  //SetBlueDebugPilon(player->GetPosition() + touchDirection * 8.0f);

  float expressionModifierFactor = NormalizedClamp(currentAnim->originatingCommand.desiredVelocityFloat, idleVelocity, sprintVelocity);

  // previous: float directionFactor = touchDirection.GetDotProduct(currentAnim->originatingCommand.touchInfo.desiredDirection) * 0.5f + 0.5f;
  // ideal angle is ~36 degrees == 0.2 * pi
  radian idealAngle = 0.2f * pi;
  radian angle1 = fabs(touchDirection.GetAngle2D(currentAnim->originatingCommand.touchInfo.desiredDirection.GetRotated2D(-idealAngle)));
  radian angle2 = fabs(touchDirection.GetAngle2D(currentAnim->originatingCommand.touchInfo.desiredDirection.GetRotated2D( idealAngle)));
  radian angle = std::min(angle1, angle2);
  float directionFactor = clamp(angle / pi, 0.0f, 1.0f);
  directionFactor = curve(1.0f - directionFactor, 0.8f);
  //printf("directionfactor: %f\n", directionFactor);


  // calculate power. take desired power as maximum, and subtract based on player/ball movement and such

  float playerDirDesiredDirPowerFactor =
      std::pow(directionFactor,
               0.8f);  // shooting sideways is still pretty easy to power up

  float playerVelocityPowerFactor =
      std::pow(NormalizedClamp(touchVelocity, 0.0f, sprintVelocity),
               0.3f);  // >= walk velo is optimum

  float positionOffsetPowerFactor = 1.0f - NormalizedClamp(positionOffset.GetLength(), 0.0f, 0.1f);

  float powerFactor = 1.0f *
                      (0.7f + playerDirDesiredDirPowerFactor * 0.3f) *
                      (0.7f + playerVelocityPowerFactor * 0.3f) *
                      (0.4f + positionOffsetPowerFactor * 0.6f);
  powerFactor = clamp(powerFactor, 0.0f, 1.0f);

  float adaptedDesiredPower =
      45.0f *
      (0.7f +
       std::pow(currentAnim->originatingCommand.touchInfo.desiredPower, 0.5f) *
           0.3f);

  float animMaxPowerFactor = atof(currentAnim->anim->GetVariable("touch_maxpowerfactor").c_str());
  if (animMaxPowerFactor == 0.0f) animMaxPowerFactor = 1.0f;

  float power = clamp(powerFactor * adaptedDesiredPower, 0.0f, (32.0f + player->GetStat(physical_shotpower) * 13.0f) * (0.2f + animMaxPowerFactor * 0.8f));

  // add this after previous stat-clamp, because using the current ball movement is like an added (power) bonus that everybody profits from, even sucky players
  float playerMovBallMovPowerFactor = (touchMovement - ball->GetMovement()).GetLength();
  playerMovBallMovPowerFactor = NormalizedClamp(playerMovBallMovPowerFactor, 0.0f, 10.0f);

  power *= 1.0f + playerMovBallMovPowerFactor * 0.2f;



  // calculate difficulty, based on factors like desired power, player/ball movement, skill, positionoffset etcetera

  float playerDirDesiredDirEasinessFactor = std::pow(
      directionFactor, 0.8f);  // shooting sideways is still pretty easy to aim

  float playerVelocityEasinessFactor = 1.0f - NormalizedClamp(fabs(touchVelocity - dribbleVelocity), 0.0f, 4.0f); // dribble velo is optimum

  float positionOffsetEasinessFactor = 1.0f - NormalizedClamp(positionOffset.GetLength(), 0.0f, 0.1f);

  float playerMovBallMovEasinessFactor = (touchMovement - ball->GetMovement()).GetLength();
  playerMovBallMovEasinessFactor = 1.0f - playerMovBallMovPowerFactor * (0.5f - player->GetStat(technical_volley) * 0.3f);

  float powerEasinessFactor = 1.0f - NormalizedClamp(power, 30.0f, 100.0f);
  powerEasinessFactor = curve(powerEasinessFactor, 1.0f);

  float easinessFactor = 1.0f *
                         (0.4f + playerDirDesiredDirEasinessFactor * 0.6f) *
                         (0.7f + playerVelocityEasinessFactor * 0.3f) *
                         (0.3f + positionOffsetEasinessFactor * 0.7f) *
                         (0.6f + playerMovBallMovEasinessFactor * 0.4f) *
                         (0.6f + powerEasinessFactor * 0.4f);
  float difficultyFactor = clamp(1.0f - easinessFactor, 0.0f, 1.0f);

  // best case result

  float desiredHeight = 0.05f;
  Vector3 desiredShot = (currentAnim->originatingCommand.touchInfo.desiredDirection.Get2D() + Vector3(0, 0, desiredHeight)).GetNormalized() * power;

  // worst case result

  Vector3 worstCaseDirection = currentAnim->originatingCommand.touchInfo.desiredDirection.Get2D();

  // direction lag
  float laggyDirectionBias = difficultyFactor * 0.8f;
  worstCaseDirection = (touchDirection * laggyDirectionBias + worstCaseDirection * (1.0f - laggyDirectionBias)).GetNormalized(0);

  // random dir
  worstCaseDirection = worstCaseDirection + (Vector3(random(-1, 1), random(-1, 1), random(-1, 1)) * 0.5f * difficultyFactor);
  worstCaseDirection.Normalize();

  float worstCaseHeight = curve(std::pow(difficultyFactor, 0.7f), 0.7f) * 0.7f;

  float worstCasePower =
      power * (1.0f - std::pow(difficultyFactor, 0.7f) * 0.5f);

  Vector3 worstCaseShot = (worstCaseDirection + Vector3(0, 0, worstCaseHeight)).GetNormalized() * worstCasePower;

  // actual result

  float worstCaseFactor = random(0.0f, 1.0f);
  worstCaseFactor =
      std::pow(worstCaseFactor, player->GetStat(technical_shot) * 0.7f);

  Vector3 shot = desiredShot * (1.0f - worstCaseFactor) +
                 worstCaseShot * worstCaseFactor;


  // add a little curve

  float randomCurveFactor = 0.3f + worstCaseFactor * 0.7f;
  float plannedCurveFactor = 0.7f;

  // forward/backward 'curve'
  xRot = -currentAnim->originatingCommand.touchInfo.desiredDirection.coords[1] * 20.0f + (random(-20, 20) * randomCurveFactor);
  yRot = -currentAnim->originatingCommand.touchInfo.desiredDirection.coords[0] * 20.0f + (random(-20, 20) * randomCurveFactor);

  // lateral curve
  radian bodyTouchAngle = spatialState.bodyDirectionVec.GetAngle2D(shot) / pi;
  if (fabs(bodyTouchAngle) > 0.5f) bodyTouchAngle = (1.0f - fabs(bodyTouchAngle)) * signSide(bodyTouchAngle);
  bodyTouchAngle *= 2.0f;
  //printf("bodyTouchAngle: %f\n", bodyTouchAngle);
  radian amount = bodyTouchAngle * 0.25f;
  shot.Rotate2D(amount * (0.4f + 0.6f * NormalizedClamp(shot.GetLength(), 0.0f, 70.0f)));
  zRot = amount * -420 + (random(-20, 20) * plannedCurveFactor);

  //SetRedDebugPilon(match->GetBall()->Predict(0).Get2D() + touchVec.Get2D() * 0.4f);

  return shot;
}
