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

#ifndef _HPP_FOOTBALL_ONTHEPITCH_BALL
#define _HPP_FOOTBALL_ONTHEPITCH_BALL

#include "../defines.hpp"
#include "../scene/scene3d/scene3d.hpp"
#include "../scene/objects/geometry.hpp"

#include "../gamedefines.hpp"
#include "../utils.hpp"

using namespace blunted;

class Match;

struct BallSpatialInfo {
  BallSpatialInfo(const Vector3 &momentum, const Quaternion &rotation_ms) { DO_VALIDATION;
    this->momentum = momentum;
    this->rotation_ms = rotation_ms;
  }
  Vector3 momentum;
  Quaternion rotation_ms;
};

class Ball {

  public:
    Ball(Match *match);
    virtual ~Ball();

    void Mirror();
    boost::intrusive_ptr<Geometry> GetBallGeom() { DO_VALIDATION; return ball; }

    inline Vector3 Predict(int predictTime_ms) const {
      int index = predictTime_ms;
      if (index >= ballPredictionSize_ms) index = ballPredictionSize_ms - 10;
      index = index / 10;
      if (index < 0) index = 0;
      return predictions[index];
    }

    void GetPredictionArray(std::vector<Vector3> &target);
    Vector3 GetMovement();
    Vector3 GetRotation();
    void Touch(const Vector3 &target);
    void SetPosition(const Vector3 &target);
    void SetMomentum(const Vector3 &target);
    void SetRotation(real x, real y, real z, float bias = 1.0);     // radians per second for each axis
    BallSpatialInfo CalculatePrediction();  // returns momentum in 10ms

    bool BallTouchesNet() { DO_VALIDATION; return ballTouchesNet; }
    Vector3 GetAveragePosition(unsigned int duration_ms) const;

    void Process();
    void Put();

    void ResetSituation(const Vector3 &focusPos);
    void ProcessState(EnvState *state);
  private:
    boost::intrusive_ptr<Node> ballNode;
    boost::intrusive_ptr<Geometry> ball;
    Vector3 momentum;
    Quaternion rotation_ms;

    Vector3 predictions[ballPredictionSize_ms / 10 + cachedPredictions + 1];
    int valid_predictions = 0;
    Quaternion orientPrediction;

    std::list<Vector3> ballPosHistory;

    Vector3 positionBuffer;
    Quaternion orientationBuffer;

    Match *match;

    bool ballTouchesNet = false;

};

#endif
