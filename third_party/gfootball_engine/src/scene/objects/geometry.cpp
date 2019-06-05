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

#include "geometry.hpp"

#include "../../base/log.hpp"

#include "../../systems/isystemobject.hpp"

#include "../../managers/resourcemanagerpool.hpp"

namespace blunted {

  Geometry::Geometry(std::string name, e_ObjectType objectType) : Object(name, objectType) {
  }

  Geometry::Geometry(const Geometry &src, const std::string &postfix) : Object(src) {
    if (src.geometryData != boost::intrusive_ptr < Resource<GeometryData> >()) {
      std::string srcName = src.geometryData->GetIdentString();
      bool alreadyThere = false;


      geometryData = (ResourceManagerPool::getGeometryManager()->FetchCopy(srcName, srcName + postfix, alreadyThere));
      //geometryData = (ResourceManagerPool::getGeometryManager()->Fetch(srcName, false, alreadyThere, true));
    }
    InvalidateBoundingVolume();
  }

  Geometry::~Geometry() {
  }

  void Geometry::Exit() { // ATOMIC

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnUnload();
    }

    Object::Exit();

    if (geometryData) geometryData.reset();


    InvalidateBoundingVolume();
  }

  void Geometry::SetGeometryData(boost::intrusive_ptr < Resource<GeometryData> > geometryData) {


    this->geometryData = geometryData;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnLoad(this);
    }


    InvalidateBoundingVolume();
  }

  void Geometry::OnUpdateGeometryData(bool updateMaterials) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnUpdateGeometry(this, updateMaterials);
    }


    InvalidateBoundingVolume();
  }

  boost::intrusive_ptr < Resource<GeometryData> > Geometry::GetGeometryData() {
    return geometryData;
  }

  void Geometry::Poke(e_SystemType targetSystemType) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {

      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      if (geometryInterpreter->GetSystemType() == targetSystemType) geometryInterpreter->OnPoke();
    }


    // did a system object feedback a new pos/rot?
    updateSpatialDataAfterPoke.Lock();

    if (updateSpatialDataAfterPoke.data.haveTo == true) {
      RecursiveUpdateSpatialData(e_SpatialDataType_Both, updateSpatialDataAfterPoke.data.excludeSystem);
      MustUpdateSpatialData clear;
      clear.haveTo = false;
      clear.excludeSystem = e_SystemType_None;
      updateSpatialDataAfterPoke.data = clear;
    }
    updateSpatialDataAfterPoke.Unlock();
  }

  void Geometry::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {

    InvalidateSpatialData();
    InvalidateBoundingVolume();


    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      //printf("%i %i\n", observers.at(i)->GetSystemType(), excludeSystem);
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
        if (spatialDataType == e_SpatialDataType_Position) {
          geometryInterpreter->OnMove(GetDerivedPosition());
        }
        else if (spatialDataType == e_SpatialDataType_Rotation) {
          // need to update both: position relies on rotation
          geometryInterpreter->OnMove(GetDerivedPosition());
          geometryInterpreter->OnRotate(GetDerivedRotation());
        }
        else if (spatialDataType == e_SpatialDataType_Both) {
          geometryInterpreter->OnMove(GetDerivedPosition());
          geometryInterpreter->OnRotate(GetDerivedRotation());
        }
      }
    }


    //Object::RecursiveUpdateSpatialData(spatialDataType);
  }

  AABB Geometry::GetAABB() const {

    //aabb.Lock();

    if (aabb.dirty == true) {
      assert(geometryData->GetResource());
      aabb.aabb = geometryData->GetResource()->GetAABB() * GetDerivedRotation() + GetDerivedPosition();
      aabb.dirty = false;
    }

    AABB tmp = aabb.aabb;

    //aabb.Unlock();

    return tmp;
  }

  void Geometry::ApplyForceAtRelativePosition(float force, const Vector3 &direction, const Vector3 &position) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->ApplyForceAtRelativePosition(force, direction, position);
    }

  }

}
