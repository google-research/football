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

#include "aabb.hpp"

#include <climits>
#include <cmath>

namespace blunted {

  AABB::AABB() {
  }

  AABB::AABB(const AABB &src) {
    this->minxyz = src.minxyz;
    this->maxxyz = src.maxxyz;
    radius_needupdate = true;
    center_needupdate = true;
  }

  AABB::~AABB() {
  }


  AABB AABB::operator += (const AABB &add) {
    if (minxyz.coords[0] > add.minxyz.coords[0]) minxyz.coords[0] = add.minxyz.coords[0];
    if (minxyz.coords[1] > add.minxyz.coords[1]) minxyz.coords[1] = add.minxyz.coords[1];
    if (minxyz.coords[2] > add.minxyz.coords[2]) minxyz.coords[2] = add.minxyz.coords[2];
    if (maxxyz.coords[0] < add.maxxyz.coords[0]) maxxyz.coords[0] = add.maxxyz.coords[0];
    if (maxxyz.coords[1] < add.maxxyz.coords[1]) maxxyz.coords[1] = add.maxxyz.coords[1];
    if (maxxyz.coords[2] < add.maxxyz.coords[2]) maxxyz.coords[2] = add.maxxyz.coords[2];
    radius_needupdate = true;
    center_needupdate = true;
    return *this;
  }

  AABB AABB::operator + (const Vector3 &vec) const {
    AABB aabb(*this);
    aabb.minxyz += vec;
    aabb.maxxyz += vec;
    radius_needupdate = true;
    center_needupdate = true;
    return aabb;
  }

  AABB AABB::operator * (const Quaternion &rot) const {
    AABB aabb(*this);
    std::vector<Vector3> vecs;
    Vector3 vec;
    vec.Set(aabb.minxyz.coords[0], aabb.minxyz.coords[1], aabb.minxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.maxxyz.coords[0], aabb.minxyz.coords[1], aabb.minxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.minxyz.coords[0], aabb.maxxyz.coords[1], aabb.minxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.maxxyz.coords[0], aabb.maxxyz.coords[1], aabb.minxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.minxyz.coords[0], aabb.minxyz.coords[1], aabb.maxxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.maxxyz.coords[0], aabb.minxyz.coords[1], aabb.maxxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.minxyz.coords[0], aabb.maxxyz.coords[1], aabb.maxxyz.coords[2]);
    vecs.push_back(vec);
    vec.Set(aabb.maxxyz.coords[0], aabb.maxxyz.coords[1], aabb.maxxyz.coords[2]);
    vecs.push_back(vec);

    for (int i = 0; i < (signed int)vecs.size(); i++) {
      vecs[i] = rot * vecs[i];
    }

    aabb.Reset();

    for (int i = 0; i < (signed int)vecs.size(); i++) {
      if (vecs[i].coords[0] < aabb.minxyz.coords[0]) aabb.minxyz.coords[0] = vecs[i].coords[0];
      if (vecs[i].coords[0] > aabb.maxxyz.coords[0]) aabb.maxxyz.coords[0] = vecs[i].coords[0];
      if (vecs[i].coords[1] < aabb.minxyz.coords[1]) aabb.minxyz.coords[1] = vecs[i].coords[1];
      if (vecs[i].coords[1] > aabb.maxxyz.coords[1]) aabb.maxxyz.coords[1] = vecs[i].coords[1];
      if (vecs[i].coords[2] < aabb.minxyz.coords[2]) aabb.minxyz.coords[2] = vecs[i].coords[2];
      if (vecs[i].coords[2] > aabb.maxxyz.coords[2]) aabb.maxxyz.coords[2] = vecs[i].coords[2];
    }

    radius_needupdate = true;
    center_needupdate = true;
    return aabb;
  }

  void AABB::Reset() {
    const float min_v = std::numeric_limits<real>::min() / 100;
    const float max_v = std::numeric_limits<real>::max() / 100;
    minxyz.Set(max_v, max_v, max_v);
    maxxyz.Set(min_v, min_v, min_v);
    radius_needupdate = true;
    center_needupdate = true;
  }

