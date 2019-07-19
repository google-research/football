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

#include "light.hpp"

#include "camera.hpp"
#include "geometry.hpp"

#include "../../systems/isystemobject.hpp"

namespace blunted {

  Light::Light(std::string name) : Object(name, e_ObjectType_Light) {
    radius = 512;
    color.Set(1, 1, 1);
    lightType = e_LightType_Point;
    shadow = false;
  }

  Light::~Light() {
  }

  void Light::Exit() { // ATOMIC

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      LightInterpreter->OnUnload();
    }

    Object::Exit();

  }

  void Light::SetColor(const Vector3 &color) {
    this->color = color;
    UpdateValues();
  }

  Vector3 Light::GetColor() const {
    Vector3 retColor = color;
    return retColor;

  }

  void Light::SetRadius(float radius) {
    this->radius = radius;
    UpdateValues();

    InvalidateBoundingVolume();
  }

  float Light::GetRadius() const {
    float rad = radius;
    return rad;
  }

  void Light::SetType(e_LightType lightType) {

    this->lightType = lightType;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      LightInterpreter->SetType(lightType);
    }

  }

  e_LightType Light::GetType() const {
    e_LightType theType = lightType;
    return theType;
  }

  void Light::SetShadow(bool shadow) {

    this->shadow = shadow;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      LightInterpreter->SetShadow(shadow);
    }

  }

  bool Light::GetShadow() const {
    return shadow;
  }

  void Light::UpdateValues() {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      LightInterpreter->SetValues(color, radius);
    }

  }

  void Light::EnqueueShadowMap(boost::intrusive_ptr<Camera> camera, std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      LightInterpreter->EnqueueShadowMap(camera, visibleGeometry);
    }

  }

  void Light::Poke(e_SystemType targetSystemType) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
      if (LightInterpreter->GetSystemType() == targetSystemType) LightInterpreter->OnPoke();
    }

  }

  void Light::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    InvalidateSpatialData();
    InvalidateBoundingVolume();


    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      if (observers[i]->GetSystemType() != excludeSystem) {
        ILightInterpreter *lightInterpreter = static_cast<ILightInterpreter*>(observers[i].get());
        lightInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
      }
    }

  }

  AABB Light::GetAABB() const {
    //aabb.Lock();
    if (aabb.dirty == true) {
      Vector3 pos = GetDerivedPosition();
      aabb.aabb.minxyz = pos - radius;
      aabb.aabb.maxxyz = pos + radius;
      aabb.aabb.MakeDirty();
      aabb.dirty = false;
    }
    AABB tmp = aabb.aabb;
    //aabb.Unlock();
    return tmp;
  }

}
