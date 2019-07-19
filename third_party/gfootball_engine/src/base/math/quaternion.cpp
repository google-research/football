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

#include "quaternion.hpp"

#include <cmath>

#include "bluntmath.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"

#include "../../base/log.hpp"

// most of the formulas derived from ogre3d
// lots of credit to the ogre3d crew!
// http://www.ogre3d.org/

namespace blunted {

  Quaternion::Quaternion(real x, real y, real z, real w) {
    elements[0] = x;
    elements[1] = y;
    elements[2] = z;
    elements[3] = w;
  }

  Quaternion::Quaternion(real values[4]) {
    elements[0] = values[0];
    elements[1] = values[1];
    elements[2] = values[2];
    elements[3] = values[3];
  }

  Quaternion::~Quaternion() {
  }

  void Quaternion::Set(real x, real y, real z, real w) {
    elements[0] = x;
    elements[1] = y;
    elements[2] = z;
    elements[3] = w;
  }

  void Quaternion::Set(const Quaternion &quat) {
    elements[0] = quat.elements[0];
    elements[1] = quat.elements[1];
    elements[2] = quat.elements[2];
    elements[3] = quat.elements[3];
  }

  void Quaternion::Set(const Matrix3 &mat) {
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/forum.htm

    real n4; // the norm of quaternion multiplied by 4
    Matrix3 tmp(mat);

    real tr = tmp.elements[0] + tmp.elements[4] + tmp.elements[8]; // trace of matrix
    if (tr > 0.0f) {
      Set(tmp.elements[7] - tmp.elements[5], tmp.elements[2] - tmp.elements[6], tmp.elements[3] - tmp.elements[1], tr + 1.0f);
      n4 = elements[3];
    } else if ( (tmp.elements[0] > tmp.elements[4]) && (tmp.elements[0] > tmp.elements[8]) ) {
      Set(1.0f + tmp.elements[0] - tmp.elements[4] - tmp.elements[8], tmp.elements[1] + tmp.elements[3],
                 tmp.elements[2] + tmp.elements[6], tmp.elements[7] - tmp.elements[5]);
      n4 = elements[0];
    } else if (tmp.elements[4] > tmp.elements[8]) {
      Set(tmp.elements[1] + tmp.elements[3], 1.0f + tmp.elements[4] - tmp.elements[0] - tmp.elements[8],
          tmp.elements[5] + tmp.elements[7], tmp.elements[2] - tmp.elements[6]);
      n4 = elements[1];
    } else {
      Set(tmp.elements[2] + tmp.elements[6], tmp.elements[5] + tmp.elements[7],
          1.0f + tmp.elements[8] - tmp.elements[0] - tmp.elements[4], tmp.elements[3] - tmp.elements[1]);
      n4 = elements[2];
    }
    scale(0.5f / (float)std::sqrt(n4));
  }


  // ----- operator overloading

  bool Quaternion::operator != (const Quaternion &fac) const {
    if (fac.elements[0] != elements[0] ||
        fac.elements[0] != elements[1] ||
        fac.elements[2] != elements[2] ||
        fac.elements[3] != elements[3]) {
      return true;
    } else {
      return false;
    }
  }

  const Quaternion Quaternion::operator * (float scale) const {
    return Quaternion(elements[0] * scale, elements[1] * scale, elements[2] * scale, elements[3] * scale);
  }

  Vector3 Quaternion::operator * (const Vector3 &fac) const {

    // nVidia SDK implementation
    Vector3 uv, uuv;
    Vector3 qvec(elements[0], elements[1], elements[2]);
    uv = qvec.GetCrossProduct(fac);
    uuv = qvec.GetCrossProduct(uv);
    uv *= (2.0f * elements[3]);
    uuv *= 2.0f;

    return fac + uv + uuv;

  }

