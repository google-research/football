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

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_ANIMATION
#define _HPP_ANIMATION

#include "../defines.hpp"

#include "../scene/scene3d/node.hpp"

#include "animationextensions/animationextension.hpp"

#include "../utils/xmlloader.hpp"

namespace blunted {

enum e_DefString {
  e_DefString_Empty = 0,
  e_DefString_OutgoingSpecialState = 1,
  e_DefString_IncomingSpecialState = 2,
  e_DefString_SpecialVar1 = 3,
  e_DefString_SpecialVar2 = 4,
  e_DefString_Type = 5,
  e_DefString_Trap = 6,
  e_DefString_Deflect = 7,
  e_DefString_Interfere = 8,
  e_DefString_Trip = 9,
  e_DefString_ShortPass = 10,
  e_DefString_LongPass = 11,
  e_DefString_Shot = 12,
  e_DefString_Sliding = 13,
  e_DefString_Movement = 14,
  e_DefString_Special = 15,
  e_DefString_BallControl = 16,
  e_DefString_HighPass = 17,
  e_DefString_Catch = 18,
  e_DefString_OutgoingRetainState = 19,
  e_DefString_IncomingRetainState = 20,
  e_DefString_Size = 21
};

  struct KeyFrame {
    Quaternion orientation;
    Vector3 position;
  };

  struct WeighedKey {
    KeyFrame keyFrame;
    float influence = 0.0f; // [0..1]
    int frame = 0;
  };

  struct NodeAnimation {
    std::string nodeName;
    std::map<int, KeyFrame> animation; // frame, angles
  };

  enum e_Foot {
    e_Foot_Left,
    e_Foot_Right
  };

  struct BiasedOffset {
    BiasedOffset() {
      bias = 0.0f;
      isRelative = false;
    }
    float bias = 0.0f; // 0 .. 1
    Quaternion orientation;
    bool isRelative = false;
  };

  static std::map < std::string, BiasedOffset > emptyOffsets;

  struct MovementHistoryEntry {
    std::string nodeName;
    Vector3 position;
    Quaternion orientation;
    int timeDiff_ms = 0;
  };

  typedef std::vector<MovementHistoryEntry> MovementHistory;


  // usage
  //
  // rules:
  //   - first node inserted (by using SetKeyFrame) should be root node of the animated object (for optimization purposes)

  class Animation {

    public:
      Animation();
      Animation(const Animation &src); // attention! this does not deep copy extensions!
      virtual ~Animation();

      void DirtyCache(); // hee hee

      int GetFrameCount() const;
      int GetEffectiveFrameCount() const { return GetFrameCount() - 1; }

      bool GetKeyFrame(std::string nodeName, int frame, Quaternion &orientation, Vector3 &position, bool getOrientation = true, bool getPosition = true) const;
      void SetKeyFrame(std::string nodeName, int frame,
                       const Quaternion &orientation,
                       const Vector3 &position = Vector3(0, 0, 0));
      void GetInterpolatedValues(const std::map<int, KeyFrame> &animation,
                                 int frame, Quaternion &orientation,
                                 Vector3 &position, bool getOrientation = true,
                                 bool getPosition = true) const;
      void ConvertToStartFacingForwardIfIdle();
      void Apply(const std::map<const std::string, boost::intrusive_ptr<Node> >
                     nodeMap,
                 int frame, int timeOffset_ms = 0, bool smooth = true,
                 float smoothFactor = 1.0f,
                 /*const boost::shared_ptr<Animation> previousAnimation, int
                    smoothFrames, */
                 const Vector3 &basePos = Vector3(0), radian baseRot = 0,
                 std::map<std::string, BiasedOffset> &offsets = emptyOffsets,
                 MovementHistory *movementHistory = 0, int timeDiff_ms = 10,
                 bool noPos = false, bool updateSpatial = true);

      // returns end position - start position
      Vector3 GetTranslation() const;
      Vector3 GetIncomingMovement() const;
      float GetIncomingVelocity() const;
      Vector3 GetOutgoingMovement() const;
      Vector3 GetOutgoingDirection() const;
      Vector3 GetIncomingBodyDirection() const;
      Vector3 GetOutgoingBodyDirection() const;
      float GetOutgoingVelocity() const;
      radian GetOutgoingAngle() const;
      radian GetIncomingBodyAngle() const;
      radian GetOutgoingBodyAngle() const;
      e_Foot GetCurrentFoot() const { return currentFoot; }
      e_Foot GetOutgoingFoot() const;

      void Reset();
      void LoadData(std::vector < std::vector<std::string> > &file);
      void Load(const std::string &filename);
      void Mirror();
      std::string GetName() const;
      void SetName(const std::string &name) { this->name = name; }

      void AddExtension(const std::string &name, boost::shared_ptr<AnimationExtension> extension);
      boost::shared_ptr<AnimationExtension> GetExtension(const std::string &name);

      const std::string &GetVariable(const char *name) const;
      void SetVariable(const std::string &name, const std::string &value);
      e_DefString GetAnimType() const { return cache_AnimType; }
      const std::string &GetAnimTypeStr() const { return cache_AnimType_str; }

      std::vector<NodeAnimation *> &GetNodeAnimations() {
        return nodeAnimations;
      }

    protected:
      std::vector<NodeAnimation*> nodeAnimations;
      int frameCount = 0;
      std::string name;

      std::map < std::string, boost::shared_ptr<AnimationExtension> > extensions;

      boost::shared_ptr<XMLTree> customData;
      std::map<std::string, std::string> variableCache;

      // this hack only applies to humanoids
      // it's which foot is moving first in this anim
      e_Foot currentFoot;

      mutable bool cache_translation_dirty = false;
      mutable Vector3 cache_translation;
      mutable bool cache_incomingMovement_dirty = false;
      mutable Vector3 cache_incomingMovement;
      mutable bool cache_incomingVelocity_dirty = false;
      mutable float cache_incomingVelocity = 0.0f;
      mutable bool cache_outgoingDirection_dirty = false;
      mutable Vector3 cache_outgoingDirection;
      mutable bool cache_outgoingMovement_dirty = false;
      mutable Vector3 cache_outgoingMovement;
      mutable bool cache_rangedOutgoingMovement_dirty = false;
      mutable Vector3 cache_rangedOutgoingMovement;
      mutable bool cache_outgoingVelocity_dirty = false;
      mutable float cache_outgoingVelocity = 0.0f;
      mutable bool cache_angle_dirty = false;
      mutable radian cache_angle;
      mutable bool cache_incomingBodyAngle_dirty = false;
      mutable radian cache_incomingBodyAngle;
      mutable bool cache_outgoingBodyAngle_dirty = false;
      mutable radian cache_outgoingBodyAngle;
      mutable bool cache_incomingBodyDirection_dirty = false;
      mutable Vector3 cache_incomingBodyDirection;
      mutable bool cache_outgoingBodyDirection_dirty = false;
      mutable Vector3 cache_outgoingBodyDirection;

      e_DefString cache_AnimType;
      std::string cache_AnimType_str;

  };

}

#endif
