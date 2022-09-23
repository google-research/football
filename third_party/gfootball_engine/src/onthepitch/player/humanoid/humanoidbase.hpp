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

#ifndef _HPP_HUMANOIDBASE
#define _HPP_HUMANOIDBASE

#include "../../../base/math/vector3.hpp"
#include "../../../scene/scene3d/node.hpp"

#include "../../../gamedefines.hpp"
#include "../../../utils.hpp"

#include "animcollection.hpp"

#include "../../AIsupport/mentalimage.hpp"

using namespace blunted;

class PlayerBase;
class Match;

struct HJoint {
  boost::intrusive_ptr<Node> node;
  Vector3 position;
  Quaternion orientation;
  Vector3 origPos;
};

struct WeightedBone {
  int jointID = 0;
  float weight = 0.0f;
};

struct WeightedVertex {
  int vertexID = 0;
  std::vector<WeightedBone> bones;
};

struct FloatArray {
  float *data;
  int size = 0;
};

enum e_InterruptAnim {
  e_InterruptAnim_None,
  e_InterruptAnim_Switch,
  e_InterruptAnim_Sliding,
  e_InterruptAnim_Bump,
  e_InterruptAnim_Trip,
  e_InterruptAnim_Cheat,
  e_InterruptAnim_Cancel,
  e_InterruptAnim_ReQueue
};

struct RotationSmuggle {
  RotationSmuggle() { DO_VALIDATION;
    begin = 0;
    end = 0;
  }
  void operator = (const float &value) { DO_VALIDATION;
    begin = value;
    end = value;
  }
  radian begin;
  radian end;
  void ProcessState(EnvState* state) { DO_VALIDATION;
    state->process(begin);
    state->process(end);
  }
};

struct Anim {
  Animation *anim = 0;
  signed int id = 0;
  int frameNum = 0;
  e_FunctionType functionType = e_FunctionType_None;
  e_InterruptAnim originatingInterrupt = e_InterruptAnim_None;
  Vector3 actionSmuggle;
  Vector3 actionSmuggleOffset;
  Vector3 actionSmuggleSustain;
  Vector3 actionSmuggleSustainOffset;
  Vector3 movementSmuggle;
  Vector3 movementSmuggleOffset;
  RotationSmuggle rotationSmuggle;
  radian rotationSmuggleOffset = 0;
  signed int touchFrame = -1;
  Vector3 touchPos;
  Vector3 incomingMovement;
  Vector3 outgoingMovement;
  Vector3 positionOffset;
  PlayerCommand originatingCommand;
  std::vector<Vector3> positions;
  void ProcessState(EnvState* state) { DO_VALIDATION;
    state->process(anim);
    state->process(id);
    state->process(frameNum);
    state->process(functionType);
    state->process(originatingInterrupt);
    state->process(actionSmuggle);
    state->process(actionSmuggleOffset);
    state->process(actionSmuggleSustain);
    state->process(actionSmuggleSustainOffset);
    state->process(movementSmuggle);
    state->process(movementSmuggleOffset);
    rotationSmuggle.ProcessState(state);
    state->process(rotationSmuggleOffset);
    state->process(touchFrame);
    state->process(touchPos);
    state->process(incomingMovement);
    state->process(outgoingMovement);
    state->process(positionOffset);
    originatingCommand.ProcessState(state);
    state->process(positions);
  }
};

struct AnimApplyBuffer {
  AnimApplyBuffer() { DO_VALIDATION;
    frameNum = 0;
    smooth = true;
    smoothFactor = 0.5f;
    noPos = false;
    orientation = 0;
  }
  AnimApplyBuffer(const AnimApplyBuffer &src) { DO_VALIDATION;
    anim = src.anim;
    frameNum = src.frameNum;
    smooth = src.smooth;
    smoothFactor = src.smoothFactor;
    noPos = src.noPos;
    position = src.position;
    orientation = src.orientation;
    offsets = src.offsets;
  }
  void ProcessState(EnvState* state) { DO_VALIDATION;
    state->process(anim);
    state->process(frameNum);
    state->process(smooth);
    state->process(smoothFactor);
    state->process(noPos);
    state->process(position);
    state->process(orientation);
    offsets.ProcessState(state);
  }
  Animation *anim = 0;
  int frameNum = 0;
  bool smooth = false;
  float smoothFactor = 0.0f;
  bool noPos = false;
  Vector3 position;
  radian orientation;
  BiasedOffsets offsets;
};

