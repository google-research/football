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

#ifndef _hpp_bluntmath_quaternion
#define _hpp_bluntmath_quaternion

#include <cmath>

#include "../../defines.hpp"

#include "bluntmath.hpp"

namespace blunted {

  class Vector3;
  class Matrix3;

  static real QUATERNION_IDENTITY[4] = { 0, 0, 0, 1 };

  class Quaternion {

    public:
      inline Quaternion() {
        elements[0] = 0;
        elements[1] = 0;
        elements[2] = 0;
        elements[3] = 1;
      }
      Quaternion(real x, real y, real z, real w);
      Quaternion(real values[4]);
      virtual ~Quaternion();

      void Set(real x, real y, real z, real w);
      void Set(const Quaternion &quat);
      void Set(const Matrix3 &mat);

      // ----- operator overloading
      bool operator != (const Quaternion &fac) const;
      const Quaternion operator * (float scale) const;
      Vector3 operator * (const Vector3 &fac) const;
      void operator = (const Vector3 &vec);

      Quaternion operator * (const Quaternion &fac) const;
      Quaternion operator + (const Quaternion &q2) const;
      Quaternion operator - (const Quaternion &q2) const;
      Quaternion operator - () const;

      // ----- mathematics
      Quaternion GetInverse() const;
      void ConstructMatrix(Matrix3 &rotation) const;
      void GetAngles(radian &X, radian &Y, radian &Z) const;
      void SetAngles(radian X, radian Y, radian Z);
      void GetAngleAxis(radian &rfangle, Vector3 &rkaxis) const;
      void SetAngleAxis(const radian &rfangle, const Vector3 &rkaxis);
      void conjugate();
      Quaternion conjugate_get() const;
      void scale(const real fac);
      real GetMagnitude() const;
      void Normalize();
      Quaternion GetNormalized() const;
      float GetDotProduct(const Quaternion &subject) const;
      Quaternion GetLerped(float bias, const Quaternion &to) const; // bias > 1 can extrapolate small angles in a hacky, incorrect way *edit: is that so? i'm not so sure
      Quaternion GetSlerped(float bias, const Quaternion &to) const;
      Quaternion GetRotationTo(const Quaternion &to) const;
      Quaternion GetRotationMultipliedBy(float factor) const;
      radian GetRotationAngle(const Quaternion &to) const {
        return 2.0f * std::acos(clamp(this->GetDotProduct(to), -1.0f, 1.0f));
      }
      float MakeSameNeighborhood(const Quaternion &src); // returns dot product as added bonus! ;)

      real elements[4];

    protected:

    private:

  };

}

#endif
