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

#include "ball.hpp"

#include <cmath>

#include "../utils/objectloader.hpp"
#include "../scene/objectfactory.hpp"

#include "../managers/resourcemanagerpool.hpp"

#include "match.hpp"

#include "../main.hpp"

Ball::Ball(Match *match) : match(match) {

  bounce = 0.62f; // 1 = full bounce, 0 = no bounce
  linearBounce = 0.06f; // bigger = more brake force
  drag = 0.015f;//previously 0.025f; // bigger = more
  friction = 0.04f; // bigger = more
  linearFriction = 1.6f; // bigger = more, arbitrary scale
  gravity = -9.81f;
  grassHeight = 0.025f;

  ballTouchesNet = false;

  scene3D = GetScene3D();

  ObjectLoader loader;
  ballNode = loader.LoadObject(scene3D, "media/objects/balls/generic.object");
  match->GetDynamicNode()->AddNode(ballNode);

  std::list < boost::intrusive_ptr<Geometry> > children;
  ballNode->GetObjects<Geometry>(e_ObjectType_Geometry, children);
  ball = (*children.begin());

  CalculatePrediction();
}

Ball::~Ball() {
  match->GetDynamicNode()->DeleteNode(ballNode);
}

void Ball::GetPredictionArray(Vector3 *target) {
  memcpy(target, predictions, sizeof(Vector3) * ballPredictionSize_ms / 10);
}

Vector3 Ball::GetMovement() {
  // meters / sec
  return momentum;
}

Vector3 Ball::GetRotation() {
  radian x, y, z;
  rotation_ms.GetAngles(x, y, z);
  return Vector3(x, y, z);
}

void Ball::Touch(const Vector3 &target) {
  valid_predictions = 0;
  if (positionBuffer.coords[2] < 0.11f) positionBuffer.coords[2] = 0.11f;

  SetMomentum(target);

  // recalculate prediction
  CalculatePrediction();
  match->UpdateLatestMentalImageBallPredictions();

  match->GetTeam(0)->UpdatePossessionStats();
  match->GetTeam(1)->UpdatePossessionStats();
}

void Ball::SetPosition(const Vector3 &target) {
  valid_predictions = 0;
  positionBuffer.Set(target);
  momentum.Set(0);
  SetRotation(0, 0, 0, 1.0);
  ballPosHistory.clear();
  previousMomentum = momentum;
  previousPosition = positionBuffer;
}

void Ball::SetMomentum(const Vector3 &target) {
  momentum.Set(target);
  CalculatePrediction();
}

void Ball::SetRotation(radian x, radian y, radian z, float bias) { // radians per second for each axis
  Quaternion rotX;
  rotX.SetAngleAxis(clamp(x * 0.001f, -pi * 0.49f, pi * 0.49f), Vector3(-1, 0, 0));
  Quaternion rotY;
  rotY.SetAngleAxis(clamp(y * 0.001f, -pi * 0.49f, pi * 0.49f), Vector3(0, 1, 0));
  Quaternion rotZ;
  rotZ.SetAngleAxis(clamp(z * 0.001f, -pi * 0.49f, pi * 0.49f), Vector3(0, 0, 1));

  Quaternion tmpRotation_ms = rotX * rotY * rotZ;
  rotation_ms = rotation_ms.GetSlerped(bias, tmpRotation_ms);

  CalculatePrediction();
}