struct SpatialState {
  Vector3 position;
  radian angle;
  Vector3 directionVec; // for efficiency, vector version of angle
  e_Velocity enumVelocity;
  float floatVelocity = 0.0f; // for efficiency, float version

  Vector3 actualMovement;
  Vector3 physicsMovement; // ignores effects like positionoffset
  Vector3 animMovement;
  Vector3 movement; // one of the above (default)
  Vector3 actionSmuggleMovement;
  Vector3 movementSmuggleMovement;
  Vector3 positionOffsetMovement;

  radian bodyAngle;
  Vector3 bodyDirectionVec; // for efficiency, vector version of bodyAngle
  radian relBodyAngleNonquantized;
  radian relBodyAngle;
  Vector3 relBodyDirectionVec; // for efficiency, vector version of relBodyAngle
  Vector3 relBodyDirectionVecNonquantized;
  e_Foot foot;

  void Mirror() { DO_VALIDATION;
    position.Mirror();
    actualMovement.Mirror();
    physicsMovement.Mirror();
    animMovement.Mirror();
    movement.Mirror();
    actionSmuggleMovement.Mirror();
    movementSmuggleMovement.Mirror();
    positionOffsetMovement.Mirror();
  }

  void ProcessState(EnvState* state) { DO_VALIDATION;
    state->process(position);
    state->process(angle);
    state->process(directionVec);
    state->process(enumVelocity);
    state->process(floatVelocity);
    state->process(actualMovement);
    state->process(physicsMovement);
    state->process(animMovement);
    state->process(movement);
    state->process(actionSmuggleMovement);
    state->process(movementSmuggleMovement);
    state->process(positionOffsetMovement);
    state->process(bodyAngle);
    state->process(bodyDirectionVec);
    state->process(relBodyAngleNonquantized);
    state->process(relBodyAngle);
    state->process(relBodyDirectionVec);
    state->process(relBodyDirectionVecNonquantized);
    state->process(foot);
  }
};

class HumanoidBase {

  public:
    HumanoidBase(PlayerBase *player, Match *match, boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::shared_ptr<AnimCollection> animCollection, boost::intrusive_ptr<Node> fullbodyTargetNode, boost::intrusive_ptr < Resource<Surface> > kit);
    virtual ~HumanoidBase();
    void Mirror();

    void PrepareFullbodyModel(std::map<Vector3, Vector3> &colorCoords);
    void UpdateFullbodyNodes(bool mirror);
    void UpdateFullbodyModel(bool updateSrc = false);

    virtual void Process();
    void PreparePutBuffers();
    void FetchPutBuffers();
    void Put(bool mirror);

    virtual void CalculateGeomOffsets();
    void SetOffset(BodyPart body_part, float bias, const Quaternion &orientation, bool isRelative = false);

    inline int GetFrameNum() { DO_VALIDATION; return currentAnim.frameNum; }
    inline int GetFrameCount() { DO_VALIDATION; return currentAnim.anim->GetFrameCount(); }

    inline Vector3 GetPosition() const { return spatialState.position; }
    inline Vector3 GetDirectionVec() const { return spatialState.directionVec; }
    inline Vector3 GetBodyDirectionVec() const {
      return spatialState.bodyDirectionVec;
    }
    inline radian GetRelBodyAngle() const { return spatialState.relBodyAngle; }
    inline e_Velocity GetEnumVelocity() const { return spatialState.enumVelocity; }
    inline e_FunctionType GetCurrentFunctionType() const { return currentAnim.functionType; }
    inline e_FunctionType GetPreviousFunctionType() const { return previousAnim_functionType; }
    inline Vector3 GetMovement() const { return spatialState.movement; }

    Vector3 GetGeomPosition() { DO_VALIDATION; return humanoidNode->GetPosition(); }

    int GetIdleMovementAnimID();
    void ResetPosition(const Vector3 &newPos, const Vector3 &focusPos);
    void OffsetPosition(const Vector3 &offset);
    void TripMe(const Vector3 &tripVector, int tripType);

