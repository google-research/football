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

#ifndef _hpp_bluntmath_vector3
#define _hpp_bluntmath_vector3

#include <cmath>

#include "bluntmath.hpp"
#include "matrix3.hpp"
#include "matrix4.hpp"
#include "quaternion.hpp"

namespace blunted {

  class Vector3;

  typedef Vector3 Vector;

  class Vector3 {

    public:
      Vector3();
      Vector3(const Vector3 &src) { coords[0] = src.coords[0]; coords[1] = src.coords[1]; coords[2] = src.coords[2]; } // gcc bug? this sometimes only sets the last element: memcpy(coords, src.coords, 3 * sizeof(real)); }
      Vector3(real xyz);
      Vector3(real x, real y, real z);
      inline virtual ~Vector3() {};

      void Set(real xyz);
      void Set(real x, real y, real z);
      void Set(const Vector3 &vec);
      float GetEnvCoord(int index) const;
      void SetEnvCoord(int index, float value);

      // ----- operator overloading
      bool operator == (const Vector3 &vector) const;
      bool operator != (const Vector3 &vector) const;
      void operator = (const Quaternion &quat);
      void operator = (const real src);
      void operator = (const Vector3 &src);
      Vector3 operator * (const real scalar) const;
      Vector3 operator * (const Vector3 &scalar) const;
      Vector3 &operator *= (const real scalar);
      Vector3 &operator *= (const Vector3 &scalar);
      Vector3 &operator *= (const Matrix3 &mat);
      Vector3 &operator *= (const Matrix4 &mat);
      Vector3 operator / (const real scalar) const;
      Vector3 operator / (const Vector3 &scalar) const;
      Vector3 &operator /= (const Vector3 &scalar);
      Vector3 &operator += (const real scalar);
      Vector3 &operator += (const Vector3 &scalar);
      Vector3 &operator -= (const Vector3 &scalar);
      Vector3 operator + (const Vector3 &vec) const;
      const Vector3 &operator + () const;
      Vector3 operator + (const real value) const;
      Vector3 operator - (const Vector3 &vec) const;
      Vector3 operator - () const;
      Vector3 operator - (const real value) const;

      bool operator < (const Vector3 &vector) const;

      // ----- mathematics
      Vector3 GetCrossProduct(const Vector3 &fac) const;
      real GetDotProduct(const Vector3 &fac) const;
      void ConstructMatrix(Matrix3 &mat) const;
      void FastNormalize();
      void Normalize(const Vector3 &ifNull);
      void Normalize();
      void NormalizeTo(float length);
      void NormalizeMax(float length);
      // this function does not make much sense does it? void NormalizeMax(float length, const Vector3 &ifNull);
      Vector3 GetNormalized(const Vector3 &ifNull) const;
      Vector3 GetNormalized() const;
      Vector3 GetNormalizedTo(float length) const;
      Vector3 GetNormalizedMax(float length) const;
      // this function does not make much sense does it? Vector3 GetNormalizedMax(float length, const Vector3 &ifNull) const;
      real GetDistance(const Vector3 &fac) const;
      real GetLength() const;
      real GetSquaredLength() const;
      radian GetAngle2D() const;
      // both need to be normalized!
      radian GetAngle2D(const Vector3 &test) const;
      void Rotate(const Quaternion &quat);
      void Rotate2D(const radian angle);
      Vector3 GetRotated2D(const radian angle) const;
      Vector3 Get2D() const;
      bool Compare(const Vector3 &test) const;
      Vector3 GetAbsolute() const;
      Vector3 EnforceMaximumDeviation(const Vector3 &deviant, float maxDeviation) const;
      Vector3 GetClamped2D(const Vector3 &v1, const Vector3 &v2) const;

      // extrapolates a vector by a direction & a time difference in the future (positive) or in the past (negative)
      // assume direction vector to be units per sec, assume time to be in milliseconds
      void Extrapolate(const Vector3 &direction, unsigned long time);

      real coords[3];

    protected:

    private:

  };

  std::ostream& operator<<(std::ostream& os, const Vector3& v);

  // INLINED FUNCTIONS


  // ----- operator overloading

  inline
  bool Vector3::operator == (const Vector3 &vector) const {
    if (coords[0] == vector.coords[0] &&
        coords[1] == vector.coords[1] &&
        coords[2] == vector.coords[2]) {
      return true;
    }
    return false;
  }

  inline
  bool Vector3::operator != (const Vector3 &vector) const {
    if (*this == vector) return false; else return true;
  }

  inline
  void Vector3::operator = (const real src) {
    Set(src);
  }

  inline
  void Vector3::operator = (const Vector3 &src) {
    coords[0] = src.coords[0];
    coords[1] = src.coords[1];
    coords[2] = src.coords[2];
  }

  inline
  Vector3 Vector3::operator * (const real scalar) const {
    return Vector3(coords[0] * scalar, coords[1] * scalar, coords[2] * scalar);
  }

  inline
  Vector3 Vector3::operator * (const Vector3 &scalar) const {
    return Vector3(coords[0] * scalar.coords[0], coords[1] * scalar.coords[1], coords[2] * scalar.coords[2]);
  }

  inline
  Vector3 &Vector3::operator *= (const real scalar) {
    coords[0] *= scalar;
    coords[1] *= scalar;
    coords[2] *= scalar;
    return *this;
  }