BallSpatialInfo Ball::CalculatePrediction() {

  Vector3 newMomentum;
  Quaternion newRotation_ms;


  // fill predictions

  Vector3 nextPos = positionBuffer;
  Quaternion nextOrientation = orientationBuffer;
  Vector3 momentumPredict = momentum;
  Quaternion rotationPredict_ms = rotation_ms;

  predictions[0] = nextPos;

  bool drag_enabled = true;
  bool groundFriction_enabled = true;
  bool woodwork_enabled = true;
  bool netting_enabled = true;
  bool groundRotationEffects_enabled = true;
  bool swerve_enabled = true;

  const float timeStep = 0.01f;//0.001f; // seconds

  bool firstTime = true;
  bool use_cache = false;

  ballTouchesNet = false;

  for (unsigned int predictTime_ms = int(timeStep * 1000.0f); predictTime_ms < ballPredictionSize_ms + cachedPredictions * 10; predictTime_ms += int(timeStep * 1000.0f)) {
    // Originally game was recomputing ball's prediction for 300 steps into the
    // future, which was expensive. Now we cache 100 additional steps and if
    // the ball was not touched etc. we just shift predictions by one.
    if (use_cache) {
      predictions[predictTime_ms / 10] = predictions[predictTime_ms / 10 + 1];
      continue;
    }

    float frictionFactor = 0.0f;


    // gravity

    // vz = vz0 + g * t
    momentumPredict.coords[2] = momentumPredict.coords[2] + gravity * timeStep;


    // air resistance

    float momentumVelo = momentumPredict.GetLength();
    float momentumVeloDragged =
        momentumVelo - drag * std::pow(momentumVelo, 2.0f) * timeStep;
    if (drag_enabled) momentumPredict = momentumPredict.GetNormalized(0) * momentumVeloDragged;


    float ballBottom = nextPos.coords[2] - 0.11f;
    float grassInfluenceBias = clamp(1.0f - (ballBottom / grassHeight), 0.0f, 1.0f); // 0 == no friction, 1 == all friction

    grassInfluenceBias = std::pow(
        grassInfluenceBias, 0.7f);  // at half grass height, there's already a
                                    // bigger amount of friction than 50%
    // printf("%f\n", bias);

    // bounce

    if (nextPos.coords[2] < 0.11f) {
      if (momentumPredict.coords[2] < 0.0f) {
        frictionFactor = NormalizedClamp(-momentumPredict.coords[2] - 0.5f, 0.0f, 12.0f); // when the ball is slammed into the ground, there's gonna be more friction. only set it here so it is only done once (on impact)
        momentumPredict.coords[2] = -momentumPredict.coords[2] * bounce;
        momentumPredict.coords[2] = std::max(momentumPredict.coords[2] - linearBounce, 0.0f); // linear bounce
      }

      nextPos.coords[2] = 0.11f;
    }


    // ground friction

    if (nextPos.coords[2] < 0.11f + grassHeight && groundFriction_enabled) {
      float adaptedFriction = (friction * grassInfluenceBias);

      // v(t) = v(0) * (k ^ t)

      Vector3 xy = momentumPredict.Get2D();
      float velo = xy.GetLength();

      float newVelo = velo - adaptedFriction * std::pow(velo, 2.0f) * timeStep;

      // linear friction
      newVelo = clamp(newVelo - (linearFriction * grassInfluenceBias * timeStep), 0.0f, 100000.0f);

      xy.Normalize(Vector3(0));
      xy *= newVelo;
      momentumPredict.coords[0] = xy.coords[0];
      momentumPredict.coords[1] = xy.coords[1];
    }

    float netAbsorbInv = 0.95f;
    float powFactor = 2.6f;
    float powerFac = 1.8f; // lol varnames
    float postAbsorbInv = 0.8f;
    float ballRadius = 0.11f;
    float postRadius = 0.07f;

    netAbsorbInv = std::pow(netAbsorbInv, timeStep * 100.0f);

    // woodwork

    if (firstTime && woodwork_enabled) {

      bool woodwork = false;


      // posts

      if (nextPos.coords[2] < goalHeight + ballRadius + postRadius && (nextPos.Get2D().GetAbsolute() - Vector3(pitchHalfW, goalHalfWidth, 0)).GetLength() < ballRadius + postRadius) {
        Vector3 normal;

        if (nextPos.coords[0] < 0) {
          // left side of pitch
          if (nextPos.coords[1] < 0) {
            // 'lower' side of pitch
            normal = (nextPos.Get2D() - Vector3(-pitchHalfW, -goalHalfWidth, 0)).GetNormalized(Vector3(1, 0, 0));
            float nextPosZ = nextPos.coords[2];
            nextPos = Vector3(-pitchHalfW, -goalHalfWidth, 0) + normal * (postRadius + ballRadius);
            nextPos.coords[2] = nextPosZ;
            woodwork = true;

          } else {
            // 'upper' side of pitch
            normal = (nextPos.Get2D() - Vector3(-pitchHalfW, goalHalfWidth, 0)).GetNormalized(Vector3(1, 0, 0));
            float nextPosZ = nextPos.coords[2];
            nextPos = Vector3(-pitchHalfW, goalHalfWidth, 0) + normal * (postRadius + ballRadius);
            nextPos.coords[2] = nextPosZ;
            woodwork = true;

          }
        } else {
          // right side of pitch
          if (nextPos.coords[1] < 0) {
            // 'lower' side of pitch
            normal = (nextPos.Get2D() - Vector3(pitchHalfW, -goalHalfWidth, 0)).GetNormalized(Vector3(-1, 0, 0));
            float nextPosZ = nextPos.coords[2];
            nextPos = Vector3(pitchHalfW, -goalHalfWidth, 0) + normal * (postRadius + ballRadius);
            nextPos.coords[2] = nextPosZ;
            woodwork = true;

          } else {
            // 'upper' side of pitch
            normal = (nextPos.Get2D() - Vector3(pitchHalfW, goalHalfWidth, 0)).GetNormalized(Vector3(-1, 0, 0));
            //match->SetDebugPilon(Vector3(55, 3.8, 0) + normal * 5);
            float nextPosZ = nextPos.coords[2];
            nextPos = Vector3(pitchHalfW, goalHalfWidth, 0) + normal * (postRadius + ballRadius);
            nextPos.coords[2] = nextPosZ;
            woodwork = true;

          }
        }

        momentumPredict = (momentumPredict.Get2D().GetNormalized(normal) + (normal * 1.1f)).GetNormalized() * momentumPredict.Get2D().GetLength() * postAbsorbInv + (Vector3(0, 0, 1) * momentumPredict.coords[2]);
      }


      // crossbar

      Vector3 nextPosXZ = nextPos * Vector3(1, 0, 1);
      if ((nextPosXZ.GetAbsolute() - Vector3(pitchHalfW, 0, goalHeight)).GetLength() < ballRadius + postRadius &&
          fabs(nextPos.coords[1]) < goalHalfWidth + ballRadius + postRadius) {
        Vector3 normal;

        if (nextPos.coords[0] < 0) {
          // left side of pitch
          normal = (nextPosXZ - Vector3(-pitchHalfW, 0, goalHeight)).GetNormalized(Vector3(0, 0, 1));
          float nextPosY = nextPos.coords[1];
          nextPos = Vector3(-pitchHalfW, 0, goalHeight) + normal * (postRadius + ballRadius);
          nextPos.coords[1] = nextPosY;
          woodwork = true;

        } else {
          // right side of pitch
          normal = (nextPosXZ - Vector3(pitchHalfW, 0, goalHeight)).GetNormalized(Vector3(0, 0, -1));
          float nextPosY = nextPos.coords[1];
          nextPos = Vector3(pitchHalfW, 0, goalHeight) + normal * (postRadius + ballRadius);
          nextPos.coords[1] = nextPosY;
          woodwork = true;

        }

        Vector3 momentumPredictXZ = momentumPredict * Vector3(1, 0, 1);
        momentumPredict = (momentumPredictXZ.GetNormalized(normal) + (normal * 1.1f)).GetNormalized() * momentumPredictXZ.GetLength() * postAbsorbInv + (Vector3(0, 1, 0) * momentumPredict.coords[1]);
      }
    }


    // netting

    if (predictTime_ms <= 10 && netting_enabled) {

      bool ballIsInGoal = match->IsBallInGoal();
      signed int inGoal = ballIsInGoal ? 1 : -1;

      bool behindBackline = fabs(nextPos.coords[0]) > pitchHalfW + 0.11f;
      bool behindGoalBack = fabs(nextPos.coords[0]) > pitchHalfW + goalDepth + 0.11f;
      bool beforeGoalBack = fabs(nextPos.coords[0]) < pitchHalfW + goalDepth - 0.11f;
      bool belowGoalHeight = nextPos.coords[2] < goalHeight + 0.11f;
      bool aboveGoalHeight = nextPos.coords[2] > goalHeight - 0.11f;
      bool betweenGoalWidth = fabs(nextPos.coords[1]) < goalHalfWidth - 0.11f;
      bool asideGoalWidth = fabs(nextPos.coords[1]) > goalHalfWidth + 0.11f;


      // side netting

      if (( ballIsInGoal && !betweenGoalWidth && behindBackline)) {

        float netDist = 0.0f;
        netDist = fabs(fabs(nextPos.coords[1]) - goalHalfWidth);
        netDist = clamp(netDist, 0, 1);
        float power = std::pow(netDist, powFactor) *
                      -signSide(nextPos.coords[1]) * inGoal;

        // net is stuck to woodwork so lay off there
        float woodworkTensionBiasInv = clamp((fabs(momentumPredict.coords[0]) - pitchHalfW) * 2.0f, 0.0f, 1.0f);
        float adaptedPowerFac = powerFac + (1.0f - woodworkTensionBiasInv) * 3.0f;

        momentumPredict.coords[1] = momentumPredict.coords[1] * netAbsorbInv + power * adaptedPowerFac * (100 * timeStep);// + -momentumPredict.coords[1] * netDist;

        if (predictTime_ms == 10) ballTouchesNet = true;
      }


      // rear netting

//      if ((fabs(nextPos.coords[0]) > (pitchHalfW + 2.5) - 0.11 && ballIsInGoal)/* ||
//          (fabs(nextPos.coords[0]) < (pitchHalfW + 2.5) + 0.11 && !ballIsInGoal) todo disabled: too hard to code :p */) {

      if (( ballIsInGoal && !beforeGoalBack && behindBackline)/* ||
          (!ballIsInGoal && !asideGoalWidth && behindBackline && !behindGoalBack && belowGoalHeight ** todo disabled: too hard to code :p */) {

        float netDist = 0.0f;
        netDist = fabs(fabs(nextPos.coords[0]) - (pitchHalfW + goalDepth));
        netDist = clamp(netDist, 0, 1);
        float power = std::pow(netDist, powFactor) *
                      -signSide(nextPos.coords[0]) * inGoal;
        momentumPredict.coords[0] = momentumPredict.coords[0] * netAbsorbInv + power * powerFac * (100 * timeStep);

        if (predictTime_ms == 10) ballTouchesNet = true;
      }


      // top netting

//      if (((nextPos.coords[2] > 2.5 - 0.11 && ballIsInGoal)/*( ||
//           (nextPos.coords[2] < 2.5 + 0.11 && !ballIsInGoal) todo disabled: too hard to code :p */) &&
//          fabs(nextPos.coords[0]) > pitchHalfW) {

      if (( ballIsInGoal && !belowGoalHeight && behindBackline )) {

        float netDist = 0.0f;
        netDist = fabs(fabs(nextPos.coords[2]) - goalHeight);
        netDist = clamp(netDist, 0, 1);
        float power = std::pow(netDist, powFactor) * -inGoal;

        // net is stuck to woodwork so lay off there
        float woodworkTensionBiasInv = clamp((fabs(momentumPredict.coords[0]) - pitchHalfW) * 2.0f, 0.0f, 1.0f);
        float adaptedPowerFac = powerFac + (1.0f - woodworkTensionBiasInv) * 3.0f;

        momentumPredict.coords[2] = momentumPredict.coords[2] * netAbsorbInv + power * adaptedPowerFac * (100 * timeStep);

        if (predictTime_ms == 10) ballTouchesNet = true;
      }

    } // </goal collisions>


    // calculate rotation

    if (nextPos.coords[2] < 0.11f + grassHeight && groundRotationEffects_enabled) {


      // rewrite idea: find out difference in ball velo / roll velo and then change both ball velo and rot (instead of having these 2 seperate sections)

      // ground friction induced rotation
      radian xR, yR;

      // x movement causes roll over y axis.. so this is correct ;)
      float radius = 0.11f;
      xR = momentumPredict.coords[1] / radius;
      yR = momentumPredict.coords[0] / radius;

      // clamp, because we can not rotate faster than this or the maths don't know what direction to rotate into anymore
      Quaternion rotX;
      rotX.SetAngleAxis(clamp(xR * 0.001f, -pi * 0.49f, pi * 0.49f), Vector3(-1, 0, 0));
      Quaternion rotY;
      rotY.SetAngleAxis(clamp(yR * 0.001f, -pi * 0.49f, pi * 0.49f), Vector3(0, 1, 0));

      Quaternion groundRot = rotX * rotY;

      Quaternion oldToNewRotation = rotationPredict_ms.GetRotationTo(groundRot).GetNormalized();
      radian rotationChangePerSecond = fabs(oldToNewRotation.GetRotationAngle(QUATERNION_IDENTITY)) * 1000.0f;

      radian maxRotationChangePerSecond = 1.0f * pi * grassInfluenceBias;
      // ball slams into ground; see origin of frictionFactor variable for more clarity. this happens only once per bounce
      // this works here because the 'if' statement is always true when frictionFactor > 0, because when then happens, nextPos.coords[2] has been set to ballRadius anyway
      if (frictionFactor > 0.0f) {
        maxRotationChangePerSecond += 4.0f * pi;
      }
      volatile radian factor = 1.0f;
      if (rotationChangePerSecond > maxRotationChangePerSecond) {
        factor = maxRotationChangePerSecond / rotationChangePerSecond;
      }
      if (factor < 1.0f) {
        oldToNewRotation = oldToNewRotation.GetRotationMultipliedBy(factor);
      }

      Quaternion newRotationPredict_ms = oldToNewRotation * rotationPredict_ms;


      // rotation induced ground friction

      radian x, y, z;
      rotationPredict_ms.GetAngles(x, y, z);
      x = -x;

      // how fast the ball would move if we took 100% of its rotational velo
      Vector3 ballRotationMomentum;
      ballRotationMomentum.coords[0] = y * radius * 1000.0f;// * bias;
      ballRotationMomentum.coords[1] = x * radius * 1000.0f;// * bias;

      // mix (not sure if mathematically correct, i think so, but maybe check out on a rainy sunday once)
      float rotBias = 0.01f; // lower == ball is lighter. higher == pitch/ball contact seems more 'rubbery'
      rotBias *= grassInfluenceBias;

      // ball slams into ground; see origin of frictionFactor variable for more clarity. this happens only once per bounce
      // this works here because the 'if' statement is always true when frictionFactor > 0, because when then happens, nextPos.coords[2] has been set to ballRadius anyway
      if (frictionFactor > 0.0f) {
        rotBias += 0.5f * frictionFactor;
      }
      rotBias = clamp(rotBias, 0.0f, 1.0f);
      momentumPredict.coords[0] = momentumPredict.coords[0] * (1.0f - rotBias) + ballRotationMomentum.coords[0] * rotBias;
      momentumPredict.coords[1] = momentumPredict.coords[1] * (1.0f - rotBias) + ballRotationMomentum.coords[1] * rotBias;


      // finally, add the previously calculated ground friction induced rotation
      rotationPredict_ms = newRotationPredict_ms;
    }


    // magnus effect (swerve)

    if (swerve_enabled) {
      Vector3 rotVec;
      rotationPredict_ms.GetAngles(rotVec.coords[0], rotVec.coords[1], rotVec.coords[2]);
      rotVec *= 10.0f;

      // magnus effect has a strength curve that goes down after a certain velocity
      float swerveAmount = NormalizedClamp(momentumPredict.GetLength(), 0.0f, 70.0f);
      // http://www.wolframalpha.com/input/?i=sin%28x+*+pi+*+0.7%29+^+2.2+from+x+%3D+0+to+1
      // <bazkie_drunk> ^ tnx, past myself, that's very convenient!
      swerveAmount = pow(std::sin(swerveAmount * pi * 0.94f), 2.6f);
      Vector3 adaptedMomentumPredict = momentumPredict.GetNormalized(0) * swerveAmount * 30.0f;

      Vector3 swerve = adaptedMomentumPredict.GetCrossProduct(-rotVec) * 1.0;

      momentumPredict += swerve * timeStep;
    }


    // predict next ms

    nextPos += momentumPredict * timeStep;

    Vector3 rotationVector;
    rotationPredict_ms.GetAngles(rotationVector.coords[0], rotationVector.coords[1], rotationVector.coords[2]);
    rotationVector *= timeStep / 0.001f;
    Quaternion rotationPredictTimeStepped;
    rotationPredictTimeStepped.SetAngles(rotationVector.coords[0], rotationVector.coords[1], rotationVector.coords[2]);

    nextOrientation = rotationPredictTimeStepped * nextOrientation;

    if (predictTime_ms == 10) {
      newMomentum = momentumPredict;
      newRotation_ms = rotationPredict_ms;
      orientPrediction = nextOrientation;
      if (valid_predictions > 0 && predictions[2] == nextPos) {
        valid_predictions--;
        use_cache = true;
      } else {
        valid_predictions = cachedPredictions;
      }
    }
    predictions[predictTime_ms / 10] = nextPos;

    firstTime = false;
  }

  return BallSpatialInfo(newMomentum, newRotation_ms);
}