    boost::intrusive_ptr<Node> GetHumanoidNode() { DO_VALIDATION; return humanoidNode; }
    boost::intrusive_ptr<Node> GetFullbodyNode() { DO_VALIDATION; return fullbodyNode; }

    virtual float GetDecayingPositionOffsetLength() const { return decayingPositionOffset.GetLength(); }
    virtual float GetDecayingDifficultyFactor() const { return decayingDifficultyFactor; }

    const Anim *GetCurrentAnim() { DO_VALIDATION; return &currentAnim; }

    const NodeMap &GetNodeMap() { DO_VALIDATION; return nodeMap; }

    void Hide() { DO_VALIDATION; fullbodyNode->SetPosition(Vector3(1000, 1000, -1000)); hairStyle->SetPosition(Vector3(1000, 1000, -1000)); } // hax ;)

    void SetKit(boost::intrusive_ptr < Resource<Surface> > newKit);

    virtual void ResetSituation(const Vector3 &focusPos);
    void ProcessState(EnvState* state);

  protected:
    bool _HighOrBouncyBall() const;
    void _KeepBestDirectionAnims(DataSet& dataset, const PlayerCommand &command, bool strict = true, radian allowedAngle = 0, int allowedVelocitySteps = 0, int forcedQuadrantID = -1); // ALERT: set sorting predicates before calling this function. strict kinda overrules the allowedstuff
    void _KeepBestBodyDirectionAnims(DataSet& dataset, const PlayerCommand &command, bool strict = true, radian allowedAngle = 0); // ALERT: set sorting predicates before calling this function. strict kinda overrules the allowedstuff
    virtual bool SelectAnim(const PlayerCommand &command, e_InterruptAnim localInterruptAnim, bool preferPassAndShot = false); // returns false on no applicable anim found
    void CalculatePredictedSituation(Vector3 &predictedPos, radian &predictedAngle);
    Vector3 CalculateOutgoingMovement(const std::vector<Vector3> &positions) const;

    void CalculateSpatialState(); // realtime properties, based on 'physics'
    void CalculateFactualSpatialState(); // realtime properties, based on anim. usable at last frame of anim. more riggid than above function

    void AddTripCommandToQueue(PlayerCommandQueue &commandQueue, const Vector3 &tripVector, int tripType);
    PlayerCommand GetTripCommand(const Vector3 &tripVector, int tripType);
    PlayerCommand GetBasicMovementCommand(const Vector3 &desiredDirection, float velocityFloat);

    void SetFootSimilarityPredicate(e_Foot desiredFoot) const;
    bool CompareFootSimilarity(e_Foot foot, int animIndex1, int animIndex2) const;
    void SetIncomingVelocitySimilarityPredicate(e_Velocity velocity) const;
    bool CompareIncomingVelocitySimilarity(int animIndex1, int animIndex2) const;
    void SetMovementSimilarityPredicate(const Vector3 &relDesiredDirection, e_Velocity desiredVelocity) const;
    float GetMovementSimilarity(int animIndex, const Vector3 &relDesiredDirection, e_Velocity desiredVelocity, float corneringBias) const;
    bool CompareMovementSimilarity(int animIndex1, int animIndex2) const;
    bool CompareByOrderFloat(int animIndex1, int animIndex2) const;
    void SetIncomingBodyDirectionSimilarityPredicate(
        const Vector3 &relIncomingBodyDirection) const;
    bool CompareIncomingBodyDirectionSimilarity(int animIndex1, int animIndex2) const;
    void SetBodyDirectionSimilarityPredicate(const Vector3 &lookAt) const;
    real DirectionSimilarityRating(int animIndex) const;
    bool CompareBodyDirectionSimilarity(int animIndex1, int animIndex2) const;
    void SetTripDirectionSimilarityPredicate(const Vector3 &relDesiredTripDirection) const;
    bool CompareTripDirectionSimilarity(int animIndex1, int animIndex2) const;
    bool CompareBaseanimSimilarity(int animIndex1, int animIndex2) const;
    bool CompareCatchOrDeflect(int animIndex1, int animIndex2) const;
    void SetIdlePredicate(float desiredValue) const;
    bool CompareIdleVariable(int animIndex1, int animIndex2) const;
    bool ComparePriorityVariable(int animIndex1, int animIndex2) const;

