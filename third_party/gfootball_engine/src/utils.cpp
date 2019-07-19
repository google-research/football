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

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "main.hpp"

#include "utils.hpp"

#include "systems/graphics/resources/texture.hpp"
#include "managers/resourcemanagerpool.hpp"

#include <boost/algorithm/string.hpp>
#include <cmath>

float GetQuantizedDirectionBias() {
  return GetConfiguration()->GetReal("gameplay_quantizeddirectionbias", _default_QuantizedDirectionBias);
}

void QuantizeDirection(Vector3 &inputDirection, float bias) {

  // digitize input

  Vector3 inputDirectionNorm = inputDirection.GetNormalized(0);

  int directions = GetConfiguration()->GetInt("gameplay_quantizeddirectioncount", 8);

  radian angle = inputDirectionNorm.GetAngle2D();
  angle /= pi * 2.0f;
  angle = std::round(angle * directions);
  angle /= directions;
  angle *= pi * 2.0f;

  inputDirection = (inputDirectionNorm * (1.0 - bias) + (Vector3(1, 0, 0).GetRotated2D(angle) * bias)).GetNormalized(inputDirectionNorm) * inputDirection.GetLength();
}

Vector3 GetProjectedCoord(const Vector3 &pos3D, boost::intrusive_ptr<Camera> camera) {
  Matrix4 rotMat;
  rotMat.ConstructInverse(camera->GetDerivedPosition(), Vector3(1, 1, 1), camera->GetDerivedRotation());
  float fov = camera->GetFOV();

  // cotangent
  float f = 1.0 / tan((fov / 360.0 * pi * 2) / 2.0);

  Vector3 contextSize3D = GetGraphicsSystem()->GetContextSize();
  float aspect = contextSize3D.coords[0] / contextSize3D.coords[1];
  float aspect2D = GetMenuTask()->GetWindowManager()->GetAspectRatio();
  if (aspect2D < aspect) aspect = aspect2D;

  //printf("aspect: %f\n", aspect);
  float zNear = 40.0;
  float zFar = 270.0;

  Matrix4 perspMat;
  perspMat.elements[0] = f / aspect;
  perspMat.elements[5] = f;
  perspMat.elements[10] = (zFar + zNear) / (zNear - zFar);
  perspMat.elements[11] = (2 * zFar * zNear) / (zNear - zFar);
  perspMat.elements[14] = -1;

  Matrix4 resMat = perspMat * rotMat;

  float x, y, z, w;
  resMat.MultiplyVec4(pos3D.coords[0], pos3D.coords[1], pos3D.coords[2], 1, x, y, z, w);

  Vector3 result;
  result.coords[0] = x / w;
  result.coords[1] = y / w;

  //printf("%f %f %f\n", x / w, y / w, z / w);

  result.coords[1] = -result.coords[1];

  result.coords[1] *= aspect2D / aspect;

  result += 1.0;
  result *= 0.5;
  result *= 100;

  return result;
}

int GetVelocityID(e_Velocity velo, bool treatDribbleAsWalk) {
  int id = 0;
  switch (velo) {
    case e_Velocity_Idle:
      id = 0;
      break;
    case e_Velocity_Dribble:
      id = 1;
      break;
    case e_Velocity_Walk:
      id = 2;
      break;
    case e_Velocity_Sprint:
      id = 3;
      break;
    default:
      id = 0;
      break;
  }
  if (treatDribbleAsWalk && id > 1) id--;
  return id;
}

std::map < e_PositionName, std::vector<Stat> > defaultProfiles;

float CalculateStat(float baseStat, float profileStat, float age, e_DevelopmentCurveType developmentCurveType) {


  float idealAge = 27;
  float ageFactor = curve( 1.0f - NormalizedClamp(fabs(age - idealAge), 0, 13) * 0.5f , 1.0f) * 2.0f - 1.0f; // 0 .. 1
  assert(ageFactor >= 0.0f && ageFactor <= 1.0f);
  //ageFactor = clamp(ageFactor, 0.0f, 1.0f);

  // this factor should roughly be around 1.0f for good players at their top age.
  float agedBaseStat = baseStat * (ageFactor * 0.5f + 0.5f) * 1.2f;

  float agedProfileStat = clamp(profileStat * 2.0f * agedBaseStat, 0.01f, 1.0f); // profile stat * 2 because average == 0.5

  return agedProfileStat;
}

template <> TemporalValue<Quaternion>::TemporalValue() {
  data = QUATERNION_IDENTITY;
  time_ms = 0;
};

template <typename T> TemporalSmoother<T>::TemporalSmoother() {
  //snapshotSize = 3;
  snapshotSize =
      3 + int(std::ceil(temporalSmoother_history_ms /
                        10.0f));  // not sure how to calculate proper number?
  values = boost::circular_buffer< TemporalValue<T> >(snapshotSize);
}

template <typename T> void TemporalSmoother<T>::SetValue(const T &data, unsigned long valueTime_ms) {
  TemporalValue<T> value;
  value.data = data;
  value.time_ms = valueTime_ms;
  //printf("adding time: %lu\n", value.time_ms);
  values.push_back(value);
}

template <typename T> T TemporalSmoother<T>::GetValue(unsigned long currentTime_ms, unsigned long history_ms) const {

  if (values.size() == 0) {
    TemporalValue<T> bla; // do it like this so the struct constructor is invoked for T
    return bla.data;
  }
  if (values.size() == 1) return (*values.begin()).data; // only one value yet

  //return (values.back()).data; // disable smoother

  unsigned long now_ms = currentTime_ms;
  //unsigned long now_ms = (currentTime_ms == 0) ? EnvironmentManager::GetInstance().GetTime_ms() : currentTime_ms;
  unsigned long targetTime_ms = 0;
  if (history_ms <= now_ms) targetTime_ms = now_ms - history_ms; // this makes sure targetTime_ms won't become negative (and loop-around since it's an unsigned var)


  // find the 2 values we need

  TemporalValue<T> value1;
  TemporalValue<T> value2;

  int t1index = -1;
  int t2index = -1;
  for (unsigned int i = 0; i < values.size(); i++) {
    const TemporalValue<T> *iter = &values[i];
    if ((*iter).time_ms <= targetTime_ms) {
      value1.data = (*iter).data;
      value1.time_ms = (*iter).time_ms;
      t1index = i;
    }
    else if ((*iter).time_ms > targetTime_ms) {
      value2.data = (*iter).data;
      value2.time_ms = (*iter).time_ms;
      t2index = i;
      break;
    }
  }
  if (value1.time_ms == 0) return value2.data;
  if (value2.time_ms == 0) return value1.data;

  float bias = NormalizedClamp(targetTime_ms, value1.time_ms, std::max(value2.time_ms, value1.time_ms + 1));

  return DataMix(value1.data, value2.data, bias);
}

template <typename T> T TemporalSmoother<T>::DataMix(const T &data1, const T &data2, float bias) const {
  return data1 * (1.0f - bias) + data2 * bias;
}

template <> Quaternion TemporalSmoother<Quaternion>::DataMix(const Quaternion &data1, const Quaternion &data2, float bias) const {
  return data1.GetLerped(bias, data2);
}

template class TemporalSmoother<Vector3>;
template class TemporalSmoother<Quaternion>;
template class TemporalSmoother<float>;
template class TemporalSmoother<unsigned long>;
template class TemporalSmoother<int>;
