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

#ifndef PERLIN_H_
#define PERLIN_H_

#include <stdlib.h>


#define SAMPLE_SIZE 1024

// (c) http://www.flipcode.com/archives/Perlin_Noise_Class.shtml

class Perlin {

  public:
    Perlin(int octaves, float freq, float amp, int seed);

    float Get(float x, float y) {
      float vec[2];
      vec[0] = x;
      vec[1] = y;
      return perlin_noise_2D(vec);
    };

   private:
    float perlin_noise_2D(float vec[2]);

    float noise2(float vec[2]);
    void normalize2(float v[2]);
    void normalize3(float v[3]);
    void init(void);

    int   mOctaves;
    float mFrequency = 0.0f;
    float mAmplitude = 0.0f;
    int   mSeed;

    int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
    float g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
    float g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
    float g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
    bool  mStart;

};

#endif