    Vector3 CalculatePhysicsVector(Animation *anim, bool useDesiredMovement,
                                   const Vector3 &desiredMovement,
                                   bool useDesiredBodyDirection,
                                   const Vector3 &desiredBodyDirectionRel,
                                   std::vector<Vector3> &positions_ret,
                                   radian &rotationOffset_ret) const;

    Vector3 ForceIntoAllowedBodyDirectionVec(const Vector3 &src) const;
    radian ForceIntoAllowedBodyDirectionAngle(radian angle) const; // for making small differences irrelevant while sorting
    Vector3 ForceIntoPreferredDirectionVec(const Vector3 &src) const;
    radian ForceIntoPreferredDirectionAngle(radian angle) const;

    // Seems to be used for rendering only, updated in
    // UpdateFullbodyModel / UpdateFullBodyNodes, Hide() method changes
    // position, so maybe Hide needs to change, otherwise collision detection
    // analysis hidden players?
    boost::intrusive_ptr<Node> fullbodyNode;
    // Modified in PrepareFullBodyModel, not changed later.
    std::vector<FloatArray> uniqueFullbodyMesh;
    // Modified in PrepareFullBodyModel, not changed later.
    std::vector < std::vector<WeightedVertex> > weightedVerticesVec;
    // Modified in PrepareFullBodyModel, not changed later.
    unsigned int fullbodySubgeomCount = 0;
    // Used only for memory releasing.
    std::vector<int*> uniqueIndicesVec;
    // Updated in UpdateFullbodyModel / UpdateFullBodyNodes,
    // snapshot not needed. References nodes point to humanoidNode.
    std::vector<HJoint> joints;
    // Used only for memory management.
    boost::intrusive_ptr<Node> fullbodyTargetNode;
    // Used for ball collision detection. Seems to be the one to snapshot.
    boost::intrusive_ptr<Node> humanoidNode;
    // Updated in UpdateFullbodyNodes, no need to snapshot.
    boost::intrusive_ptr<Geometry> hairStyle;
    // Initiated in the constructor, no need to snapshot.
    std::string kitDiffuseTextureIdentString = "kit_template.png";

    Match *match;
    PlayerBase *player;
    // Shared between all players, no need to snapshot.
    boost::shared_ptr<AnimCollection> anims;
    // Pointers from elements in humanoidNode to Nodes.
    NodeMap nodeMap;
    // Seems to contain current animation context.
    AnimApplyBuffer animApplyBuffer;

    BiasedOffsets offsets;

    Anim currentAnim;
    int previousAnim_frameNum;
    e_FunctionType previousAnim_functionType = e_FunctionType_None;

    // position/rotation offsets at the start of currentAnim
    Vector3 startPos;
    radian startAngle;

    // position/rotation offsets at the end of currentAnim
    Vector3 nextStartPos;
    radian nextStartAngle;

    // realtime info
    SpatialState spatialState;

    Vector3 previousPosition2D;

    e_InterruptAnim interruptAnim;
    int reQueueDelayFrames = 0;
    int tripType = 0;
    Vector3 tripDirection;

    Vector3 decayingPositionOffset;
    float decayingDifficultyFactor = 0.0f;

    // for comparing dataset entries (needed by std::list::sort)
    mutable e_Foot predicate_DesiredFoot;
    mutable e_Velocity predicate_IncomingVelocity;
    mutable Vector3 predicate_RelDesiredDirection;
    mutable Vector3 predicate_DesiredDirection;
    mutable float predicate_CorneringBias = 0.0f;
    mutable e_Velocity predicate_DesiredVelocity;
    mutable Vector3 predicate_RelIncomingBodyDirection;
    mutable Vector3 predicate_LookAt;
    mutable Vector3 predicate_RelDesiredTripDirection;
    mutable Vector3 predicate_RelDesiredBallDirection;
    mutable float predicate_idle = 0.0f;

    // Should be dynamically retrieved from match, don't cache.
    int mentalImageTime = 0;

    const float zMultiplier;
    MovementHistory movementHistory;
    bool mirrored = false;
};

#endif