  void Quaternion::operator = (const Vector3 &vec) {

    Vector3 z = vec;
    z.Normalize();
    Vector3 y(0, -1, 0);
    Vector3 x = y.GetCrossProduct(z); // x = y cross z
    x.Normalize();
    y = z.GetCrossProduct(x); // y = z cross x

    Matrix3 mat(x.coords[0], x.coords[1], x.coords[2],
                y.coords[0], y.coords[1], y.coords[2],
                z.coords[0], z.coords[1], z.coords[2]);

    this->Set(mat);
  }

  Quaternion Quaternion::operator * (const Quaternion &fac) const {
    Quaternion result;

    result.elements[0] = elements[3] * fac.elements[0] + elements[0] * fac.elements[3] + elements[1] * fac.elements[2] - elements[2] * fac.elements[1];
    result.elements[1] = elements[3] * fac.elements[1] + elements[1] * fac.elements[3] + elements[2] * fac.elements[0] - elements[0] * fac.elements[2];
    result.elements[2] = elements[3] * fac.elements[2] + elements[2] * fac.elements[3] + elements[0] * fac.elements[1] - elements[1] * fac.elements[0];
    result.elements[3] = elements[3] * fac.elements[3] - elements[0] * fac.elements[0] - elements[1] * fac.elements[1] - elements[2] * fac.elements[2];

    return result;
  }

  Quaternion Quaternion::operator + (const Quaternion &q2) const {
    return Quaternion(elements[0] + q2.elements[0], elements[1] + q2.elements[1], elements[2] + q2.elements[2], elements[3] + q2.elements[3]);
  }

  Quaternion Quaternion::operator - (const Quaternion &q2) const {
    return Quaternion(elements[0] - q2.elements[0], elements[1] - q2.elements[1], elements[2] - q2.elements[2], elements[3] - q2.elements[3]);
  }

  Quaternion Quaternion::operator - () const {
    return Quaternion(-elements[0], -elements[1], -elements[2], -elements[3]);
  }


  // ----- mathematics

  Quaternion Quaternion::GetInverse() const {
    real fnorm = GetMagnitude();
    if (fnorm < 0.000001f) {
      //Log(e_Warning, "Quaternion", "GetInverse", "Unable to normalize quaternion");
      return QUATERNION_IDENTITY;
    } else {
      real finvnorm = 1.0 / fnorm;
      return Quaternion(-elements[0] * finvnorm, -elements[1] * finvnorm, -elements[2] * finvnorm, elements[3] * finvnorm);
    }
  }

  void Quaternion::ConstructMatrix(Matrix3 &rotation) const {

    real xx      = elements[0] * elements[0];
    real xy      = elements[0] * elements[1];
    real xz      = elements[0] * elements[2];
    real xw      = elements[0] * elements[3];

    real yy      = elements[1] * elements[1];
    real yz      = elements[1] * elements[2];
    real yw      = elements[1] * elements[3];

    real zz      = elements[2] * elements[2];
    real zw      = elements[2] * elements[3];

    rotation.elements[0]  = 1 - 2 * ( yy + zz );
    rotation.elements[1]  =     2 * ( xy - zw );
    rotation.elements[2]  =     2 * ( xz + yw );

    rotation.elements[3]  =     2 * ( xy + zw );
    rotation.elements[4]  = 1 - 2 * ( xx + zz );
    rotation.elements[5]  =     2 * ( yz - xw );

    rotation.elements[6]  =     2 * ( xz - yw );
    rotation.elements[7]  =     2 * ( yz + xw );
    rotation.elements[8]  = 1 - 2 * ( xx + yy );
  }