  void AABB::SetMinXYZ(const Vector3 &min) {
    minxyz = min;
    MakeDirty();
  }

  void AABB::SetMaxXYZ(const Vector3 &max) {
    maxxyz = max;
    MakeDirty();
  }

  real AABB::GetRadius() const {
    if (radius_needupdate) {
      real x, y, z;
      x = maxxyz.coords[0] - minxyz.coords[0];
      y = maxxyz.coords[1] - minxyz.coords[1];
      z = maxxyz.coords[2] - minxyz.coords[2];
      real length = sqrt(std::pow(x, 2) + std::pow(y, 2));
      radius = sqrt(std::pow(length, 2) + std::pow(z, 2)) / 2.0;
      radius_needupdate = false;
    }
    return radius;
  }

  void AABB::GetCenter(Vector3 &center) const {
    if (center_needupdate) {
      real x = (minxyz.coords[0] + maxxyz.coords[0]) / 2.0;
      real y = (minxyz.coords[1] + maxxyz.coords[1]) / 2.0;
      real z = (minxyz.coords[2] + maxxyz.coords[2]) / 2.0;
      this->center.Set(x, y, z);
      center_needupdate = false;
    }
    center.Set(this->center);
  }

  bool AABB::Intersects(const Vector3 &center, const real radius) const {
    real s, d = 0;

    for (int i = 0; i < 3; i++) {
      if (center.coords[i] < minxyz.coords[i]) {
        s = center.coords[i] - minxyz.coords[i];
        d += s * s;
      } else if (center.coords[i] > maxxyz.coords[i]) {
        s = center.coords[i] - maxxyz.coords[i];
        d += s * s;
      }
    }
    return d <= radius * radius;
  }

  bool AABB::Intersects(const vector_Planes &planes) const {
    bool aabb_intersects_planecollection = true;

    // all points in the aabb
    Vector3 aabb_points[8];
    aabb_points[0].Set(minxyz);
    aabb_points[1].Set(minxyz.coords[0], maxxyz.coords[1], minxyz.coords[2]);
    aabb_points[2].Set(minxyz.coords[0], maxxyz.coords[1], maxxyz.coords[2]);
    aabb_points[3].Set(minxyz.coords[0], minxyz.coords[1], maxxyz.coords[2]);
    aabb_points[4].Set(maxxyz.coords[0], minxyz.coords[1], minxyz.coords[2]);
    aabb_points[5].Set(maxxyz.coords[0], maxxyz.coords[1], minxyz.coords[2]);
    aabb_points[6].Set(maxxyz);
    aabb_points[7].Set(maxxyz.coords[0], minxyz.coords[1], maxxyz.coords[2]);

    real determinant;
    real distance;
    int planes_size = planes.size();
    for (int p = 0; p < planes_size; p++) {
      // check if there is any point (that defines this aabb) on the inside of this plane
      aabb_intersects_planecollection = false;

      determinant = planes.at(p).GetDeterminant();
      for (int ap = 0; ap < 8; ap++) {
        distance = determinant + planes.at(p).GetVertex(1).coords[0] * aabb_points[ap].coords[0] +
                                 planes.at(p).GetVertex(1).coords[1] * aabb_points[ap].coords[1] +
                                 planes.at(p).GetVertex(1).coords[2] * aabb_points[ap].coords[2];

        if (distance > 0) {
          aabb_intersects_planecollection = true;
          break;
        }
      }
      if (aabb_intersects_planecollection == false) break;
    }

    return aabb_intersects_planecollection;
  }

  bool AABB::Intersects(const AABB &src) const {
    if ((src.minxyz.coords[0] < maxxyz.coords[0] &&
         src.minxyz.coords[1] < maxxyz.coords[1] &&
         src.minxyz.coords[2] < maxxyz.coords[2]) &&
        (src.maxxyz.coords[0] > minxyz.coords[0] &&
         src.maxxyz.coords[1] > minxyz.coords[1] &&
         src.maxxyz.coords[2] > minxyz.coords[2])) {
      return true;
    } else {
      return false;
    }
  }
}
