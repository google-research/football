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

#include "joint.hpp"

#include "geometry.hpp"

#include "../../systems/isystemobject.hpp"

namespace blunted {

  Joint::Joint(std::string name) : Object(name, e_ObjectType_Joint) {
  }

  Joint::~Joint() {
  }

  void Joint::Exit() { // ATOMIC
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->OnUnload();
    }

    Object::Exit();
  }

  void Joint::Connect(boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->OnLoad(this, object1, object2, anchor, axis1, axis2);
    }
  }

  void Joint::SetStops(radian lowStop, radian highStop, int paramNum) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetStops(lowStop, highStop, paramNum);
    }
  }

  // desired velocity
  void Joint::SetVelocity(float velocity, int paramNum) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetVelocity(velocity, paramNum);
    }
  }

  // maximum force to reach the desired velocity
  void Joint::SetMaxForce(float force, int paramNum) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetMaxForce(force, paramNum);
    }
  }

  void Joint::SetConstraintForceMixing(float value, int paramNum) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetConstraintForceMixing(value, paramNum);
    }
  }

  void Joint::SetErrorCorrection(float value, int paramNum) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetErrorCorrection(value, paramNum);
    }
  }

  // only on hinge2 joints
  void Joint::SetSuspensionConstraintForceMixing(float value) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetSuspensionConstraintForceMixing(value);
    }
  }

  void Joint::SetSuspensionErrorReduction(float value) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->SetSuspensionErrorReduction(value);
    }
  }

  void Joint::Poke(e_SystemType targetSystemType) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      if (jointInterpreter->GetSystemType() == targetSystemType) jointInterpreter->OnPoke();
    }
  }

  void Joint::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    // that's odd, no code here, only this commented block. I don't know either.
/*

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers[i].get());
      jointInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
    }

*/
  }

}
