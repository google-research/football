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
      virtual ~Triangle();

      virtual bool operator == (const Triangle &triangle) const;

      virtual void SetVertex(unsigned char pos, const real x, const real y, const real z);
      virtual void SetVertex(unsigned char pos, const Vector3 &vec);
      virtual void Translate(const real x, const real y, const real z);
      virtual void SetTextureVertex(unsigned char pos, const real x, const real y, const real z);
      virtual void SetTextureVertex(unsigned char texture_unit, unsigned char pos, const Vector3 &xyz);
      virtual void SetTextureVertex(unsigned char texture_unit, unsigned char pos, const real x, const real y, const real z);
      virtual void SetNormal(unsigned char pos, const real x, const real y, const real z);
      virtual void SetNormal(unsigned char pos, const Vector3 &vec);
      virtual void SetNormals(const real x, const real y, const real z);
      virtual void SetNormals(const Vector3 &vec);
      virtual const Vector3 &GetVertex(unsigned char pos) const;
      virtual const Vector3 &GetTextureVertex(unsigned char pos) const;
      virtual const Vector3 &GetTextureVertex(unsigned char texture_unit, unsigned char pos) const;
      virtual const Vector3 &GetNormal(unsigned char pos) const;
      virtual void Rewind();
      virtual AABB GetAABB();

      // ----- intersections
      virtual void IntersectsPlane(bool intersects[], const Vector3 &pvector, const Vector3 &pnormal) const;
      virtual bool IntersectsLine(const Line &u_ray) const;
      virtual bool IntersectsLine(const Line &u_ray, Vector3 &intersectVec) const;
      virtual bool IntersectsTriangle(const Triangle &triangle) const;
      virtual bool IsCoplanar(const Vector3 &N, const Triangle &triangle) const;

      // ----- utility
      virtual void CalculateTangents();
      virtual const Vector3 &GetTangent(unsigned char pos) const;
      virtual const Vector3 &GetBiTangent(unsigned char pos) const;

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
