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

#version 150

#pragma optimize(on)

// http://www.gamedev.net/community/forums/topic.asp?topic_id=463075&PageSize=25&WhichPage=4
// by ArKano22

uniform sampler2D map_albedo; // 0
uniform sampler2D map_normal; // 1
uniform sampler2D map_depth;  // was: 3, now: 2
uniform sampler2D map_aux;    // was: 2, now: 3
uniform sampler2D map_noise;  // 4

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionViewMatrix;

uniform float contextWidth;
uniform float contextHeight;
uniform float contextX;
uniform float contextY;

uniform vec2 cameraClip;

const int SSAO_kernelSize = 32; // don't make larger than this: 32 is hardcoded in source, so no more sample positions exist
//uniform int SSAO_kernelSize; // won't work until glsl 4.3 or so.
uniform vec3 SSAO_kernel[SSAO_kernelSize];

out vec4 stdout0;
out vec4 stdout1;

// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150% etc
vec3 ContrastSaturationBrightness(vec3 color, float brt, float con, float sat) {
  // Increase or decrease theese values to adjust r, g and b color channels separately
  const float AvgLumR = 0.0;
  const float AvgLumG = 0.0;
  const float AvgLumB = 0.0;

  const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);

  vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
  vec3 brtColor = color * brt;
  vec3 intensity = vec3(dot(brtColor, LumCoeff));
  vec3 satColor = mix(intensity, brtColor, sat);
  vec3 conColor = mix(AvgLumin, satColor, con);

  return conColor;
}

float GetEdge(vec2 pos) {
  vec2 offsets[9];
  offsets[0] = vec2(0.0, 0.0);
  offsets[1] = vec2(-1.0 / (contextWidth * 0.001), -1.0 / (contextHeight * 0.001)); // round the clock ;)
  offsets[2] = vec2(0.0, -1.0 / (contextHeight * 0.001));
  offsets[3] = vec2(1.0 / (contextWidth * 0.001), -1.0 / (contextHeight * 0.001));
  offsets[4] = vec2(1.0 / (contextWidth * 0.001), 0.0);
  offsets[5] = vec2(1.0 / (contextWidth * 0.001), 1.0 / (contextHeight * 0.001));
  offsets[6] = vec2(0.0, 1.0 / (contextHeight * 0.001));
  offsets[7] = vec2(-1.0 / (contextWidth * 0.001), 1.0 / (contextHeight * 0.001));
  offsets[8] = vec2(-1.0 / (contextWidth * 0.001), 0.0);

  float depths[9];
  vec3 normals[9];
  for (int i = 0; i < 9; i++) {
    float depth = texture2D(map_depth, pos + offsets[i] * 0.001).x;
    depths[i] = cameraClip.y / (depth - cameraClip.x);
    normals[i] = texture2D(map_normal, pos + offsets[i] * 0.001).xyz;
  }

  vec4 deltas1;
  vec4 deltas2;
  deltas1.x = depths[1];
  deltas1.y = depths[2];
  deltas1.z = depths[3];
  deltas1.w = depths[4];
  deltas2.x = depths[5];
  deltas2.y = depths[6];
  deltas2.z = depths[7];
  deltas2.w = depths[8];

  deltas1 = abs(deltas1 - depths[0]);
  deltas2 = abs(depths[0] - deltas2);

  vec4 minDeltas = max(min(deltas1, deltas2), 0.00001);
  vec4 maxDeltas = max(deltas1, deltas2);

  vec4 depthResults = step(minDeltas * 35.0, maxDeltas);

  deltas1.x = dot(normals[1], normals[0]);
  deltas1.y = dot(normals[2], normals[0]);
  deltas1.z = dot(normals[3], normals[0]);
  deltas1.w = dot(normals[4], normals[0]);
  deltas2.x = dot(normals[5], normals[0]);
  deltas2.y = dot(normals[6], normals[0]);
  deltas2.z = dot(normals[7], normals[0]);
  deltas2.w = dot(normals[8], normals[0]);
  deltas1 = abs(deltas1 - deltas2);

  vec4 normalResults = step(0.5, deltas1);
  vec4 results = max(normalResults, depthResults);

  return (results.x + results.y + results.z + results.w) * 0.25;
}

