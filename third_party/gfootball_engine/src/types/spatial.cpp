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

#include "spatial.hpp"

#include "../scene/scene3d/node.hpp"

namespace blunted {

Spatial::Spatial(const std::string &name)
    : name(name), parent(0), localMode(e_LocalMode_Relative) {
  DO_VALIDATION;
  scale.Set(1, 1, 1);
  Vector axis(0, 0, -1);
  rotation.SetAngleAxis(0, axis);
  position.Set(0, 0, 0);
  aabb.aabb.Reset();
  aabb.dirty = false;
  InvalidateSpatialData();
}

Spatial::~Spatial() {
  DO_VALIDATION;
  parent = 0;
}

Spatial::Spatial(const Spatial &src) {
  DO_VALIDATION;
  name = src.GetName();
  position = src.position;
  rotation = src.rotation;
  scale = src.scale;
  localMode = src.localMode;
  aabb.aabb = src.GetAABB();
  aabb.dirty = false;
  parent = 0;
  InvalidateSpatialData();
}

void Spatial::SetLocalMode(e_LocalMode localMode) {
  DO_VALIDATION;
  this->localMode = localMode;
  InvalidateBoundingVolume();
}

e_LocalMode Spatial::GetLocalMode() {
  DO_VALIDATION;
  return localMode;
}

void Spatial::SetName(const std::string &name) {
  DO_VALIDATION;
  this->name = name;
}

  const std::string Spatial::GetName() const {
    return name.c_str();
  }

  void Spatial::SetParent(Node *parent) {
    DO_VALIDATION;
    this->parent = parent;
    InvalidateBoundingVolume();
  }

  void Spatial::SetPosition(const Vector3 &newPosition,
                            bool updateSpatialData) {
    DO_VALIDATION;
    position = newPosition;
    if (updateSpatialData) RecursiveUpdateSpatialData(e_SpatialDataType_Position);
  }

  Vector3 Spatial::GetPosition() const {
    return position;
  }

  void Spatial::SetRotation(const Quaternion &newRotation,
                            bool updateSpatialData) {
    rotation = newRotation;
    if (updateSpatialData) RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }

  Quaternion Spatial::GetRotation() const {
    return rotation;
  }

  void Spatial::SetScale(const Vector3 &newScale) {
    DO_VALIDATION;
    this->scale = newScale;
    RecursiveUpdateSpatialData(e_SpatialDataType_Rotation);
  }

  Vector3 Spatial::GetScale() const {
    Vector3 retScale = scale;
    return retScale;
  }

  Vector3 Spatial::GetDerivedPosition() const {
    if (_dirty_DerivedPosition) {
      DO_VALIDATION;
      if (localMode == e_LocalMode_Relative) {
        DO_VALIDATION;
        if (parent) {
          DO_VALIDATION;
          const Quaternion parentDerivedRotation = parent->GetDerivedRotation();
          const Vector3 parentDerivedScale = parent->GetDerivedScale();
          const Vector3 parentDerivedPosition = parent->GetDerivedPosition();

          _cache_DerivedPosition.Set(parentDerivedRotation * (parentDerivedScale * GetPosition()));
          _cache_DerivedPosition += parentDerivedPosition;
        } else {
          _cache_DerivedPosition = GetPosition();
        }
      } else {
        _cache_DerivedPosition = GetPosition();
      }
      _dirty_DerivedPosition = false;
    }
    return _cache_DerivedPosition;
  }

  Quaternion Spatial::GetDerivedRotation() const {
    if (_dirty_DerivedRotation) {
      DO_VALIDATION;
      if (localMode == e_LocalMode_Relative) {
        DO_VALIDATION;
        if (parent) {
          DO_VALIDATION;
          _cache_DerivedRotation = (parent->GetDerivedRotation() * GetRotation()).GetNormalized();
        } else {
          _cache_DerivedRotation = GetRotation();
        }
      } else {
        _cache_DerivedRotation = GetRotation();
      }
      _dirty_DerivedRotation = false;
    }
    return _cache_DerivedRotation;
  }

  Vector3 Spatial::GetDerivedScale() const {
    if (_dirty_DerivedScale) {
      DO_VALIDATION;
      if (localMode == e_LocalMode_Relative) {
        DO_VALIDATION;
        if (parent) {
          DO_VALIDATION;
          _cache_DerivedScale = parent->GetDerivedScale() * GetScale();
        } else {
          _cache_DerivedScale = GetScale();
        }
      } else {
        _cache_DerivedScale = GetScale();
      }
      _dirty_DerivedScale = false;
    }
    return _cache_DerivedScale;
  }

  void Spatial::InvalidateBoundingVolume() {
    bool changed = false;
    if (aabb.dirty == false) {
      aabb.dirty = true;
      aabb.aabb.Reset();
      changed = true;
    }
    if (changed) if (parent) parent->InvalidateBoundingVolume();
  }

  void Spatial::InvalidateSpatialData() {
    DO_VALIDATION;
    _dirty_DerivedPosition = true;
    _dirty_DerivedRotation = true;
    _dirty_DerivedScale = true;
  }

  AABB Spatial::GetAABB() const {
    AABB tmp;
    tmp = aabb.aabb;
    return tmp;
  }

}
