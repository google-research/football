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

#include "../../main.hpp"

namespace blunted {

  unsigned int fastrandseed = 0;

  real clamp(const real value, const real min, const real max) {
    DO_VALIDATION;
    assert(max >= min);
    if (min > value) return min;
    if (max < value) return max;
    return value;
  }

  real NormalizedClamp(const real value, const real min, const real max) {
    DO_VALIDATION;
    assert(max > min);
    real banana = clamp(value, min, max);
    banana = (banana - min) / (max - min);
    return banana;
  }

  float dot_product(real v1[3], real v2[3]) {
    DO_VALIDATION;
    return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
  }

  void normalize(real v[3]) {
    DO_VALIDATION;
    real f = 1.0f / std::sqrt(dot_product(v, v));

    v[0] *= f;
    v[1] *= f;
    v[2] *= f;
  }

  signed int signSide(real n) {
    DO_VALIDATION;
    return n >= 0 ? 1 : -1;
  }

  bool is_odd(int n) {
    DO_VALIDATION;
    return n & 1;
  }

  void randomseed(unsigned int seed) {
    DO_VALIDATION;
    GetContext().rng.engine().seed(seed);
    GetContext().rng_non_deterministic.engine().seed(seed);
  }

  inline real boostrandom() {
    DO_VALIDATION;
    return GetContext().rng();
  }

  real boostrandom(real min, real max) {
    DO_VALIDATION;
    float stretch = max - min;
    real value = min + (boostrandom() * stretch);
    return value;
  }

  real random_non_determ(real min, real max) {
    DO_VALIDATION;
    float stretch = max - min;
    real value = min + (GetContext().rng_non_deterministic() * stretch);
    return value;
  }

  radian ModulateIntoRange(real min, real max, radian value) {
    DO_VALIDATION;
    real step = max - min;
    real newValue = value;
    while (newValue < min) newValue += step;
    while (newValue > max) newValue -= step;
    return newValue;
  }
}
