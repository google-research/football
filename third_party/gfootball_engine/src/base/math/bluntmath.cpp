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

#include "bluntmath.hpp"

#include <cmath>

#include <boost/random.hpp>

namespace blunted {

  unsigned int fastrandseed = 0;
  unsigned int max_uint = 0;

  typedef boost::mt19937 BaseGenerator;
  typedef boost::uniform_real<float> Distribution;
  typedef boost::variate_generator<BaseGenerator, Distribution> Generator;
  BaseGenerator base;
  Distribution dist;
  Generator rng(base, dist);

  // Two random number generators are needed. One (deterministic when running
  // in deterministic mode) to be used in places which generate deterministic
  // game state. Second one is used in places which are optional and don't
  // affect observations (like position of the sun).
  BaseGenerator base2;
  Distribution dist2;
  Generator rng_non_deterministic(base2, dist2);

  real clamp(const real value, const real min, const real max) {
    assert(max >= min);
    if (min > value) return min;
    if (max < value) return max;
    return value;
  }

  real NormalizedClamp(const real value, const real min, const real max) {
    assert(max > min);
    real banana = clamp(value, min, max);
    banana = (banana - min) / (max - min);
    return banana;
  }

  float dot_product(real v1[3], real v2[3]) {
    return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
  }

  void normalize(real v[3]) {
    real f = 1.0f / std::sqrt(dot_product(v, v));

    v[0] *= f;
    v[1] *= f;
    v[2] *= f;
  }

  signed int signSide(real n) {
    return n >= 0 ? 1 : -1;
  }

  bool is_odd(int n) {
    return n & 1;
  }

  void randomseed(unsigned int seed) {
    rng.engine().seed(seed);
    rng_non_deterministic.engine().seed(seed);
  }

  inline real boostrandom() {
    return rng();
  }

  real random(real min, real max) {
    float stretch = max - min;
    real value = min + (boostrandom() * stretch);
    return value;
  }

  real random_non_determ(real min, real max) {
    float stretch = max - min;
    real value = min + (rng_non_deterministic() * stretch);
    return value;
  }

  real ModulateIntoRange(real min, real max, real value) {
    real step = max - min;
    real newValue = value;
    while (newValue < min) newValue += step;
    while (newValue > max) newValue -= step;
    return newValue;
  }

}
