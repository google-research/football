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

struct Joint {
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
  RotationSmuggle() {
    begin = 0;
    end = 0;
  }
  void operator = (const float &value) {
    begin = value;
    end = value;
  }
  radian begin;
  radian end;
};

struct Anim {

  Anim() {
    id = 0;
    frameNum = 0;

    rotationSmuggle.begin = 0;
    rotationSmuggle.end = 0;
    rotationSmuggleOffset = 0;
    touchFrame = -1;
    radiusOffset = 0.0;
  }

  Animation *anim;
  signed int id = 0;
  int frameNum = 0;

  e_FunctionType functionType;

  e_InterruptAnim originatingInterrupt;

  Vector3 fullActionSmuggle; // without cheatdiscarddistance
  Vector3 actionSmuggle;
  Vector3 actionSmuggleOffset;
  Vector3 actionSmuggleSustain;
  Vector3 actionSmuggleSustainOffset;
  Vector3 movementSmuggle;
  Vector3 movementSmuggleOffset;
  RotationSmuggle rotationSmuggle;
  radian rotationSmuggleOffset;
  signed int touchFrame = 0;
  float radiusOffset = 0.0f;
  Vector3 touchPos;

  Vector3 incomingMovement;
  Vector3 outgoingMovement;

  Vector3 positionOffset;

  PlayerCommand originatingCommand;

  std::vector<Vector3> positions;
};

struct AnimApplyBuffer {
  AnimApplyBuffer() {
    frameNum = 0;
    snapshotTime_ms = 0;
    smooth = true;
    smoothFactor = 0.5f;
    noPos = false;
    orientation = 0;
  }
  AnimApplyBuffer(const AnimApplyBuffer &src) {
    anim = src.anim;
    frameNum = src.frameNum;
    snapshotTime_ms = src.snapshotTime_ms;
    smooth = src.smooth;
    smoothFactor = src.smoothFactor;
    noPos = src.noPos;
    position = src.position;
    orientation = src.orientation;
    offsets = src.offsets;
  }
  Animation *anim = 0;
  int frameNum = 0;
  unsigned long snapshotTime_ms = 0;
  bool smooth = false;
  float smoothFactor = 0.0f;
  bool noPos = false;
  Vector3 position;
  radian orientation;
  BiasedOffsets offsets;
};

struct TemporalHumanoidNode {
  boost::intrusive_ptr<Node> actualNode;
  Vector3 cachedPosition;
  Quaternion cachedOrientation;
  TemporalSmoother<Vector3> position;
  TemporalSmoother<Quaternion> orientation;
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
};

static const Vector3 emptyVec(0);

class HumanoidBase {

  public:
    HumanoidBase(PlayerBase *player, Match *match, boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::shared_ptr<AnimCollection> animCollection, boost::intrusive_ptr<Node> fullbodyTargetNode, boost::intrusive_ptr < Resource<Surface> > kit, int bodyUpdatePhaseOffset);
    virtual ~HumanoidBase();

    void PrepareFullbodyModel(std::map<Vector3, Vector3> &colorCoords);
    void UpdateFullbodyNodes();
    bool NeedsModelUpdate();
    void UpdateFullbodyModel(bool updateSrc = false);

    virtual void Process();
    void PreparePutBuffers(unsigned long snapshotTime_ms);
    void FetchPutBuffers(unsigned long putTime_ms);
    void Put();

    virtual void CalculateGeomOffsets();
    void SetOffset(BodyPart body_part, float bias, const Quaternion &orientation, bool isRelative = false);

    inline int GetFrameNum() { return currentAnim->frameNum; }
    inline int GetFrameCount() { return currentAnim->anim->GetFrameCount(); }

