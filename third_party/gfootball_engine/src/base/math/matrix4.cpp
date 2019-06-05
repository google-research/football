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

#include "matrix4.hpp"

#include <cmath>

namespace blunted {

  Matrix4::Matrix4() {
    for (int i = 0; i < 16; i++) {
      elements[i] = 0;
    }
  }

  Matrix4::Matrix4(real values[16]) {
    for (int i = 0; i < 16; i++) {
      elements[i] = values[i];
    }
  }

  Matrix4::~Matrix4() {
  }


  // ----- operator overloading

  void Matrix4::operator = (const Matrix3 &mat3) {
    elements[0] = mat3.elements[0];
    elements[1] = mat3.elements[1];
    elements[2] = mat3.elements[2];

    elements[4] = mat3.elements[3];
    elements[5] = mat3.elements[4];
    elements[6] = mat3.elements[5];

    elements[8] = mat3.elements[6];
    elements[9] = mat3.elements[7];
    elements[10] = mat3.elements[8];
  }

  Matrix4 Matrix4::operator * (const Matrix4 &multiplier) const {
    Matrix4 result;

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        result.elements[r * 4 + c] =
          elements[r * 4 + 0] * multiplier.elements[0 + c] +
          elements[r * 4 + 1] * multiplier.elements[4 + c] +
          elements[r * 4 + 2] * multiplier.elements[8 + c] +
          elements[r * 4 + 3] * multiplier.elements[12 + c];
      }
    }

    return result;
  }

  bool Matrix4::operator == (const Matrix4 &mat) {
    for (int i = 0; i < 16; i++) {
      if (elements[i] != mat.elements[i]) return false;
    }
    return true;
  }

  bool Matrix4::operator != (const Matrix4 &mat) {
    return !(*this == mat);
  }


  // ----- mathematics!!! don't we just love it

  Matrix4 Matrix4::GetInverse() const {

    real mat[16];
    real dst[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int i = 0; i < 16; i++) {
      mat[i] = elements[i];
    }

    real    tmp[12]; /* temp array for pairs                      */
    real    src[16]; /* array of transpose source matrix */
    real    det;     /* determinant                                  */
    /* transpose matrix */
    for (int i = 0; i < 4; i++) {
        src[i]        = mat[i*4];
        src[i + 4]    = mat[i*4 + 1];
        src[i + 8]    = mat[i*4 + 2];
        src[i + 12]   = mat[i*4 + 3];
    }
    /* calculate pairs for first 8 elements (cofactors) */
    tmp[0]  = src[10] * src[15];
    tmp[1]  = src[11] * src[14];
    tmp[2]  = src[9]  * src[15];
    tmp[3]  = src[11] * src[13];
    tmp[4]  = src[9]  * src[14];
    tmp[5]  = src[10] * src[13];
    tmp[6]  = src[8]  * src[15];
    tmp[7]  = src[11] * src[12];
    tmp[8]  = src[8]  * src[14];
    tmp[9]  = src[10] * src[12];
    tmp[10] = src[8]  * src[13];
    tmp[11] = src[9]  * src[12];
    /* calculate first 8 elements (cofactors) */
    dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
    dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
    dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
    dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
    dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
    dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
    dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
    dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
    dst[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
    dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
    dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
    dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
    dst[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
    dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
    dst[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
    dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
    /* calculate pairs for second 8 elements (cofactors) */
    tmp[0]  = src[2]*src[7];
    tmp[1]  = src[3]*src[6];
    tmp[2]  = src[1]*src[7];
    tmp[3]  = src[3]*src[5];
    tmp[4]  = src[1]*src[6];
    tmp[5]  = src[2]*src[5];
    tmp[6]  = src[0]*src[7];
    tmp[7]  = src[3]*src[4];
    tmp[8]  = src[0]*src[6];
    tmp[9]  = src[2]*src[4];
    tmp[10] = src[0]*src[5];
    tmp[11] = src[1]*src[4];
    /* calculate second 8 elements (cofactors) */
    dst[8]  = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
    dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
    dst[9]  = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
    dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
    dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
    dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
    dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
    dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
    dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
    dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
    dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
    dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
    dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
    dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
    dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
    dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
    /* calculate determinant */
    det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
    /* calculate matrix inverse */
    if (det != 0.0f) {
      det = 1/det;
      for (int j = 0; j < 16; j++)
          dst[j] *= det;
    }

    Matrix4 tmpm(dst);
    return tmpm;
  }

  void Matrix4::Transpose() {
    Matrix4 tmp = GetTransposed();
    for (int i = 0; i < 16; i++) {
      elements[i] = tmp.elements[i];
    }
  }

  Matrix4 Matrix4::GetTransposed() const {
    Matrix4 matrix;

    matrix.elements[0] = elements[0];
    matrix.elements[1] = elements[4];
    matrix.elements[2] = elements[8];
    matrix.elements[3] = elements[12];

    matrix.elements[4] = elements[1];
    matrix.elements[5] = elements[5];
    matrix.elements[6] = elements[9];
    matrix.elements[7] = elements[13];

    matrix.elements[8] = elements[2];
    matrix.elements[9] = elements[6];
    matrix.elements[10] = elements[10];
    matrix.elements[11] = elements[14];

    matrix.elements[12] = elements[3];
    matrix.elements[13] = elements[7];
    matrix.elements[14] = elements[11];
    matrix.elements[15] = elements[15];

    return matrix;
  }

  void Matrix4::SetTranslation(const Vector3 &trans) {
    elements[3] = trans.coords[0];
    elements[7] = trans.coords[1];
    elements[11] = trans.coords[2];
  }

  Vector3 Matrix4::GetTranslation() const {
    return Vector3(elements[3], elements[7], elements[11]);
  }

  void Matrix4::Translate(const Vector3 &trans) {
    elements[3] += trans.coords[0];
    elements[7] += trans.coords[1];
    elements[11] += trans.coords[2];
  }

  Matrix4 Matrix4::GetTranslated(const Vector3 &trans) {
    Matrix4 tmp(elements);
    tmp.elements[3] += trans.coords[0];
    tmp.elements[7] += trans.coords[1];
    tmp.elements[11] += trans.coords[2];
    return tmp;
  }

  void Matrix4::SetScale(const Vector3 &scale) {
    elements[0] = scale.coords[0];
    elements[5] = scale.coords[1];
    elements[10] = scale.coords[2];
  }

  Vector3 Matrix4::GetScale() const {
    return Vector3(elements[0], elements[5], elements[10]);
  }

  void Matrix4::Construct(const Vector3 &position, const Vector3 &scale, const Quaternion &rotation) {
    // credit to the ogre3d crew
    // http://www.ogre3d.org/

    // convert rotation quaternion to 3x3 rotation matrix
    Matrix3 rot3x3;
    rotation.ConstructMatrix(rot3x3);

    // create a scale matrix
    Matrix3 scale3x3;
    scale.ConstructMatrix(scale3x3);

    // set up final matrix with scale, rotation and translation
    *this = rot3x3 * scale3x3;

    this->SetTranslation(position);

    // no projection term
    elements[12] = 0;
    elements[13] = 0;
    elements[14] = 0;
    elements[15] = 1;
  }

  void Matrix4::ConstructInverse(const Vector3 &position, const Vector3 &scale, const Quaternion &rotation) {
    // credit to the ogre3d crew
    // http://www.ogre3d.org/

    // invert parameters
    Vector3 position_inverse = -position;
    Vector3 scale_inverse(1 / scale.coords[0], 1 / scale.coords[1], 1 / scale.coords[2]);
    Quaternion rotation_inverse = rotation.GetInverse();

    position_inverse *= scale_inverse;
    position_inverse = rotation_inverse * position_inverse;

    Matrix3 rot3x3;
    rotation_inverse.ConstructMatrix(rot3x3);

    // create a scale matrix
    Matrix3 scale3x3;
    scale_inverse.ConstructMatrix(scale3x3);

    // set up final matrix with scale, rotation and translation
    *this = scale3x3 * rot3x3;

    this->SetTranslation(position_inverse);

    // no projection term
    elements[12] = 0;
    elements[13] = 0;
    elements[14] = 0;
    elements[15] = 1;
  }

  void Matrix4::MultiplyVec4(float x, float y, float z, float w, float &rx, float &ry, float &rz, float &rw) {
    rx = elements[0] * x + elements[1] * y + elements[2] * z + elements[3] * w;
    ry = elements[4] * x + elements[5] * y + elements[6] * z + elements[7] * w;
    rz = elements[8] * x + elements[9] * y + elements[10] * z + elements[11] * w;
    rw = elements[12] * x + elements[13] * y + elements[14] * z + elements[15] * w;
  }

  void Matrix4::ConstructProjection(float fov, float aspect, float zNear, float zFar) {

    // https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/

    float top = zNear * std::tan(fov * (pi / 360.0f));
    float bottom = -top;
    float right = top * aspect;
    float left = -right;//bottom * aspect;

    volatile float zFar_min_zNear = 0;
    if (fabs(zFar - zNear) > EPSILON) {
      zFar_min_zNear = 1.0f / (zFar - zNear);
    }

    elements[0] = right != left ? (2 * zNear) / (right - left) : 0;
    elements[1] = 0;
    elements[2] = 0;
    elements[3] = 0;

    elements[4] = 0;
    elements[5] = fabs(top - bottom) > EPSILON ? (2 * zNear) / (top - bottom) : 0;
    elements[6] = 0;
    elements[7] = 0;

    elements[8] = fabs(right - left) > EPSILON ? (right + left) / (right - left) : 0;
    elements[9] = fabs(top - bottom) > EPSILON ? (top + bottom) / (top - bottom) : 0;
    elements[10] = -(zFar + zNear) * zFar_min_zNear;
    elements[11] = -1;

    elements[12] = 0;
    elements[13] = 0;
    elements[14] = -(2 * zFar * zNear) * zFar_min_zNear;
    elements[15] = 0;

    Transpose(); // to non-opengl
  }

  void Matrix4::ConstructOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {

    // https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/

    elements[0] = fabs(right - left) > EPSILON ? 2 / (right - left) : 0;
    elements[1] = 0;
    elements[2] = 0;
    elements[3] = 0;

    elements[4] = 0;
    elements[5] = fabs(top - bottom) > EPSILON ? 2 / (top - bottom) : 0;
    elements[6] = 0;
    elements[7] = 0;

    elements[8] = 0;
    elements[9] = 0;
    elements[10] = fabs(zFar - zNear) > EPSILON ? -(2 / (zFar - zNear)): 0;
    elements[11] = 0;

    elements[12] = fabs(right - left) > EPSILON ? -((right + left) / (right - left)) : 0;
    elements[13] = fabs(top - bottom) > EPSILON ? -((top + bottom) / (top - bottom)) : 0;
    elements[14] = fabs(zFar - zNear) > EPSILON ? -((zFar + zNear) / (zFar - zNear)) : 0;
    elements[15] = 1;

    Transpose(); // to non-opengl
  }

}
