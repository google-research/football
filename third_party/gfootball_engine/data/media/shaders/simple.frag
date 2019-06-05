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

uniform sampler2D map_albedo;
uniform sampler2D map_normal;
uniform sampler2D map_specular;
uniform sampler2D map_illumination;

uniform vec3 materialparams;
uniform vec3 materialbools;

// all these are in eye-space
in vec4 frag_position;
in vec3 frag_normal;
in vec3 frag_texcoord;
in vec3 frag_tangent;
in vec3 frag_bitangent;

out vec4 stdout0;
out vec4 stdout1;
out vec4 stdout2;

void main(void) {

  vec4 base = texture2D(map_albedo, frag_texcoord.st);
  if (base.a < 0.12) discard;
  vec3 bump;
  if (materialbools.x == 1.0f) {
    bump = normalize(texture2D(map_normal, frag_texcoord.st).xyz * 2.0 - 1.0);
  } else {
    bump = vec3(0, 0, 1);
  }
  float spec;
  if (materialbools.y == 1.0f) {
    spec = texture2D(map_specular, frag_texcoord.st).x * materialparams.y;
  } else {
    spec = materialparams.y;
  }
  float illumination;
  if (materialbools.z == 1.0f) {
    illumination = texture2D(map_illumination, frag_texcoord.st).x;
  } else {
    illumination = materialparams.z;
  }




  // recently disabled vec3 n = normalize(frag_normal);
  vec3 bumpNormal = bump.x * frag_tangent + bump.y * frag_bitangent + bump.z * frag_normal;//n;

  // partial derivative
  // bumpNormal = normalize(vec3(n.xy * bn.z + bn.xy * n.z, n.z * bn.z));

  // white-out blending
  //bumpNormal = normalize(vec3(n.xy + bn.xy, n.z * bn.z));

  //bn = bn.x * frag_tangent + bn.y * frag_bitangent + bn.z * vec3(0, 0, 1);

  bumpNormal = normalize(bumpNormal);

  stdout0 = vec4(base.rgb, spec);
  stdout1 = vec4(bumpNormal.xyz, materialparams.x);
  //gl_FragData[2] = vec4(frag_position.xyz, illumination); // *fixed* position data is needed for the ssao ambient shader. maybe a bit inefficient? in the other shaders, position is calculated on the fly
  stdout2 = vec4(0, 0, 0, illumination); // free channels! for future use, methinks :)
}