  void Quaternion::GetAngles(radian &X, radian &Y, radian &Z) const {
    int x = 0;
    int y = 2;
    int z = 1;

    float singularityTest = elements[x] * elements[y] + elements[z] * elements[3];
    if (singularityTest > 0.49999 || singularityTest < -0.49999) { // north and south pole
      if (singularityTest > 0) {
        Z = 2 * std::atan2(elements[x], elements[z]);
        Y = pi * 0.5;
      }
      if (singularityTest < 0) {
        Z = -2 * std::atan2(elements[x], elements[z]);
        Y = -pi * 0.5;
      }
      X = 0;
      return;
    }
    real sqx = elements[x] * elements[x];
    real sqy = elements[y] * elements[y];
    real sqz = elements[z] * elements[z];
    Z = std::atan2(
        2 * elements[y] * elements[3] - 2 * elements[x] * elements[z],
        1 - 2 * sqy - 2 * sqz);
    Y = std::asin(2 * elements[x] * elements[y] +
                  2 * elements[z] * elements[3]);
    X = std::atan2(
        2 * elements[x] * elements[3] - 2 * elements[y] * elements[z],
        1 - 2 * sqx - 2 * sqz);
  }

  void Quaternion::SetAngles(radian X, radian Y, radian Z) {
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
    // assuming the angles are in radians.

    float c1 = cos(Y / 2.0);
    float s1 = sin(Y / 2.0);
    float c2 = cos(Z / 2.0);
    float s2 = sin(Z / 2.0);
    float c3 = cos(X / 2.0);
    float s3 = sin(X / 2.0);
    float c1c2 = c1 * c2;
    float s1s2 = s1 * s2;
    elements[3] = c1c2 * c3 - s1s2 * s3;
    elements[0] = c1c2 * s3 + s1s2 * c3;
    elements[1] = s1 * c2 * c3 + c1 * s2 * s3;
    elements[2] = c1 * s2 * c3 - s1 * c2 * s3;

  }

  void Quaternion::GetAngleAxis(radian& rfangle, Vector3& rkaxis) const {
    // the quaternion representing the rotation is
    // q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle

    rfangle = 2.0f * std::acos(elements[3]);

    double div = std::sqrt(1.0f - elements[3] * elements[3]);
    if (div < 0.000001f) {
      rkaxis.coords[0] = elements[0];
      rkaxis.coords[1] = elements[1];
      rkaxis.coords[2] = elements[2];
    } else {
      rkaxis.coords[0] = elements[0] / div;
      rkaxis.coords[1] = elements[1] / div;
      rkaxis.coords[2] = elements[2] / div;
    }
  }

  void Quaternion::SetAngleAxis(const radian& rfangle, const Vector3& rkaxis) {
    // assert: rkaxis[] is unit length
    //
    // the quaternion representing the rotation is
    // q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

    radian fhalfangle = 0.5f * rfangle;
    real fsin = sinf(fhalfangle);
    elements[0] = fsin * rkaxis.coords[0];
    elements[1] = fsin * rkaxis.coords[1];
    elements[2] = fsin * rkaxis.coords[2];
    elements[3] = cosf(fhalfangle);
  }

  void Quaternion::conjugate() {
    elements[0] = -elements[0];
    elements[1] = -elements[1];
    elements[2] = -elements[2];
  }

  Quaternion Quaternion::conjugate_get() const {
    return Quaternion(-elements[0], -elements[1], -elements[2], elements[3]);
  }

  void Quaternion::scale(const real fac) {
    for (int i = 0; i < 4; i++) {
      elements[i] *= fac;
    }
  }

  real Quaternion::GetMagnitude() const {
    if (elements[0] == 0.0f && elements[1] == 0.0f && elements[2] == 0.0f && elements[3] == 0.0f) return 0.0f;
    real magnitude =
        std::sqrt(elements[0] * elements[0] + elements[1] * elements[1] +
                  elements[2] * elements[2] + elements[3] * elements[3]);
    if (magnitude < 0.000001) return 0.0f;
    return magnitude;
  }

  void Quaternion::Normalize() {
    // http://stackoverflow.com/questions/11667783/quaternion-and-normalization
    double qmagsq = elements[0] * elements[0] + elements[1] * elements[1] + elements[2] * elements[2] + elements[3] * elements[3]; // squared magnitude
    if (qmagsq < 0.000001f) {
      Set(QUATERNION_IDENTITY[0], QUATERNION_IDENTITY[1], QUATERNION_IDENTITY[2], QUATERNION_IDENTITY[3]);
    } else {
      if (abs(1.0 - qmagsq) < 2.107342e-08) {
        scale(2.0 / (1.0 + qmagsq));
      } else {
        scale(1.0 / sqrt(qmagsq));
      }
    }

  }

