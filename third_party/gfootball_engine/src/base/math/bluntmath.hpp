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

#include <cmath>
#include <limits>
#include <assert.h>
#include <iostream>
#include "../log.hpp"



namespace blunted {
  typedef float real;

  real clamp(const real value, const real min, const real max);
  real NormalizedClamp(const real value, const real min, const real max);

  // you can never be too specific ;)
  constexpr real pi = 3.1415926535897932384626433832795028841972f; // last decimal rounded ;)
  class radian {
   public:
    radian() { DO_VALIDATION;}
    radian(float r) : _angle(r) { DO_VALIDATION;}
    radian &operator+=(radian r) { DO_VALIDATION;
      _angle += r._angle;
      _rotated ^= r._rotated;
      return *this;
    }
    radian &operator-=(radian r){
      _angle -= r._angle;
      _rotated ^= r._rotated;
      return *this;
    }
    radian &operator/=(radian r) { DO_VALIDATION;
      *this = radian(real(*this) / real(r));
      return *this;
    }
    radian &operator*=(radian r) { DO_VALIDATION;
      *this = radian(real(*this) * real(r));
      return *this;
    }
    operator real() const {
      if (_rotated) { DO_VALIDATION;
        return _angle - pi;
      }
      return _angle;
    }
    void Mirror() { DO_VALIDATION;
      _rotated = !_rotated;
    }
   private:
    float _angle = 0.0f;
    // Was angle rotated by PI.
    bool _rotated = false;
  };

  void normalize(real v[3]);
  signed int signSide(real n);  // returns -1 or 1
  bool is_odd(int n);
  void randomseed(unsigned int seed);
  real boostrandom(real min, real max);
  real random_non_determ(real min, real max);

  inline float curve(float source, float bias = 1.0f) { DO_VALIDATION; // make linear / into sined _/-
    return (std::sin((source - 0.5f) * pi) * 0.5f + 0.5f) * bias +
           source * (1.0f - bias);
  }

  radian ModulateIntoRange(real min, real max, radian value);
}

#endif
