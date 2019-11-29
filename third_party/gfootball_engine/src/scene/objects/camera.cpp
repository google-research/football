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

#include "camera.hpp"

#include "../../systems/isystemobject.hpp"

#include "../../scene/objects/light.hpp"
#include "../../scene/objects/geometry.hpp"
#include "../../scene/objects/skybox.hpp"

namespace blunted {

Camera::Camera(std::string name) : Object(name, e_ObjectType_Camera) {
  DO_VALIDATION;
}

Camera::~Camera() { DO_VALIDATION; }

void Camera::Init() {
  DO_VALIDATION;  // ATOMIC

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    CameraInterpreter->OnLoad(properties);
  }
}

void Camera::Exit() {
  DO_VALIDATION;  // ATOMIC

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    CameraInterpreter->OnUnload();
  }

  Object::Exit();
}

void Camera::SetFOV(float fov) {
  DO_VALIDATION;

  this->fov = fov;

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    CameraInterpreter->SetFOV(fov);
  }
}

void Camera::SetCapping(float nearCap, float farCap) {
  DO_VALIDATION;

  this->nearCap = nearCap;
  this->farCap = farCap;

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    CameraInterpreter->SetCapping(nearCap, farCap);
  }
}

void Camera::EnqueueView(
    std::deque<boost::intrusive_ptr<Geometry> > &visibleGeometry,
    std::deque<boost::intrusive_ptr<Light> > &visibleLights,
    std::deque<boost::intrusive_ptr<Skybox> > &skyboxes) {
  DO_VALIDATION;

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    CameraInterpreter->EnqueueView(GetName(), visibleGeometry, visibleLights,
                                   skyboxes);
  }
}

void Camera::Poke(e_SystemType targetSystemType) {
  DO_VALIDATION;

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    ICameraInterpreter *CameraInterpreter =
        static_cast<ICameraInterpreter *>(observers[i].get());
    if (CameraInterpreter->GetSystemType() == targetSystemType)
      CameraInterpreter->OnPoke();
  }
}

void Camera::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType,
                                        e_SystemType excludeSystem) {
  DO_VALIDATION;
  InvalidateSpatialData();

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    if (observers[i]->GetSystemType() != excludeSystem) {
      DO_VALIDATION;
      ICameraInterpreter *cameraInterpreter =
          static_cast<ICameraInterpreter *>(observers[i].get());
      cameraInterpreter->OnSpatialChange(GetDerivedPosition(),
                                         GetDerivedRotation());
    }
  }
}
}
