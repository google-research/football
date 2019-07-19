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

#include "splitgeometry.hpp"

#include <cmath>

#include "../base/utils.hpp"
#include "../scene/objectfactory.hpp"
#include "../managers/resourcemanagerpool.hpp"
#include "../base/geometry/trianglemeshutils.hpp"

namespace blunted {

  struct GeomIndex {
    float x = 0.0f;
    float y = 0.0f;
    boost::intrusive_ptr < Resource<GeometryData> > geomData;
  };

  boost::intrusive_ptr < Resource<GeometryData> > GetGridGeom(const std::string &name, boost::shared_ptr<Scene3D> scene3D, std::vector<GeomIndex> &geomVec, const AABB &aabb, float gridSize) {

    // calculate grid position

    Vector3 center;
    aabb.GetCenter(center);
    //center.Print();

    float x, y;

    float index = center.coords[0] / gridSize;
    int intIndex = int(std::round(index));
    x = gridSize * intIndex;

    index = center.coords[1] / gridSize;
    intIndex = int(std::round(index));
    y = gridSize * intIndex;


    // check if geom exists

    for (int i = 0; i < (signed int)geomVec.size(); i++) {
      if (geomVec[i].x == x && geomVec[i].y == y) {
        //printf("%f %f %f %f\n", geomVec[i].x, x, geomVec[i].y, y);
        return geomVec[i].geomData; // found! return geomdata
      }
    }


    // not found! create geomdata

    GeomIndex newIndex;
    newIndex.x = x;
    newIndex.y = y;
    newIndex.geomData = ResourceManagerPool::getGeometryManager()->Fetch(name + " gridGeomData @ " + int_to_str(x) + ", " + int_to_str(y), false, false);
    geomVec.push_back(newIndex);

    return newIndex.geomData;
  }

  boost::intrusive_ptr<Node> SplitGeometry(boost::shared_ptr<Scene3D> scene3D, boost::intrusive_ptr<Geometry> source, float gridSize) {

    boost::intrusive_ptr<Node> resultNode(new Node(source->GetName()));

    std::vector<GeomIndex> geomVec;

    // iterate trianglemeshes
    boost::intrusive_ptr < Resource<GeometryData> > geomData = source->GetGeometryData();

    std::vector < MaterializedTriangleMesh > tmeshes = geomData->GetResource()->GetTriangleMeshes();

    //printf("%i subgeoms found\n", tmeshes.size());
    for (int i = 0; i < (signed int)tmeshes.size(); i++) {

      std::vector<unsigned int> indices; // empty == don't use indices
      AABB aabb = GetTriangleMeshAABB(tmeshes[i].vertices, tmeshes[i].verticesDataSize, indices);
      boost::intrusive_ptr < Resource<GeometryData> > subGeomData = GetGridGeom(source->GetName(), scene3D, geomVec, aabb, gridSize);

      float *newTMesh = new float[tmeshes[i].verticesDataSize];
      memcpy(newTMesh, tmeshes[i].vertices, tmeshes[i].verticesDataSize * sizeof(float));
      subGeomData->GetResource()->AddTriangleMesh(tmeshes[i].material, newTMesh, tmeshes[i].verticesDataSize, indices);
    }

    for (int i = 0; i < (signed int)geomVec.size(); i++) {
      float x = geomVec[i].x;
      float y = geomVec[i].y;
      boost::intrusive_ptr<Geometry> geom = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject(source->GetName() + " gridGeom @ " + int_to_str(x) + ", " + int_to_str(y), e_ObjectType_Geometry));
      scene3D->CreateSystemObjects(geom);
      geom->SetGeometryData(geomVec[i].geomData);
      resultNode->AddObject(geom);
    }

    return resultNode;
  }

}
