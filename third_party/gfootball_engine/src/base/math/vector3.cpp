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

#include "vector3.hpp"

#include "quaternion.hpp"

#include <cmath>

#include "../../base/log.hpp"

namespace blunted {

  Vector3::Vector3() {
    coords[0] = 0;
    coords[1] = 0;
    coords[2] = 0;
  }

  Vector3::Vector3(real xyz) {
    coords[0] = xyz;
    coords[1] = xyz;
    coords[2] = xyz;
  }

  Vector3::Vector3(real x, real y, real z) {
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
  }

  void Vector3::Set(real xyz) {
    coords[0] = xyz;
    coords[1] = xyz;
    coords[2] = xyz;
  }

  void Vector3::Set(real x, real y, real z) {
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
  }

  void Vector3::Set(const Vector3 &vec) {
    coords[0] = vec.coords[0];
    coords[1] = vec.coords[1];
    coords[2] = vec.coords[2];
  }

  float Vector3::GetEnvCoord(int index) const {
    switch (index) {
      case 0:
        return coords[0];
      case 1:
        return coords[1];
      case 2:
        return coords[2];
      default:
        Log(e_FatalError, "Vector", "GetEnvCoord", "Invalid coordinate");
    }
    return 0;
  }

  void Vector3::SetEnvCoord(int index, float value) {
    switch (index) {
      case 0:
        coords[0] = value;
        break;
      case 1:
        coords[1] = value;
        break;
      case 2:
        coords[2] = value;
        break;
      default:
        Log(e_FatalError, "Vector", "GetEnvCoord", "Invalid coordinate");
    }
  }

  // ----- operator overloading

  void Vector3::operator = (const Quaternion &quat) {
    // http://www.devmaster.net/forums/showthread.php?t=14097
    // thanks @ reedbeta

    Quaternion blah;
    blah.Set(0, 0, -1, 0);

    Quaternion result = quat * blah * quat.GetInverse();
    Set(result.elements[0], result.elements[1], result.elements[2]);
  }

  Vector3 &Vector3::operator *= (const Matrix3 &mat) {
    Vector3 tmp;
    tmp.coords[0] = coords[0] * mat.elements[0] + coords[1] * mat.elements[3] + coords[2] * mat.elements[6];
    tmp.coords[1] = coords[0] * mat.elements[1] + coords[1] * mat.elements[4] + coords[2] * mat.elements[7];
    tmp.coords[2] = coords[0] * mat.elements[2] + coords[1] * mat.elements[5] + coords[2] * mat.elements[8];
    coords[0] = tmp.coords[0];
    coords[1] = tmp.coords[1];
    coords[2] = tmp.coords[2];

    return *this;
  }

  // not sure if legal
  Vector3 &Vector3::operator *= (const Matrix4 &mat) {

    const Matrix3 bla = Matrix3(mat);
    Vector3 tmp = *this;
    tmp *= bla;
    *this = tmp;

    coords[0] += mat.elements[3];
    coords[1] += mat.elements[7];
    coords[2] += mat.elements[11];

    return *this;
  }


  // ----- mathematics!!! don't we just love it

  Vector3 Vector3::GetCrossProduct(const Vector3 &fac) const {
    return Vector3(coords[1] * fac.coords[2] - coords[2] * fac.coords[1],
                   coords[2] * fac.coords[0] - coords[0] * fac.coords[2],
                   coords[0] * fac.coords[1] - coords[1] * fac.coords[0]);
  }

  real Vector3::GetDotProduct(const Vector3 &fac) const {
    return (coords[0] * fac.coords[0] + coords[1] * fac.coords[1] + coords[2] * fac.coords[2]);
  }

  void Vector3::ConstructMatrix(Matrix3 &mat) const {
    mat.elements[0] = coords[0];
    mat.elements[4] = coords[1];
    mat.elements[8] = coords[2];
  }

  void Vector3::FastNormalize() {

    // http://www.devmaster.net/forums/showthread.php?t=4460

    float x = GetDotProduct(*this);
    float xhalf = 0.5f * x;
    int i = *(int*)&x;         // get bits for floating value
    i = 0x5f3759df - (i>>1);   // give initial guess y0
    x = *(float*)&i;           // convert bits back to float
    x *= 1.5f - xhalf*x*x;     // newton step, repeating this step
                               // increases accuracy

    coords[0] *= x;
    coords[1] *= x;
    coords[2] *= x;
  }

