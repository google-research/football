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

#ifndef _HPP_OBJECT_GEOMETRY
#define _HPP_OBJECT_GEOMETRY

#include "../../defines.hpp"
#include "../../base/math/vector3.hpp"
#include "../../scene/object.hpp"
#include "../../scene/resources/geometrydata.hpp"
#include "../../types/interpreter.hpp"
#include "../../types/resource.hpp"

namespace blunted {

  class Geometry : public Object {

    public:
      Geometry(std::string name, e_ObjectType objectType = e_ObjectType_Geometry);
      Geometry(const Geometry &src, const std::string &postfix);
      virtual ~Geometry();

      virtual void Exit();

      void SetGeometryData(boost::intrusive_ptr < Resource<GeometryData> > geometryData);
      boost::intrusive_ptr < Resource<GeometryData> > GetGeometryData();

      void OnUpdateGeometryData(bool updateMaterials = true);

      virtual void Poke(e_SystemType targetSystemType);

      void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

      virtual AABB GetAABB() const;

    protected:
      boost::intrusive_ptr < Resource<GeometryData> > geometryData;

  };

  class IGeometryInterpreter : public Interpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr<Geometry> geom) = 0;
      virtual void OnUpdateGeometry(boost::intrusive_ptr<Geometry> geometry, bool updateMaterials) = 0;
      virtual void OnUnload() = 0;
      virtual void OnMove(const Vector3 &position) = 0;
      virtual void OnRotate(const Quaternion &rotation) = 0;

      virtual void OnPoke() = 0;
  };

}

#endif
