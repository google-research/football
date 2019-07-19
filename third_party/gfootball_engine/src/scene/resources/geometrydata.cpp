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

#include "geometrydata.hpp"

#include "../../systems/isystemobject.hpp"

#include "../../base/geometry/trianglemeshutils.hpp"

namespace blunted {

  GeometryData::GeometryData() {
    aabb.aabb.Reset();
    aabb.dirty = false;
  }

  GeometryData::~GeometryData() {
    //printf("ANNIHILATING TMESH\n");
    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {
      delete [] triangleMeshes[i].vertices;
    }
  }

  GeometryData::GeometryData(const GeometryData &src) {
    for (unsigned int i = 0; i < src.triangleMeshes.size(); i++) {
      // shallow copy
      MaterializedTriangleMesh mesh = src.triangleMeshes[i];

      // 'deepen' vertices
      mesh.vertices = new float[src.triangleMeshes[i].verticesDataSize];
      memcpy(mesh.vertices, src.triangleMeshes[i].vertices, src.triangleMeshes[i].verticesDataSize * sizeof(float));

      triangleMeshes.push_back(mesh);
    }
    aabb.aabb = src.GetAABB();
    aabb.dirty = false;
  }

  void GeometryData::AddTriangleMesh(Material material, float *vertices, int verticesDataSize, std::vector<unsigned int> indices) {
    assert(indices.size() % 3 == 0);
    MaterializedTriangleMesh mesh;
    mesh.material = material;
    mesh.vertices = vertices;
    mesh.verticesDataSize = verticesDataSize;
    mesh.indices = indices;

    triangleMeshes.push_back(mesh);
    aabb.aabb.Reset();
    aabb.dirty = true;
  }

  std::vector < MaterializedTriangleMesh > GeometryData::GetTriangleMeshes() {
    return triangleMeshes;
  }

  std::vector < MaterializedTriangleMesh > &GeometryData::GetTriangleMeshesRef() {
    return triangleMeshes;
  }

  AABB GeometryData::GetAABB() const {
    if (aabb.dirty) {
      aabb.aabb.Reset();
      for (int i = 0; i < (signed int)triangleMeshes.size(); i++) {
        aabb.aabb += GetTriangleMeshAABB(triangleMeshes[i].vertices, triangleMeshes[i].verticesDataSize, triangleMeshes[i].indices);
      }
      aabb.dirty = false;
    }
    return aabb.aabb;
  }

}