vec3 GetWorldPosition(vec2 texCoord, float depth) {
  vec4 projectedPos = vec4(texCoord.x * 2 - 1, texCoord.y * 2 - 1, depth * 2 - 1, 1.0f);
  vec4 worldPosition = inverseProjectionViewMatrix * projectedPos;
  worldPosition.xyz /= worldPosition.w;
  return worldPosition.xyz;
}

void main(void) {

  vec2 texCoord = gl_FragCoord.xy;
  texCoord.x -= contextX;
  texCoord.y -= contextY;
  texCoord.x /= contextWidth;
  texCoord.y /= contextHeight;

  float depth = texture2D(map_depth, texCoord).x;

  vec3 worldPosition = GetWorldPosition(texCoord, depth);

  vec3 base = texture2D(map_albedo, texCoord).xyz;

  float brightness = 0.15f;//0.25f;

  // blueish tint + brightness
  vec3 baseDesaturated = vec3((base.r + base.g + base.b) / 3);
  base = baseDesaturated * 0.7 + base * 0.3;
  base *= vec3(0.9f, 1.0f, 1.2f) * brightness;

  // screen space ambient occlusion
  // normal-oriented hemisphere method, (c) john chapman
  // http://john-chapman-graphics.blogspot.nl/2013/01/ssao-tutorial.html

  vec2 noiseScale = vec2(contextWidth / 4.0f, contextHeight / 4.0f);
  float SSAO_radius = 0.18f;//0.12f;

  vec3 normal = texture2D(map_normal, texCoord).xyz;
  float SSAO = 0.0;

  vec3 viewPosition = vec3(viewMatrix * vec4(worldPosition, 1.0f));
  vec3 viewNormal = vec3(viewMatrix * vec4(normal, 0.0f));

  vec3 randomVec = -texture2D(map_noise, texCoord * noiseScale).xyz * 2.0f - 1.0f;
  vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
  vec3 bitangent = cross(viewNormal, tangent);
  mat3 tbn = mat3(tangent, bitangent, viewNormal);

  for (int i = 0; i < SSAO_kernelSize; ++i) {

    // get sample position in viewspace coords
    vec3 sample = tbn * SSAO_kernel[i];
    sample = viewPosition + sample * SSAO_radius;
    // sample.z darkens in the distance
    //test += sample / (SSAO_kernelSize * 1.0f);

    // sample position to projection space
    vec4 offset = projectionMatrix * vec4(sample, 1.0f);
    offset.xy /= offset.w;
    offset.xy = offset.xy * 0.5f + 0.5f;

    float sampleDepth = texture(map_depth, offset.xy).x;
    // convert from non-linear [0..1] to linear [near .. far]
    sampleDepth = cameraClip.y / (sampleDepth - cameraClip.x);

    // falloff after radius
    //float rangeCheckFalloff = abs(-viewPosition.z - sampleDepth) < SSAO_radius ? 1.0f : 0.0f;
    float rangeCheckFalloff = clamp((SSAO_radius - abs(-viewPosition.z - sampleDepth) * 0.5f) / SSAO_radius, 0.0f, 1.0f); // gradual falloff version. * 0.4f == don't fall off all too gracefully; pow would be better (but slower!)
    // if actual fragment's depth is behind sample position, do not count
    float rangeCheckInside = sampleDepth < -sample.z ? 1.0f : 0.0f;

    SSAO += rangeCheckFalloff * rangeCheckInside;
  }
  SSAO /= SSAO_kernelSize * 1.0f;
  SSAO = 1.0f - SSAO;
  //SSAO = SSAO * 2.0f - 1.0f; // exaggerate effect
  SSAO = SSAO * 1.5f - 0.5f; // exaggerate effect
  //SSAO *= SSAO; // exaggerate effect
  SSAO = clamp(SSAO, 0.0f, 1.0f);
  //SSAO = 1; //disable


  // self-illumination

  vec4 aux = texture2D(map_aux, texCoord.st);
  float self_illumination = aux.w;

  vec3 fragColor = vec3(clamp(base * (1.0 + self_illumination), 0.0, 1.0));

  //stdout0 = vec4(vec3(depth / 2.0f), 1.0);
  stdout0 = vec4(fragColor * SSAO, 1.0);
  stdout1.r = GetEdge(texCoord); // AA
  stdout1.g = SSAO;
}
