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

/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmath>

#include "perlin.h"

#include "../base/math/bluntmath.hpp"

using namespace blunted;

#define B SAMPLE_SIZE
#define BM (SAMPLE_SIZE-1)

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ( t * t * (3.0f - 2.0f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.0f;

float Perlin::noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int i, j;

	if (mStart)
  {
    srand(mSeed);
		mStart = false;
		init();
	}

	setup(0,bx0,bx1,rx0,rx1);
	setup(1,by0,by1,ry0,ry1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

  #define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[b00];
	u = at2(rx0,ry0);
	q = g2[b10];
	v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[b01];
	u = at2(rx0,ry1);
	q = g2[b11];
	v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

void Perlin::normalize2(float v[2])
{
	float s = 0.0f;

        s = (float)std::sqrt(v[0] * v[0] + v[1] * v[1]);
        s = 1.0f / s;
        v[0] = v[0] * s;
        v[1] = v[1] * s;
}

void Perlin::normalize3(float v[3])
{
	float s = 0.0f;

        s = (float)std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        s = 1.0f / s;

        v[0] = v[0] * s;
        v[1] = v[1] * s;
	v[2] = v[2] * s;
}

void Perlin::init(void)
{
	int i, j, k;

	for (i = 0 ; i < B ; i++)
  {
		p[i] = i;
		g1[i] = (float)((rand() % (B + B)) - B) / B;
		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize2(g2[i]);
		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize3(g3[i]);
	}

	while (--i)
  {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++)
  {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}

}


float Perlin::perlin_noise_2D(float vec[2])
{
  int terms    = mOctaves;
	//float freq   = mFrequency;
	float result = 0.0f;
  float amp = mAmplitude;

  vec[0]*=mFrequency;
  vec[1]*=mFrequency;

	for( int i=0; i<terms; i++ )
	{
		result += noise2(vec)*amp;
		vec[0] *= 2.0f;
		vec[1] *= 2.0f;
    amp*=0.5f;
	}


	return result;
}



Perlin::Perlin(int octaves,float freq,float amp,int seed)
{
  mOctaves = octaves;
  mFrequency = freq;
  mAmplitude = amp;
  mSeed = seed;
  mStart = true;
}