    inline Vector3 GetPosition() const { return spatialState.position; }
    inline Vector3 GetDirectionVec() const { return spatialState.directionVec; }
    inline Vector3 GetBodyDirectionVec() const {
      return spatialState.bodyDirectionVec;
    }
    inline radian GetRelBodyAngle() const { return spatialState.relBodyAngle; }
    inline e_Velocity GetEnumVelocity() const { return spatialState.enumVelocity; }
    inline e_FunctionType GetCurrentFunctionType() const { return currentAnim->functionType; }
    inline e_FunctionType GetPreviousFunctionType() const { return previousAnim->functionType; }
    inline Vector3 GetMovement() const { return spatialState.movement; }

    Vector3 GetGeomPosition() { return humanoidNode->GetPosition(); }

    int GetIdleMovementAnimID();
    void ResetPosition(const Vector3 &newPos, const Vector3 &focusPos);
    void OffsetPosition(const Vector3 &offset);
    void TripMe(const Vector3 &tripVector, int tripType);

    boost::intrusive_ptr<Node> GetHumanoidNode() { return humanoidNode; }
    boost::intrusive_ptr<Node> GetFullbodyNode() { return fullbodyNode; }

    virtual float GetDecayingPositionOffsetLength() const { return decayingPositionOffset.GetLength(); }
    virtual float GetDecayingDifficultyFactor() const { return decayingDifficultyFactor; }

    const Anim *GetCurrentAnim() { return currentAnim; }

    const NodeMap &GetNodeMap() { return nodeMap; }

    void Hide() { fullbodyNode->SetPosition(Vector3(1000, 1000, -1000)); hairStyle->SetPosition(Vector3(1000, 1000, -1000)); } // hax ;)

    void SetKit(boost::intrusive_ptr < Resource<Surface> > newKit);

    virtual void ResetSituation(const Vector3 &focusPos);

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
    bool CompareFootSimilarity(int animIndex1, int animIndex2) const;
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

    boost::intrusive_ptr<Node> fullbodyNode;
    std::vector<FloatArray> uniqueFullbodyMesh;
    std::vector < std::vector<WeightedVertex> > weightedVerticesVec; // < subgeoms < vertices > >
    unsigned int fullbodySubgeomCount = 0;
    std::vector<int*> uniqueIndicesVec;
    std::vector<Joint> joints;
    Vector3 fullbodyOffset;
    boost::intrusive_ptr<Node> fullbodyTargetNode;

    boost::intrusive_ptr<Node> humanoidNode;
    boost::shared_ptr<Scene3D> scene3D;

    boost::intrusive_ptr<Geometry> hairStyle;

    std::string kitDiffuseTextureIdentString;

    Match *match;
    PlayerBase *player;

    boost::shared_ptr<AnimCollection> anims;
    NodeMap nodeMap;

    AnimApplyBuffer animApplyBuffer;

    AnimApplyBuffer buf_animApplyBuffer;

    std::vector<TemporalHumanoidNode> buf_TemporalHumanoidNodes;

    bool buf_LowDetailMode = false;
    int buf_bodyUpdatePhase = 0;
    int buf_bodyUpdatePhaseOffset = 0;

    AnimApplyBuffer fetchedbuf_animApplyBuffer;

    unsigned long fetchedbuf_previousSnapshotTime_ms = 0;

    bool fetchedbuf_LowDetailMode = false;
    int fetchedbuf_bodyUpdatePhase = 0;
    int fetchedbuf_bodyUpdatePhaseOffset = 0;

    BiasedOffsets offsets;

    Anim *currentAnim;
    Anim *previousAnim;

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

    const MentalImage *currentMentalImage;

    float _cache_AgilityFactor = 0.0f;
    float _cache_AccelerationFactor = 0.0f;

    float zMultiplier = 0.0f;

    std::vector<Vector3> allowedBodyDirVecs;
    std::vector<radian> allowedBodyDirAngles;
    std::vector<Vector3> preferredDirectionVecs;
    std::vector<radian> preferredDirectionAngles;

    MovementHistory movementHistory;

};

#endif
