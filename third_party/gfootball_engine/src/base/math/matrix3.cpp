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

#include "matrix3.hpp"
#include "matrix4.hpp"
#include "vector3.hpp"

namespace blunted {

  const Matrix3 Matrix3::ZERO(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  const Matrix3 Matrix3::IDENTITY(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

  Matrix3::Matrix3() {
    for (int i = 0; i < 9; i++) {
      elements[i] = 0;
    }
  }

  Matrix3::Matrix3(real values[9]) {
    for (int i = 0; i < 9; i++) {
      elements[i] = values[i];
    }
  }

  Matrix3::Matrix3(real v1, real v2, real v3, real v4, real v5, real v6, real v7, real v8, real v9) {
    elements[0] = v1;
    elements[1] = v2;
    elements[2] = v3;
    elements[3] = v4;
    elements[4] = v5;
    elements[5] = v6;
    elements[6] = v7;
    elements[7] = v8;
    elements[8] = v9;
  }

  Matrix3::Matrix3(const Matrix3 &mat3) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        elements[x + y * 3] = mat3.elements[x + y * 3];
      }
    }
  }

  Matrix3::Matrix3(const Matrix4 &mat4) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        elements[x + y * 3] = mat4.elements[x + y * 4];
      }
    }
  }

  Matrix3::~Matrix3() {
  }


  // ----- operator overloading

  void Matrix3::operator = (const Matrix4 &mat4) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        elements[x + y * 3] = mat4.elements[x + y * 4];
      }
    }
  }

  Matrix3 Matrix3::operator * (const Matrix3 &multiplier) const {
    Matrix3 result;

    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        result.elements[r * 3 + c] =
          elements[r * 3 + 0] * multiplier.elements[0 + c] +
          elements[r * 3 + 1] * multiplier.elements[3 + c] +
          elements[r * 3 + 2] * multiplier.elements[6 + c];
      }
    }

    return result;
  }

  // http://www.facstaff.bucknell.edu/mastascu/elessonshtml/circuit/matvecmultiply.htm
  Vector3 Matrix3::operator * (const Vector3 &multiplier) const {
    Vector3 result;

    for (int r = 0; r < 3; r++) {
      result.coords[r] =
        elements[r * 3 + 0] * multiplier.coords[0] +
        elements[r * 3 + 1] * multiplier.coords[1] +
        elements[r * 3 + 2] * multiplier.coords[2];
    }

    return result;
  }

  void Matrix3::Transpose() {
    Matrix3 temp;
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        temp.elements[y + x * 3] = elements[x + y * 3];
      }
    }
    *this = Matrix3(temp);
  }

}
