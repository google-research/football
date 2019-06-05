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

#include "trianglemeshutils.hpp"

#include "aabb.hpp"

namespace blunted {

  const int __triangleMeshElementCount = 5;

  AABB GetTriangleMeshAABB(float *vertices, int verticesDataSize, const std::vector<unsigned int> &indices) {
    AABB aabb;

    aabb.Reset();

    if (indices.size() == 0) {

      for (unsigned int t = 0; t < verticesDataSize / (unsigned int)GetTriangleMeshElementCount() / 3 / 3; t++) {
        for (unsigned int v = 0; v < 3; v++) {
          for (unsigned int i = 0; i < 3; i++) {
            if (vertices[t * 9 + v * 3 + i] < aabb.minxyz.coords[i]) aabb.minxyz.coords[i] = vertices[t * 9 + v * 3 + i];
            if (vertices[t * 9 + v * 3 + i] > aabb.maxxyz.coords[i]) aabb.maxxyz.coords[i] = vertices[t * 9 + v * 3 + i];
          }
        }
      }

    } else {

      for (unsigned int t = 0; t < indices.size() / 3; t++) {
        for (unsigned int v = 0; v < 3; v++) {
          for (unsigned int i = 0; i < 3; i++) {
            assert(verticesDataSize > indices[t * 3 + v] + i);
            if (vertices[indices[t * 3 + v] + i] < aabb.minxyz.coords[i]) aabb.minxyz.coords[i] = vertices[indices[t * 3 + v] + i];
            if (vertices[indices[t * 3 + v] + i] > aabb.maxxyz.coords[i]) aabb.maxxyz.coords[i] = vertices[indices[t * 3 + v] + i];
          }
        }
      }

    }

    aabb.MakeDirty();
    return aabb;
  }

  int GetTriangleMeshElementCount() {
    return __triangleMeshElementCount;
  }

}
