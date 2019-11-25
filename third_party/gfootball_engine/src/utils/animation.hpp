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
#include <iostream>

#include "../scene/scene3d/node.hpp"

#include "animationextensions/animationextension.hpp"

#include "../utils/xmlloader.hpp"
#include "../gamedefines.hpp"

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


e_FunctionType StringToFunctionType(e_DefString fun);

  struct KeyFrame {
    Quaternion orientation;
    Vector3 position;
    bool operator<(const KeyFrame& a) const {
      return position < a.position;
    }
  };

  struct WeighedKey {
    const KeyFrame* keyFrame;
    float influence = 0.0f; // [0..1]
    int frame = 0;
  };

  struct KeyFrames {
    std::vector<std::pair<int, KeyFrame>> d;
    void clear() { DO_VALIDATION;
      d.clear();
    }
    KeyFrame* getFrame(int frame) { DO_VALIDATION;
      for (auto& i : d) { DO_VALIDATION;
        if (i.first == frame) { DO_VALIDATION;
          return &i.second;
        }
      }
      return nullptr;
    }
    void addFrame(const std::pair<int, KeyFrame>& frame) { DO_VALIDATION;
      d.push_back(frame);
      std::sort(d.begin(), d.end());
    }
  };

  enum BodyPart {
    middle,
    neck,
    left_thigh,
    right_thigh,
    left_knee,
    right_knee,
    left_ankle,
    right_ankle,
    left_shoulder,
    right_shoulder,
    left_elbow,
    right_elbow,
    body,
    player,
    body_part_max
  };

  typedef boost::intrusive_ptr<Node> NodeMap[body_part_max];

  static std::string BodyPartString(BodyPart part) { DO_VALIDATION;
    switch(part) { DO_VALIDATION;
      case middle:
        return "middle";
      case neck:
        return "neck";
      case left_thigh:
        return "left_thigh";
      case right_thigh:
        return "right_thigh";
      case left_knee:
        return "left_knee";
      case right_knee:
        return "right_knee";
      case left_ankle:
        return "left_ankle";
      case right_ankle:
        return "right_ankle";
      case left_shoulder:
        return "left_shoulder";
      case right_shoulder:
        return "right_shoulder";
      case left_elbow:
        return "left_elbow";
      case right_elbow:
        return "right_elbow";
      case body:
        return "body";
      case player:
        return "player";
      case body_part_max:
        return "body_part_max";
      default:
        Log(e_FatalError, "", "", "Body part not known");
    }
    return "";
  }

  static BodyPart BodyPartFromString(const std::string part) { DO_VALIDATION;
    if (part == "middle")
      return middle;
    if (part == "neck")
      return neck;
    if (part == "left_thigh")
      return left_thigh;
    if (part == "right_thigh")
      return right_thigh;
    if (part == "left_knee")
      return left_knee;
    if (part == "right_knee")
      return right_knee;
    if (part == "left_ankle")
      return left_ankle;
    if (part == "right_ankle")
      return right_ankle;
    if (part == "left_shoulder")
      return left_shoulder;
    if (part == "right_shoulder")
      return right_shoulder;
    if (part == "left_elbow")
      return left_elbow;
    if (part == "right_elbow")
      return right_elbow;
    if (part == "body")
      return body;
    if (part == "player")
      return player;
    Log(e_FatalError, "", "", "Body part not known: '" + part + "'");
    return body_part_max;
  }

  struct NodeAnimation {
    BodyPart nodeName;
    KeyFrames animation; // frame, angles
  };

  enum e_Foot {
    e_Foot_Left,
    e_Foot_Right
  };

  struct BiasedOffset {
    Quaternion orientation;
    float bias = 0.0f; // 0 .. 1
    void ProcessState(EnvState* state) { DO_VALIDATION;
      state->process(orientation);
      state->process(bias);
    }
  };

  struct BiasedOffsets {
   public:
    BiasedOffsets() { DO_VALIDATION;
    }
    BiasedOffsets(const BiasedOffsets &obj) { DO_VALIDATION;
      for (int x = 0; x < body_part_max; x++) { DO_VALIDATION;
        elements[x] = obj.elements[x];
      }
    }
    void clear() { DO_VALIDATION;
    }
    inline BiasedOffset& operator[](BodyPart part) { DO_VALIDATION;
      return elements[part];
    }
    void ProcessState(EnvState* state) { DO_VALIDATION;
      for (auto& el : elements) { DO_VALIDATION;
        el.ProcessState(state);
      }
      state->process((void*) elements, sizeof(elements));
    }
  private:
    BiasedOffset elements[body_part_max];
  };

  static BiasedOffsets emptyOffsets;

  struct MovementHistoryEntry {
    Vector3 position;
    Quaternion orientation;
    int timeDiff_ms = 0;
    BodyPart nodeName;
    void ProcessState(EnvState* state) { DO_VALIDATION;
      state->process(position);
      state->process(orientation);
      state->process(timeDiff_ms);
      state->process(&nodeName, sizeof(nodeName));
    }
  };

  typedef std::vector<MovementHistoryEntry> MovementHistory;


  // usage
  //
  // rules:
  //   - first node inserted (by using SetKeyFrame) should be root node of the animated object (for optimization purposes)

  class VariableCache {
   public:
    std::string get(const std::string& key) const {
      auto iter = values.find(key);
      if (iter != values.end()) { DO_VALIDATION;
        return iter->second;
      } else {
        return "";
      }
    }
    void set(const std::string& key, const std::string& value) { DO_VALIDATION;
      values[key] = value;
      if (key == "idlelevel") { DO_VALIDATION;
        _idlelevel = atof(value.c_str());
      } else if (key == "quadrant_id") { DO_VALIDATION;
        _quadrant_id = atoi(value.c_str());
      } else if (key == "specialvar1") { DO_VALIDATION;
        _specialvar1 = atof(value.c_str());
      } else if (key == "specialvar2") { DO_VALIDATION;
        _specialvar2 = atof(value.c_str());
      } else if (key == "lastditch") { DO_VALIDATION;
        _lastditch = value.compare("true") == 0;
      } else if (key == "baseanim") { DO_VALIDATION;
        _baseanim = value.compare("true") == 0;
      } else if (key == "outgoing_special_state") { DO_VALIDATION;
        _outgoing_special_state = value;
      } else if (key == "incoming_retain_state") { DO_VALIDATION;
        _incoming_retain_state = value;
      } else if (key == "incoming_special_state") { DO_VALIDATION;
        _incoming_special_state = value;
      }
    }

    void set_specialvar1(float v) { DO_VALIDATION;
      _specialvar1 = v;
    }

    void set_specialvar2(float v) { DO_VALIDATION;
      _specialvar2 = v;
    }

    void mirror() { DO_VALIDATION;
      for (auto varIter : values) { DO_VALIDATION;
        mirror(varIter.second);
      }
      mirror(_outgoing_special_state);
      mirror(_incoming_retain_state);
      mirror(_incoming_special_state);
    }

    inline float idlelevel() const { return _idlelevel; }
    inline int quadrant_id() const { return _quadrant_id; }
    inline float specialvar1() const { return _specialvar1; }
    inline float specialvar2() const { return _specialvar2; }
    inline bool lastditch() const { return _lastditch; }
    inline bool baseanim() const { return _baseanim; }
    inline const std::string& outgoing_special_state() const { return _outgoing_special_state; }
    inline const std::string& incoming_retain_state() const { return _incoming_retain_state; }
    inline const std::string& incoming_special_state() const { return _incoming_special_state; }

   private:
    void mirror(std::string& varData) { DO_VALIDATION;
      if (varData.substr(0, 4) == "left") { DO_VALIDATION;
        varData = varData.replace(0, 4, "right");
      } else if (varData.substr(0, 5) == "right") { DO_VALIDATION;
        varData = varData.replace(0, 5, "left");
      }
    }

    float _idlelevel = 0;
    int _quadrant_id = 0;
    float _specialvar1 = 0;
    float _specialvar2 = 0;
    bool _lastditch = false;
    bool _baseanim = false;
    std::string _outgoing_special_state;
    std::string _incoming_retain_state;
    std::string _incoming_special_state;
    std::map<std::string, std::string> values;
  };

  class Animation {

    public:
      Animation();
      Animation(const Animation &src); // attention! this does not deep copy extensions!
      virtual ~Animation();

      void DirtyCache(); // hee hee

      int GetFrameCount() const;
      int GetEffectiveFrameCount() const { return GetFrameCount() - 1; }

      bool GetKeyFrame(BodyPart nodeName, int frame, Quaternion &orientation, Vector3 &position) const;
      void SetKeyFrame(BodyPart nodeName, int frame,
                       const Quaternion &orientation,
                       const Vector3 &position = Vector3(0, 0, 0));
      void GetInterpolatedValues(const KeyFrames &animation,
                                 int frame, Quaternion &orientation,
                                 Vector3 &position) const;
      void ConvertToStartFacingForwardIfIdle();
      void Apply(const NodeMap& nodeMap,
                 int frame, int timeOffset_ms = 0, bool smooth = true,
                 float smoothFactor = 1.0f,
                 /*const boost::shared_ptr<Animation> previousAnimation, int
                    smoothFrames, */
                 const Vector3 &basePos = Vector3(0), radian baseRot = 0,
                 BiasedOffsets &offsets = emptyOffsets,
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
      void SetName(const std::string &name) { DO_VALIDATION; this->name = name; }

      void AddExtension(const std::string &name, boost::shared_ptr<AnimationExtension> extension);
      boost::shared_ptr<AnimationExtension> GetExtension(const std::string &name);

      const std::string GetVariable(const char *name) const;
      const VariableCache& GetVariableCache() const {
        return variableCache;
      }
      void SetVariable(const std::string &name, const std::string &value);
      e_DefString GetAnimType() const { return cache_AnimType; }

      std::vector<NodeAnimation *> &GetNodeAnimations() { DO_VALIDATION;
        return nodeAnimations;
      }
      mutable float order_float = 0;
      void ProcessState(EnvState* state);

    protected:
      std::vector<NodeAnimation*> nodeAnimations;
      int frameCount = 0;
      std::string name;

      std::map < std::string, boost::shared_ptr<AnimationExtension> > extensions;

      boost::shared_ptr<XMLTree> customData;
      VariableCache variableCache;

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
  };

}

#endif
