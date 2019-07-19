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

#ifndef _HPP_LINE
#define _HPP_LINE

#include "../../base/math/vector3.hpp"

namespace blunted {

  class AABB;

  class Line {

    public:
      Line();
      Line(const Vector3 vec1, const Vector3 vec2);
      ~Line();

      void SetVertex(unsigned char pos, const Vector3 &vec);
      const Vector3 &GetVertex(unsigned char pos) const;

      float GetClosestToPoint(const Vector3 &point) const;
      float GetDistanceToPoint(const Vector3 &point, float &u) const;

      Vector3 GetIntersectionPoint(const Line &line) const;
      Vector3 GetIntersectionPoint(const Line &line, float &u) const;

      bool WhatSide(const Vector3 &point);
      float GetLength() const { return (vertices[0] - vertices[1]).GetLength(); }

    protected:
      Vector3 vertices[2];

    private:

  };

}

#endif