Vector3 Ball::GetAveragePosition(unsigned int duration_ms) const {
  std::list<Vector3>::const_reverse_iterator iter = ballPosHistory.rbegin();
  unsigned int total = 0;
  Vector3 averageVec;
  while (iter != ballPosHistory.rend()) {
    averageVec += *iter;
    total++;
    if (total * 10 > duration_ms) break;
    iter++;
  }
  if (total > 0) averageVec /= total; else averageVec = Predict(0);
  return averageVec;
}

void Ball::Process() {
  BallSpatialInfo spatialInfo = CalculatePrediction();
  momentum = spatialInfo.momentum;
  rotation_ms = spatialInfo.rotation_ms;

  positionBuffer = Predict(10);
  orientationBuffer = orientPrediction;

  ballPosHistory.push_back(positionBuffer);
  if (ballPosHistory.size() > ballHistorySize_ms) ballPosHistory.pop_front();

  Vector3 changedMomentum = momentum - previousMomentum;

  previousMomentum = momentum;
  previousPosition = positionBuffer;
}

void Ball::PreparePutBuffers(unsigned long snapshotTime_ms) {
  buf_positionBuffer.SetValue(positionBuffer, snapshotTime_ms);//Predict(0);//positionBuffer;
  buf_orientationBuffer.SetValue(orientationBuffer, snapshotTime_ms);
}

void Ball::FetchPutBuffers(unsigned long putTime_ms) {
  fetchedbuf_positionBuffer = buf_positionBuffer.GetValue(putTime_ms);
  fetchedbuf_orientationBuffer = buf_orientationBuffer.GetValue(putTime_ms);
}

void Ball::Put() {
  ball->SetPosition(fetchedbuf_positionBuffer, false);
  ball->SetRotation(fetchedbuf_orientationBuffer, false);
}

void Ball::ResetSituation(const Vector3 &focusPos) {
  momentum = Vector3(0);
  rotation_ms = QUATERNION_IDENTITY;
  for (unsigned int i = 0; i < ballPredictionSize_ms / 10; i++) {
    predictions[i] = Vector3(focusPos + Vector3(0, 0, 0.11));
  }
  orientPrediction = QUATERNION_IDENTITY;
  ballPosHistory.clear();
  previousMomentum = Vector3(0);
  previousPosition = Vector3(focusPos + Vector3(0, 0, 0.11));
  positionBuffer = Vector3(focusPos + Vector3(0, 0, 0.11));
  valid_predictions = 0;
  orientationBuffer = QUATERNION_IDENTITY;
  ballTouchesNet = false;
}
