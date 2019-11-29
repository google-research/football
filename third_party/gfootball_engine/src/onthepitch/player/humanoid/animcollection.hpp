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

#ifndef _HPP_FOOTBALL_ONTHEPITCH_ANIMCOLLECTION
#define _HPP_FOOTBALL_ONTHEPITCH_ANIMCOLLECTION

#include "../../../utils/animation.hpp"
#include "../../ball.hpp"

#include "../../../scene/objects/geometry.hpp"
#include "../../../utils/objectloader.hpp"

#include "../../../gamedefines.hpp"

using namespace blunted;

inline radian FixAngle(radian angle);
inline float RangeVelocity(float velocity);
inline float ClampVelocity(float velocity);
inline float EnumToFloatVelocity(e_Velocity velocity);
inline e_Velocity FloatToEnumVelocity(float velocity);

radian FixAngle(radian angle) { DO_VALIDATION;
  // convert engine angle into football angle (different base orientation: 'down' on y instead of 'right' on x)
  angle += 0.5f * pi;
  return ModulateIntoRange(-pi, pi, angle);
}

float RangeVelocity(float velocity) { DO_VALIDATION;
  float retVelocity = idleVelocity;
  if (velocity >= idleDribbleSwitch && velocity < dribbleWalkSwitch) retVelocity = dribbleVelocity;
  else if (velocity >= dribbleWalkSwitch && velocity < walkSprintSwitch) retVelocity = walkVelocity;
  else if (velocity >= walkSprintSwitch) retVelocity = sprintVelocity;
  return retVelocity;
}

float ClampVelocity(float velocity) { DO_VALIDATION;
  if (velocity < 0) return 0;
  if (velocity > sprintVelocity) return sprintVelocity;
  return velocity;
}

float EnumToFloatVelocity(e_Velocity velocity) { DO_VALIDATION;
  switch (velocity) { DO_VALIDATION;
    case e_Velocity_Idle:
      return idleVelocity;
      break;
    case e_Velocity_Dribble:
      return dribbleVelocity;
      break;
    case e_Velocity_Walk:
      return walkVelocity;
      break;
    case e_Velocity_Sprint:
      return sprintVelocity;
      break;
  }
  return 0;
}

e_Velocity FloatToEnumVelocity(float velocity) { DO_VALIDATION;
  float rangedVelocity = RangeVelocity(velocity);
  if (rangedVelocity == idleVelocity) return e_Velocity_Idle;
  else if (rangedVelocity == dribbleVelocity) return e_Velocity_Dribble;
  else if (rangedVelocity == walkVelocity) return e_Velocity_Walk;
  else if (rangedVelocity == sprintVelocity) return e_Velocity_Sprint;
  else return e_Velocity_Idle;
}

struct CrudeSelectionQuery {
  CrudeSelectionQuery() { DO_VALIDATION;
    byFunctionType = false;
    byFoot = false; foot = e_Foot_Left;
    heedForcedFoot = false; strongFoot = e_Foot_Right;
    bySide = false;
    allowLastDitchAnims = false;
    byIncomingVelocity = false; incomingVelocity_Strict = false; incomingVelocity_NoDribbleToIdle = false; incomingVelocity_NoDribbleToSprint = false; incomingVelocity_ForceLinearity = false;
    byOutgoingVelocity = false;
    byPickupBall = false; pickupBall = true;
    byIncomingBodyDirection = false; incomingBodyDirection_Strict = false; incomingBodyDirection_ForceLinearity = false;
    byIncomingBallDirection = false;
    byOutgoingBallDirection = false;
    byTripType = false;
  }

  bool byFunctionType = false;
  e_FunctionType functionType;

  bool byFoot = false;
  e_Foot foot;

  bool heedForcedFoot = false;
  e_Foot strongFoot;

  bool bySide = false;
  Vector3 lookAtVecRel;

  bool allowLastDitchAnims = false;

  bool byIncomingVelocity = false;
  bool incomingVelocity_Strict = false; // if true, allow no difference in velocity
  bool incomingVelocity_NoDribbleToIdle = false;
  bool incomingVelocity_NoDribbleToSprint = false;
  bool incomingVelocity_ForceLinearity = false;
  e_Velocity incomingVelocity;

  bool byOutgoingVelocity = false;
  e_Velocity outgoingVelocity;

  bool byPickupBall = false;
  bool pickupBall = false;

  bool byIncomingBodyDirection = false;
  Vector3 incomingBodyDirection;
  bool incomingBodyDirection_Strict = false;
  bool incomingBodyDirection_ForceLinearity = false;

  bool byIncomingBallDirection = false;
  Vector3 incomingBallDirection;

  bool byOutgoingBallDirection = false;
  Vector3 outgoingBallDirection;

  bool byTripType = false;
  int tripType = 0;

  VariableCache properties;
};

struct Quadrant {
  int id = 0;
  Vector3 position;
  e_Velocity velocity;
  radian angle;
  void ProcessState(EnvState* state) {
    state->process(id);
    state->process(position);
    state->process(&velocity, sizeof(velocity));
    state->process(angle);
  }
};

void FillNodeMap(boost::intrusive_ptr<Node> targetNode, NodeMap &nodeMap);

class AnimCollection {

  public:
    // scene3D for debugging pilon
    AnimCollection();
    virtual ~AnimCollection();

    void Clear();
    void Load();

    const std::vector < Animation* > &GetAnimations() const;

    void CrudeSelection(DataSet &dataSet, const CrudeSelectionQuery &query);

    inline Animation* GetAnim(int index) { DO_VALIDATION;
      return animations.at(index);
    }

    inline const Quadrant &GetQuadrant(int id) { DO_VALIDATION;
      return quadrants.at(id);
    }

    int GetQuadrantID(Animation *animation, const Vector3 &movement, radian angle) const;

    void ProcessState(EnvState* state);

  protected:

    void _PrepareAnim(Animation *animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const NodeMap &nodeMap, bool convertAngledDribbleToWalk = false);

    bool _CheckFunctionType(e_DefString functionType, e_FunctionType queryFunctionType) const;

    std::vector<Animation*> animations;
    std::vector<Quadrant> quadrants;

    radian maxIncomingBallDirectionDeviation;
    radian maxOutgoingBallDirectionDeviation;

};

#endif
