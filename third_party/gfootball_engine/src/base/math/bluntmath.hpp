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

#ifndef _hpp_bluntmath
#define _hpp_bluntmath

#include "../../defines.hpp"

#include <cmath>
#include <limits>



namespace blunted {

  real clamp(const real value, const real min, const real max);
  real NormalizedClamp(const real value, const real min, const real max);

  // you can never be too specific ;)
  const real pi = 3.1415926535897932384626433832795028841972f; // last decimal rounded ;)
  extern unsigned int fastrandseed;
  extern unsigned int max_uint;

  typedef real radian;

  void normalize(real v[3]);
  signed int signSide(real n);  // returns -1 or 1
  bool is_odd(int n);
  void randomseed(unsigned int seed);
  real random(real min, real max);
  real random_non_determ(real min, real max);

  inline void fastrandomseed(unsigned int seed) {
    fastrandseed = seed;
    max_uint = std::numeric_limits<unsigned int>::max();
  }

  inline real fastrandom(real min, real max) {
    real range = max - min;
    real tmp = (fastrandseed / (max_uint * 1.0f)) * range + min;
    fastrandseed = (214013 * fastrandseed + 2531011);
    return tmp;
  }

  inline float curve(float source, float bias = 1.0f) { // make linear / into sined _/-
    return (std::sin((source - 0.5f) * pi) * 0.5f + 0.5f) * bias +
           source * (1.0f - bias);
  }

  real ModulateIntoRange(real min, real max, real value);
}

#endif
