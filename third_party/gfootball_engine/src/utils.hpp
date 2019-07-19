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

#ifndef _HPP_FOOTBALL_UTILS
#define _HPP_FOOTBALL_UTILS

#include "gamedefines.hpp"

#include "scene/objects/camera.hpp"

#include <boost/circular_buffer.hpp>

using namespace blunted;

float GetQuantizedDirectionBias();
void QuantizeDirection(Vector3 &inputDirection, float bias = 1.0f);
Vector3 GetProjectedCoord(const Vector3 &pos3D, boost::intrusive_ptr<Camera> camera);

int GetVelocityID(e_Velocity velo, bool treatDribbleAsWalk = false);


// stats fiddling

enum PlayerStat {
  physical_balance,
  physical_reaction,
  physical_acceleration,
  physical_velocity,
  physical_stamina,
  physical_agility,
  physical_shotpower,
  technical_standingtackle,
  technical_slidingtackle,
  technical_ballcontrol,
  technical_dribble,
  technical_shortpass,
  technical_highpass,
  technical_header,
  technical_shot,
  technical_volley,
  mental_calmness,
  mental_workrate,
  mental_resilience,
  mental_defensivepositioning,
  mental_offensivepositioning,
  mental_vision,
  player_stat_max
};

enum e_PositionName {
  e_PositionName_GK,
  e_PositionName_SW,
  e_PositionName_D,
  e_PositionName_WB,
  e_PositionName_DM,
  e_PositionName_M,
  e_PositionName_AM,
  e_PositionName_F,
  e_PositionName_ST
};

struct WeightedPosition {
  e_PositionName positionName;
  float weight = 0.0f;
};

struct Stat {
  PlayerStat name;
  float value = 0.0f;
};

enum e_DevelopmentCurveType {
  e_DevelopmentCurveType_Early,
  e_DevelopmentCurveType_Normal,
  e_DevelopmentCurveType_Late
};

float CalculateStat(float baseStat, float profileStat, float age, e_DevelopmentCurveType developmentCurveType);
/* ^ above one supersedes these
float GetIndividualStat(float averageStat, float profileStat, float age);
float GetAverageStatFromBaseStat(int age, float baseStat, e_DevelopmentCurveType developmentCurveType);
*/

// /stats fiddling

template <typename T> struct TemporalValue {
  TemporalValue() {
    data = 0;
    time_ms = 0;
  }
  T data;
  unsigned long time_ms = 0;
};

template <> TemporalValue<Quaternion>::TemporalValue();

template <typename T> class TemporalSmoother {

  public:
    TemporalSmoother();
    virtual ~TemporalSmoother() {}

    void SetValue(const T &data, unsigned long valueTime_ms);
    T GetValue(unsigned long currentTime_ms, unsigned long history_ms = temporalSmoother_history_ms) const; // get interpolated measurement, history_ms seconds ago from now

  protected:
    T DataMix(const T &data1, const T &data2, float bias = 0.0f) const;

    boost::circular_buffer< TemporalValue<T> > values;
    unsigned int snapshotSize = 0;

};

template <> Quaternion TemporalSmoother<Quaternion>::DataMix(const Quaternion &data1, const Quaternion &data2, float bias) const;

#endif
