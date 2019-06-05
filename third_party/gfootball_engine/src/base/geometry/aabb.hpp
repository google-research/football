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

#ifndef _HPP_AABB
#define _HPP_AABB

#include "../../base/math/vector3.hpp"
#include "../../base/math/quaternion.hpp"

#include "line.hpp"
#include "plane.hpp"

namespace blunted {

  class AABB {

    public:
      AABB();
      AABB(const AABB &src);
      virtual ~AABB();

      AABB operator += (const AABB &add);
      AABB operator + (const Vector3 &vec) const;
      AABB operator * (const Quaternion &rot) const;

      void Reset();

      void SetMinXYZ(const Vector3 &min);
      void SetMaxXYZ(const Vector3 &max);

      real GetRadius() const;
      void GetCenter(Vector3 &center) const;
      bool Intersects(const Vector3 &center, const real radius) const;
      bool Intersects(const vector_Planes &planes) const;
      bool Intersects(const AABB &src) const;

      void MakeDirty() { radius_needupdate = true; center_needupdate = true; }

      Vector3 minxyz;
      Vector3 maxxyz;
      mutable real radius;
      mutable Vector3 center;

    protected:
      mutable bool radius_needupdate = false;
      mutable bool center_needupdate = false;

  };

  struct AABBCache {
    bool dirty = false;
    AABB aabb;
  };

}

#endif