  void Vector3::Normalize(const Vector3 &ifNull) {
    if (fabs(this->coords[0]) < 0.000001f && fabs(this->coords[1]) < 0.000001f && fabs(this->coords[2]) < 0.000001f) {
      this->coords[0] = ifNull.coords[0];
      this->coords[1] = ifNull.coords[1];
      this->coords[2] = ifNull.coords[2];
    } else {
      real f = 1.0f / std::sqrt(GetDotProduct(*this));
      this->coords[0] *= f;
      this->coords[1] *= f;
      this->coords[2] *= f;
    }
  }

  void Vector3::Normalize() {
    real f = 1.0f / std::sqrt(GetDotProduct(*this));
    coords[0] *= f;
    coords[1] *= f;
    coords[2] *= f;
  }

  void Vector3::NormalizeTo(float length) {
    if (fabs(this->coords[0]) < 0.000001f && fabs(this->coords[1]) < 0.000001f && fabs(this->coords[2]) < 0.000001f) Log(e_Warning, "Vector3", "NormalizeTo", "Trying to normalize 0-vector");
    real f = length / std::sqrt(GetDotProduct(*this));

    coords[0] *= f;
    coords[1] *= f;
    coords[2] *= f;
  }

  void Vector3::NormalizeMax(float length) {
    if (GetLength() > length) {
      Normalize();
      *this *= length;
    }
  }

  Vector3 Vector3::GetNormalized(const Vector3 &ifNull) const {
    Vector3 tmp(*this);
    tmp.Normalize(ifNull);
    return tmp;
  }

  Vector3 Vector3::GetNormalized() const {
    Vector3 tmp(*this);
    tmp.Normalize();
    return tmp;
  }

  Vector3 Vector3::GetNormalizedTo(float length) const {
    if (fabs(this->coords[0]) < 0.000001f && fabs(this->coords[1]) < 0.000001f && fabs(this->coords[2]) < 0.000001f) Log(e_Warning, "Vector3", "GetNormalizedTo", "Trying to normalize 0-vector");
    Vector3 tmp(*this);
    tmp.NormalizeTo(length);
    return tmp;
  }

  Vector3 Vector3::GetNormalizedMax(float length) const {
    if (GetLength() > length) return GetNormalized(0) * length; else return Vector3(*this);
  }

  real Vector3::GetDistance(const Vector3 &fac) const {
    real v3[3];
    v3[0] = coords[0] - fac.coords[0];
    v3[1] = coords[1] - fac.coords[1];
    v3[2] = coords[2] - fac.coords[2];

    // premature optimization ;)
    if (v3[0] == 0.0f && v3[1] == 0.0f && v3[2] == 0.0f) return 0.0f;

    float length =
        sqrt(std::pow(v3[0], 2) + std::pow(v3[1], 2) + std::pow(v3[2], 2));

    if (length < 0.000001) length = 0;
    return length;
  }

  bool Vector3::Compare(const Vector3 &test) const {
    if (test.coords[0] == coords[0] &&
        test.coords[1] == coords[1] &&
        test.coords[2] == coords[2]) {
      return true;
    } else {
      return false;
    }
  }

  Vector3 Vector3::GetAbsolute() const {
    return Vector3(fabsf(coords[0]), fabsf(coords[1]), fabsf(coords[2]));
  }

  Vector3 Vector3::EnforceMaximumDeviation(const Vector3 &deviant, float maxDeviation) const {
    Vector3 result = *this;
    Vector3 difference = deviant - *this;
    float differenceDistance = difference.GetLength();
    if (differenceDistance > maxDeviation) {
      result += difference.GetNormalized() * (differenceDistance - maxDeviation);
    }
    if (maxDeviation == 0.0f) assert(deviant.GetDistance(result) < 0.001f);
    return result;
  }

  Vector3 Vector3::GetClamped2D(const Vector3 &v1, const Vector3 &v2) const {

    Vector3 result = *this;

    radian v1_to_v2 = v2.GetAngle2D(v1);
    signed int direction = signSide(v1_to_v2);
    radian v1_to_this = (*this).GetAngle2D(v1);
    radian v2_to_this = (*this).GetAngle2D(v2);
    if (signSide(v1_to_this) != direction || signSide(v2_to_this) == direction) { // wrong side! (what this does: make 2 cross sections through the virtual circle, one for each parameter, and check if we are on the wrong side of either one
      // check to which parameter-vec we are the closest and clamp to that one
      if (fabs(v1_to_this) < fabs(v2_to_this)) {
        result = v1;
      } else {
        result = v2;
      }
    }

    return result;
  }

  void Vector3::Extrapolate(const Vector3 &direction, unsigned long time) {
    *this += direction * (time / 1000.0);
  }

  std::ostream& operator<<(std::ostream& os, const Vector3& v)
  {
    os << v.coords[0] << " " << v.coords[1] << " " << v.coords[2];
    return os;
  }

}
