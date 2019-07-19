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

#ifndef _HPP_BASE_GEOMETRY_TRIANGLE
#define _HPP_BASE_GEOMETRY_TRIANGLE

#include "../../defines.hpp"

#include "../../base/math/vector3.hpp"
#include "aabb.hpp"

namespace blunted {

  class Triangle {

    public:
      Triangle();
      Triangle(const Triangle &triangle);
      Triangle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3);
      ~Triangle();
      bool operator == (const Triangle &triangle) const;

      inline void SetVertex(unsigned char pos, const Vector &vec) {
        vertices[pos].coords[0] = vec.coords[0];
        vertices[pos].coords[1] = vec.coords[1];
        vertices[pos].coords[2] = vec.coords[2];
      }

      inline void SetTextureVertex(unsigned char texture_unit, unsigned char pos, const real x, const real y, const real z) {
        textureVertices[pos][texture_unit].coords[0] = x;
        textureVertices[pos][texture_unit].coords[1] = y;
        textureVertices[pos][texture_unit].coords[2] = z;
      }

      inline void SetNormal(unsigned char pos, const real x, const real y, const real z) {
        normals[pos].coords[0] = x;
        normals[pos].coords[1] = y;
        normals[pos].coords[2] = z;
      }

      inline void SetNormals(const Vector &vec) {
        for (int i = 0; i < 3; i++) {
          normals[i].coords[0] = vec.coords[0];
          normals[i].coords[1] = vec.coords[1];
          normals[i].coords[2] = vec.coords[2];
        }
      }

      inline const Vector3 &GetVertex(unsigned char pos) const {
        return vertices[pos];
      }

      inline const Vector3 &GetTextureVertex(unsigned char pos) const {
        return GetTextureVertex(0, pos);
      }

      inline const Vector3 &GetTextureVertex(unsigned char texture_unit, unsigned char pos) const {
        return textureVertices[pos][texture_unit];
      }

      inline const Vector3 &GetNormal(unsigned char pos) const {
        return normals[pos];
      }

      // ----- intersections
      bool IntersectsLine(const Line &u_ray, Vector3 &intersectVec) const;

      // ----- utility
      void CalculateTangents();

      inline const Vector3 &GetTangent(unsigned char pos) const {
        return tangents[pos];
      }

      inline const Vector3 &GetBiTangent(unsigned char pos) const {
        return biTangents[pos];
      }

    protected:
      Vector3 vertices[3];
      Vector3 tangents[3];
      Vector3 biTangents[3];
      Vector3 textureVertices[3][8];
      Vector3 normals[3];

    private:

  };

}

#endif
