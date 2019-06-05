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

#ifndef _hpp_bluntmath_matrix3
#define _hpp_bluntmath_matrix3

#include "../../defines.hpp"

#include "bluntmath.hpp"

namespace blunted {

  class Vector3;
  class Matrix4;

  // column major, row minor matrix
  class Matrix3 {

    public:
      Matrix3();
      Matrix3(real values[9]);
      Matrix3(real v1, real v2, real v3, real v4, real v5, real v6, real v7, real v8, real v9);
      Matrix3(const Matrix3 &mat3);
      Matrix3(const Matrix4 &mat4);
      virtual ~Matrix3();

      // ----- operator overloading
      void operator = (const Matrix4 &mat4);
      Matrix3 operator * (const Matrix3 &multiplier) const;
      Vector3 operator * (const Vector3 &multiplier) const;

      // ----- mathematics
      void Transpose();

      static const Matrix3 ZERO;
      static const Matrix3 IDENTITY;
      real elements[9];

    protected:

    private:

  };

}

#endif