  inline
  Vector3 &Vector3::operator *= (const Vector3 &scalar) {
    coords[0] *= scalar.coords[0];
    coords[1] *= scalar.coords[1];
    coords[2] *= scalar.coords[2];
    return *this;
  }

  inline
  Vector3 Vector3::operator / (const real scalar) const {
    return Vector3(coords[0] / scalar, coords[1] / scalar, coords[2] / scalar);
  }

  inline
  Vector3 Vector3::operator / (const Vector3 &scalar) const {
    return Vector3(coords[0] / scalar.coords[0], coords[1] / scalar.coords[1], coords[2] / scalar.coords[2]);
  }

  inline
  Vector3 &Vector3::operator /= (const Vector3 &scalar) {
    coords[0] /= scalar.coords[0];
    coords[1] /= scalar.coords[1];
    coords[2] /= scalar.coords[2];
    return *this;
  }

  inline
  Vector3 &Vector3::operator += (const real scalar) {
    coords[0] += scalar;
    coords[1] += scalar;
    coords[2] += scalar;
    return *this;
  }

  inline
  Vector3 &Vector3::operator += (const Vector3 &scalar) {
    coords[0] += scalar.coords[0];
    coords[1] += scalar.coords[1];
    coords[2] += scalar.coords[2];
    return *this;
  }

  inline
  Vector3 &Vector3::operator -= (const Vector3 &scalar) {
    coords[0] -= scalar.coords[0];
    coords[1] -= scalar.coords[1];
    coords[2] -= scalar.coords[2];
    return *this;
  }

  inline
  Vector3 Vector3::operator + (const Vector3 &vec) const {
    return Vector3(coords[0] + vec.coords[0], coords[1] + vec.coords[1], coords[2] + vec.coords[2]);
  }

  inline
  const Vector3 &Vector3::operator + () const {
    return *this;
  }

  inline
  Vector3 Vector3::operator + (const real value) const {
    return Vector3(coords[0] + value, coords[1] + value, coords[2] + value);
  }

  inline
  Vector3 Vector3::operator - (const Vector3 &vec) const {
    return Vector3(coords[0] - vec.coords[0], coords[1] - vec.coords[1], coords[2] - vec.coords[2]);
  }

  inline
  Vector3 Vector3::operator - () const {
    return Vector3(-coords[0], -coords[1], -coords[2]);
  }

  inline
  Vector3 Vector3::operator - (const real value) const {
    return Vector3(coords[0] - value, coords[1] - value, coords[2] - value);
  }

  inline
  bool Vector3::operator < (const Vector3 &vector) const {
    if (coords[0] == vector.coords[0]) {
      if (coords[1] == vector.coords[1]) {
        return coords[2] < vector.coords[2];
      } else return coords[1] < vector.coords[1];
    } else return coords[0] < vector.coords[0];
  }

  inline
  real Vector3::GetLength() const {
    float length = sqrt(std::pow(coords[0], 2) + std::pow(coords[1], 2) +
                        std::pow(coords[2], 2));

    if (length < 0.000001) length = 0;
    return length;
  }

  inline
  real Vector3::GetSquaredLength() const {
    return coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2];
  }

  inline
  radian Vector3::GetAngle2D() const {
    real angle = std::atan2(coords[1], coords[0]);
    if (angle < 0) angle += 2 * pi;
    return angle;
  }

  inline
  radian Vector3::GetAngle2D(const Vector3 &test) const {
    radian angle =
        -std::atan2(coords[0] * test.coords[1] - coords[1] * test.coords[0],
                    coords[0] * test.coords[0] + coords[1] * test.coords[1]);
    angle = ModulateIntoRange(-pi, pi, angle);
    return angle;
  }

  inline
  void Vector3::Rotate(const Quaternion &quat) {

    // cross product
    float uvx = coords[2] * quat.elements[1] - coords[1] * quat.elements[2];
    float uvy = coords[0] * quat.elements[2] - coords[2] * quat.elements[0];
    float uvz = coords[1] * quat.elements[0] - coords[0] * quat.elements[1];

    // cross product
    float uuvx = uvz * quat.elements[1] - uvy * quat.elements[2];
    float uuvy = uvx * quat.elements[2] - uvz * quat.elements[0];
    float uuvz = uvy * quat.elements[0] - uvx * quat.elements[1];

    uvx *= 2.0f * quat.elements[3];
    uvy *= 2.0f * quat.elements[3];
    uvz *= 2.0f * quat.elements[3];
    uuvx *= 2.0f;
    uuvy *= 2.0f;
    uuvz *= 2.0f;

    coords[0] += uvx + uuvx;
    coords[1] += uvy + uuvy;
    coords[2] += uvz + uuvz;
  }

  inline
  void Vector3::Rotate2D(const radian angle) {
    real x = (coords[0] * std::cos(angle)) - (coords[1] * std::sin(angle));
    real y = (coords[1] * std::cos(angle)) + (coords[0] * std::sin(angle));
    coords[0] = x;
    coords[1] = y;
  }

  inline
  Vector3 Vector3::GetRotated2D(const radian angle) const {
    Vector3 result;
    real x = (coords[0] * std::cos(angle)) - (coords[1] * std::sin(angle));
    real y = (coords[1] * std::cos(angle)) + (coords[0] * std::sin(angle));
    result.coords[0] = x;
    result.coords[1] = y;
    result.coords[2] = coords[2];
    return result;
  }

  inline
  Vector3 Vector3::Get2D() const {
    Vector3 result(coords[0], coords[1], 0);
    return result;
  }

}

#endif
