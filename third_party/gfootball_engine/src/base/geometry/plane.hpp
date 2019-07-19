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

#ifndef _hpp_plane
#define _hpp_plane

#include "../math/vector3.hpp"

#include <vector>

namespace blunted {

  class Plane;
  typedef std::vector<Plane> vector_Planes;

  class Plane {
    // vector 0 = a position on the plane
    // vector 1 = plane normal

    public:
      Plane();
      Plane(const Vector3 vec1, const Vector3 vec2);
      ~Plane();

      void Set(const Vector3 &pos, const Vector3 &dir);
      void SetVertex(unsigned char pos, const Vector3 &vec);
      const Vector3 &GetVertex(unsigned char pos) const;

      void CalculateDeterminant() const;
      real GetDeterminant() const;

    protected:
      Vector3 vertices[2];
      mutable real determinant;
      mutable bool _dirty_determinant = false;

    private:

  };

}

#endif