  Quaternion Quaternion::GetNormalized() const {
    Quaternion tmp(*this);
    tmp.Normalize();
    return tmp;
  }

  float Quaternion::GetDotProduct(const Quaternion &subject) const {
    return (elements[0] * subject.elements[0] + elements[1] * subject.elements[1] + elements[2] * subject.elements[2] + elements[3] * subject.elements[3]);
  }

  Quaternion Quaternion::GetLerped(float bias, const Quaternion &to) const {
    return (*this * (1 - bias) + to * bias).GetNormalized();
  }

  Quaternion Quaternion::GetSlerped(float bias, const Quaternion &to) const {

    // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm

    // quaternion to return
    Quaternion qm;
    Quaternion qb = to;

    // Calculate angle between them.
    double cosHalfTheta = elements[3] * qb.elements[3] + elements[0] * qb.elements[0] + elements[1] * qb.elements[1] + elements[2] * qb.elements[2];

    if (cosHalfTheta < 0) {
      qb = -qb;
      cosHalfTheta = -cosHalfTheta;
    }

    // if *this=to or *this=-to then theta = 0 and we can return *this
    if (fabs(cosHalfTheta) >= 1.0) {
      qm.elements[3] = elements[3];
      qm.elements[0] = elements[0];
      qm.elements[1] = elements[1];
      qm.elements[2] = elements[2];
      return qm;
    }

    // Calculate temporary values.
    double halfTheta = acos(cosHalfTheta);
    double sinHalfTheta = sqrt(1.0 - cosHalfTheta * cosHalfTheta);

    // if theta = 180 degrees then result is not fully defined
    // we could rotate around any axis normal to *this or to
    if (fabs(sinHalfTheta) < 0.000001f) { // fabs is floating point absolute
      qm.elements[3] = (elements[3] * 0.5 + qb.elements[3] * 0.5);
      qm.elements[0] = (elements[0] * 0.5 + qb.elements[0] * 0.5);
      qm.elements[1] = (elements[1] * 0.5 + qb.elements[1] * 0.5);
      qm.elements[2] = (elements[2] * 0.5 + qb.elements[2] * 0.5);
      return qm;
    }

    double ratioA = sin((1 - bias) * halfTheta) / sinHalfTheta;
    double ratioB = sin(bias * halfTheta) / sinHalfTheta;

    //calculate Quaternion.
    qm.elements[3] = (elements[3] * ratioA + qb.elements[3] * ratioB);
    qm.elements[0] = (elements[0] * ratioA + qb.elements[0] * ratioB);
    qm.elements[1] = (elements[1] * ratioA + qb.elements[1] * ratioB);
    qm.elements[2] = (elements[2] * ratioA + qb.elements[2] * ratioB);

    return qm;
  }

  Quaternion Quaternion::GetRotationTo(const Quaternion &to) const {
    // http://stackoverflow.com/questions/8781129/when-i-have-two-orientation-quaternions-how-do-i-find-the-rotation-quaternion-n
    return to * this->GetInverse();
  }

  Quaternion Quaternion::GetRotationMultipliedBy(float factor) const {

    Quaternion result;

    Vector3 axis;
    radian angle;
    GetAngleAxis(angle, axis);

    if (angle > pi) angle -= 2.0f * pi; // range -pi .. pi

    angle = std::fmod(angle * factor, 2.0f * pi);  // remove multiples of 2pi

    result.SetAngleAxis(angle, axis);

    return result.GetNormalized();
  }

  float Quaternion::MakeSameNeighborhood(const Quaternion &src) {
    float dot = GetDotProduct(src);
    if (dot < 0) {
      dot = -dot;
      *this = -*this;
    }
    return dot;
  }

}
