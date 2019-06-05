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

e_PositionName GetPositionName(const std::string &shortcut) {

  if (shortcut == "GK") return e_PositionName_GK;
  if (shortcut == "SW") return e_PositionName_SW;
  if (shortcut == "D") return e_PositionName_D;
  if (shortcut == "WB") return e_PositionName_WB;
  if (shortcut == "DM") return e_PositionName_DM;
  if (shortcut == "M") return e_PositionName_M;
  if (shortcut == "AM") return e_PositionName_AM;
  if (shortcut == "F") return e_PositionName_F;
  if (shortcut == "ST") return e_PositionName_ST;

  Log(e_FatalError, "utils.cpp", "GetPositionName", "Position '" + shortcut + "' not found!");
  return e_PositionName_M;
}

void GetWeightedPositions(const std::string &positionString, std::vector<WeightedPosition> &weightedPositions) {
  // split by comma
  std::vector<std::string> positions;
  tokenize(positionString, positions, ",");
  for (unsigned int i = 0; i < positions.size(); i++) {
    // trim leading/trailing space
    boost::algorithm::trim(positions.at(i));
    // remove L/C/R notice (after space)
    std::size_t firstspace = positions.at(i).find_first_of(" ");
    if (firstspace != std::string::npos) positions.at(i) = positions.at(i).substr(0, firstspace);

    if (positions.at(i).find_first_of("/") != std::string::npos) {
      // position mixture
      std::vector<std::string> subpositions;
      tokenize(positions.at(i), subpositions, "/");
      for (unsigned int j = 0; j < subpositions.size(); j++) {
        WeightedPosition weightedPosition;
        weightedPosition.positionName = GetPositionName(subpositions.at(j));
        weightedPosition.weight = 1.0 / subpositions.size();
        weightedPositions.push_back(weightedPosition);
      }
    } else {
      // just one position
      WeightedPosition weightedPosition;
      weightedPosition.positionName = GetPositionName(positions.at(i));
      weightedPosition.weight = 1.0;
      weightedPositions.push_back(weightedPosition);
    }
  }

  /*
  for (unsigned int i = 0; i < weightedPositions.size(); i++) {
    printf("[%i:%f]", weightedPositions.at(i).positionName, weightedPositions.at(i).weight);
  }
  printf("\n");
  */
}

void InitDefaultProfiles() {
  // std::map < e_PositionName, std::vector<Stat> > defaultProfiles;



  std::vector<Stat> statsGK;
  statsGK.push_back((Stat){"physical_balance", 0.6});
  statsGK.push_back((Stat){"physical_reaction", 0.8});
  statsGK.push_back((Stat){"physical_acceleration", 0.5});
  statsGK.push_back((Stat){"physical_velocity", 0.4});
  statsGK.push_back((Stat){"physical_stamina", 0.5});
  statsGK.push_back((Stat){"physical_agility", 0.6});
  statsGK.push_back((Stat){"physical_shotpower", 0.6});
  statsGK.push_back((Stat){"technical_standingtackle", 0.2});
  statsGK.push_back((Stat){"technical_slidingtackle", 0.2});
  statsGK.push_back((Stat){"technical_ballcontrol", 0.3});
  statsGK.push_back((Stat){"technical_dribble", 0.2});
  statsGK.push_back((Stat){"technical_shortpass", 0.6});
  statsGK.push_back((Stat){"technical_highpass", 0.7});
  statsGK.push_back((Stat){"technical_header", 0.2});
  statsGK.push_back((Stat){"technical_shot", 0.3});
  statsGK.push_back((Stat){"technical_volley", 0.2});
  statsGK.push_back((Stat){"mental_calmness", 0.5});
  statsGK.push_back((Stat){"mental_workrate", 0.5});
  statsGK.push_back((Stat){"mental_resilience", 0.5});
  statsGK.push_back((Stat){"mental_defensivepositioning", 0.9});
  statsGK.push_back((Stat){"mental_offensivepositioning", 0.1});
  statsGK.push_back((Stat){"mental_vision", 0.5});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_GK, statsGK));

  std::vector<Stat> statsSW;
  statsSW.push_back((Stat){"physical_balance", 0.7});
  statsSW.push_back((Stat){"physical_reaction", 0.7});
  statsSW.push_back((Stat){"physical_acceleration", 0.5});
  statsSW.push_back((Stat){"physical_velocity", 0.5});
  statsSW.push_back((Stat){"physical_stamina", 0.5});
  statsSW.push_back((Stat){"physical_agility", 0.4});
  statsSW.push_back((Stat){"physical_shotpower", 0.7});
  statsSW.push_back((Stat){"technical_standingtackle", 0.8});
  statsSW.push_back((Stat){"technical_slidingtackle", 0.8});
  statsSW.push_back((Stat){"technical_ballcontrol", 0.3});
  statsSW.push_back((Stat){"technical_dribble", 0.2});
  statsSW.push_back((Stat){"technical_shortpass", 0.4});
  statsSW.push_back((Stat){"technical_highpass", 0.4});
  statsSW.push_back((Stat){"technical_header", 0.6});
  statsSW.push_back((Stat){"technical_shot", 0.2});
  statsSW.push_back((Stat){"technical_volley", 0.2});
  statsSW.push_back((Stat){"mental_calmness", 0.5});
  statsSW.push_back((Stat){"mental_workrate", 0.5});
  statsSW.push_back((Stat){"mental_resilience", 0.5});
  statsSW.push_back((Stat){"mental_defensivepositioning", 0.8});
  statsSW.push_back((Stat){"mental_offensivepositioning", 0.2});
  statsSW.push_back((Stat){"mental_vision", 0.4});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_SW, statsSW));

  std::vector<Stat> statsD;
  statsD.push_back((Stat){"physical_balance", 0.8});
  statsD.push_back((Stat){"physical_reaction", 0.7});
  statsD.push_back((Stat){"physical_acceleration", 0.5});
  statsD.push_back((Stat){"physical_velocity", 0.5});
  statsD.push_back((Stat){"physical_stamina", 0.5});
  statsD.push_back((Stat){"physical_agility", 0.5});
  statsD.push_back((Stat){"physical_shotpower", 0.6});
  statsD.push_back((Stat){"technical_standingtackle", 0.8});
  statsD.push_back((Stat){"technical_slidingtackle", 0.8});
  statsD.push_back((Stat){"technical_ballcontrol", 0.5});
  statsD.push_back((Stat){"technical_dribble", 0.3});
  statsD.push_back((Stat){"technical_shortpass", 0.5});
  statsD.push_back((Stat){"technical_highpass", 0.5});
  statsD.push_back((Stat){"technical_header", 0.6});
  statsD.push_back((Stat){"technical_shot", 0.3});
  statsD.push_back((Stat){"technical_volley", 0.2});
  statsD.push_back((Stat){"mental_calmness", 0.5});
  statsD.push_back((Stat){"mental_workrate", 0.5});
  statsD.push_back((Stat){"mental_resilience", 0.5});
  statsD.push_back((Stat){"mental_defensivepositioning", 0.8});
  statsD.push_back((Stat){"mental_offensivepositioning", 0.3});
  statsD.push_back((Stat){"mental_vision", 0.6});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_D, statsD));

  std::vector<Stat> statsWB;
  statsWB.push_back((Stat){"physical_balance", 0.6});
  statsWB.push_back((Stat){"physical_reaction", 0.5});
  statsWB.push_back((Stat){"physical_acceleration", 0.6});
  statsWB.push_back((Stat){"physical_velocity", 0.6});
  statsWB.push_back((Stat){"physical_stamina", 0.5});
  statsWB.push_back((Stat){"physical_agility", 0.5});
  statsWB.push_back((Stat){"physical_shotpower", 0.5});
  statsWB.push_back((Stat){"technical_standingtackle", 0.7});
  statsWB.push_back((Stat){"technical_slidingtackle", 0.7});
  statsWB.push_back((Stat){"technical_ballcontrol", 0.5});
  statsWB.push_back((Stat){"technical_dribble", 0.5});
  statsWB.push_back((Stat){"technical_shortpass", 0.7});
  statsWB.push_back((Stat){"technical_highpass", 0.7});
  statsWB.push_back((Stat){"technical_header", 0.5});
  statsWB.push_back((Stat){"technical_shot", 0.4});
  statsWB.push_back((Stat){"technical_volley", 0.3});
  statsWB.push_back((Stat){"mental_calmness", 0.5});
  statsWB.push_back((Stat){"mental_workrate", 0.5});
  statsWB.push_back((Stat){"mental_resilience", 0.5});
  statsWB.push_back((Stat){"mental_defensivepositioning", 0.6});
  statsWB.push_back((Stat){"mental_offensivepositioning", 0.4});
  statsWB.push_back((Stat){"mental_vision", 0.6});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_WB, statsWB));

  std::vector<Stat> statsDM;
  statsDM.push_back((Stat){"physical_balance", 0.7});
  statsDM.push_back((Stat){"physical_reaction", 0.6});
  statsDM.push_back((Stat){"physical_acceleration", 0.5});
  statsDM.push_back((Stat){"physical_velocity", 0.5});
  statsDM.push_back((Stat){"physical_stamina", 0.5});
  statsDM.push_back((Stat){"physical_agility", 0.5});
  statsDM.push_back((Stat){"physical_shotpower", 0.6});
  statsDM.push_back((Stat){"technical_standingtackle", 0.7});
  statsDM.push_back((Stat){"technical_slidingtackle", 0.6});
  statsDM.push_back((Stat){"technical_ballcontrol", 0.5});
  statsDM.push_back((Stat){"technical_dribble", 0.4});
  statsDM.push_back((Stat){"technical_shortpass", 0.7});
  statsDM.push_back((Stat){"technical_highpass", 0.7});
  statsDM.push_back((Stat){"technical_header", 0.5});
  statsDM.push_back((Stat){"technical_shot", 0.4});
  statsDM.push_back((Stat){"technical_volley", 0.3});
  statsDM.push_back((Stat){"mental_calmness", 0.5});
  statsDM.push_back((Stat){"mental_workrate", 0.5});
  statsDM.push_back((Stat){"mental_resilience", 0.5});
  statsDM.push_back((Stat){"mental_defensivepositioning", 0.6});
  statsDM.push_back((Stat){"mental_offensivepositioning", 0.4});
  statsDM.push_back((Stat){"mental_vision", 0.6});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_DM, statsDM));

  std::vector<Stat> statsM;
  statsM.push_back((Stat){"physical_balance", 0.4});
  statsM.push_back((Stat){"physical_reaction", 0.5});
  statsM.push_back((Stat){"physical_acceleration", 0.6});
  statsM.push_back((Stat){"physical_velocity", 0.6});
  statsM.push_back((Stat){"physical_stamina", 0.5});
  statsM.push_back((Stat){"physical_agility", 0.6});
  statsM.push_back((Stat){"physical_shotpower", 0.6});
  statsM.push_back((Stat){"technical_standingtackle", 0.4});
  statsM.push_back((Stat){"technical_slidingtackle", 0.3});
  statsM.push_back((Stat){"technical_ballcontrol", 0.7});
  statsM.push_back((Stat){"technical_dribble", 0.6});
  statsM.push_back((Stat){"technical_shortpass", 0.8});
  statsM.push_back((Stat){"technical_highpass", 0.7});
  statsM.push_back((Stat){"technical_header", 0.4});
  statsM.push_back((Stat){"technical_shot", 0.5});
  statsM.push_back((Stat){"technical_volley", 0.4});
  statsM.push_back((Stat){"mental_calmness", 0.5});
  statsM.push_back((Stat){"mental_workrate", 0.5});
  statsM.push_back((Stat){"mental_resilience", 0.5});
  statsM.push_back((Stat){"mental_defensivepositioning", 0.5});
  statsM.push_back((Stat){"mental_offensivepositioning", 0.5});
  statsM.push_back((Stat){"mental_vision", 0.7});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_M, statsM));

  std::vector<Stat> statsAM;
  statsAM.push_back((Stat){"physical_balance", 0.4});
  statsAM.push_back((Stat){"physical_reaction", 0.5});
  statsAM.push_back((Stat){"physical_acceleration", 0.6});
  statsAM.push_back((Stat){"physical_velocity", 0.6});
  statsAM.push_back((Stat){"physical_stamina", 0.5});
  statsAM.push_back((Stat){"physical_agility", 0.7});
  statsAM.push_back((Stat){"physical_shotpower", 0.7});
  statsAM.push_back((Stat){"technical_standingtackle", 0.2});
  statsAM.push_back((Stat){"technical_slidingtackle", 0.2});
  statsAM.push_back((Stat){"technical_ballcontrol", 0.6});
  statsAM.push_back((Stat){"technical_dribble", 0.7});
  statsAM.push_back((Stat){"technical_shortpass", 0.6});
  statsAM.push_back((Stat){"technical_highpass", 0.5});
  statsAM.push_back((Stat){"technical_header", 0.4});
  statsAM.push_back((Stat){"technical_shot", 0.7});
  statsAM.push_back((Stat){"technical_volley", 0.5});
  statsAM.push_back((Stat){"mental_calmness", 0.5});
  statsAM.push_back((Stat){"mental_workrate", 0.5});
  statsAM.push_back((Stat){"mental_resilience", 0.5});
  statsAM.push_back((Stat){"mental_defensivepositioning", 0.3});
  statsAM.push_back((Stat){"mental_offensivepositioning", 0.7});
  statsAM.push_back((Stat){"mental_vision", 0.4});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_AM, statsAM));

  std::vector<Stat> statsF;
  statsF.push_back((Stat){"physical_balance", 0.4});
  statsF.push_back((Stat){"physical_reaction", 0.5});
  statsF.push_back((Stat){"physical_acceleration", 0.7});
  statsF.push_back((Stat){"physical_velocity", 0.7});
  statsF.push_back((Stat){"physical_stamina", 0.5});
  statsF.push_back((Stat){"physical_agility", 0.7});
  statsF.push_back((Stat){"physical_shotpower", 0.7});
  statsF.push_back((Stat){"technical_standingtackle", 0.2});
  statsF.push_back((Stat){"technical_slidingtackle", 0.1});
  statsF.push_back((Stat){"technical_ballcontrol", 0.5});
  statsF.push_back((Stat){"technical_dribble", 0.8});
  statsF.push_back((Stat){"technical_shortpass", 0.6});
  statsF.push_back((Stat){"technical_highpass", 0.6});
  statsF.push_back((Stat){"technical_header", 0.4});
  statsF.push_back((Stat){"technical_shot", 0.7});
  statsF.push_back((Stat){"technical_volley", 0.4});
  statsF.push_back((Stat){"mental_calmness", 0.5});
  statsF.push_back((Stat){"mental_workrate", 0.5});
  statsF.push_back((Stat){"mental_resilience", 0.5});
  statsF.push_back((Stat){"mental_defensivepositioning", 0.25});
  statsF.push_back((Stat){"mental_offensivepositioning", 0.75});
  statsF.push_back((Stat){"mental_vision", 0.5});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_F, statsF));

  std::vector<Stat> statsST;
  statsST.push_back((Stat){"physical_balance", 0.3});
  statsST.push_back((Stat){"physical_reaction", 0.6});
  statsST.push_back((Stat){"physical_acceleration", 0.6});
  statsST.push_back((Stat){"physical_velocity", 0.6});
  statsST.push_back((Stat){"physical_stamina", 0.5});
  statsST.push_back((Stat){"physical_agility", 0.7});
  statsST.push_back((Stat){"physical_shotpower", 0.8});
  statsST.push_back((Stat){"technical_standingtackle", 0.1});
  statsST.push_back((Stat){"technical_slidingtackle", 0.1});
  statsST.push_back((Stat){"technical_ballcontrol", 0.5});
  statsST.push_back((Stat){"technical_dribble", 0.7});
  statsST.push_back((Stat){"technical_shortpass", 0.5});
  statsST.push_back((Stat){"technical_highpass", 0.4});
  statsST.push_back((Stat){"technical_header", 0.6});
  statsST.push_back((Stat){"technical_shot", 0.8});
  statsST.push_back((Stat){"technical_volley", 0.7});
  statsST.push_back((Stat){"mental_calmness", 0.5});
  statsST.push_back((Stat){"mental_workrate", 0.5});
  statsST.push_back((Stat){"mental_resilience", 0.5});
  statsST.push_back((Stat){"mental_defensivepositioning", 0.2});
  statsST.push_back((Stat){"mental_offensivepositioning", 0.8});
  statsST.push_back((Stat){"mental_vision", 0.3});
  defaultProfiles.insert(std::pair< e_PositionName, std::vector<Stat> >(e_PositionName_ST, statsST));


  // give all positions the same stats on average

  std::map < e_PositionName, std::vector<Stat> > ::iterator iter = defaultProfiles.begin();
  while (iter != defaultProfiles.end()) {
    std::vector<Stat> &stats = iter->second;
    float totalStats = 0.0f;
    int statsAmount = stats.size();
    for (unsigned int j = 0; j < stats.size(); j++) {
      totalStats += stats.at(j).value;
    }
    float averageStat = totalStats / (float)statsAmount;
    // average should be 0.5...
    for (unsigned int j = 0; j < stats.size(); j++) {
      stats.at(j).value += (0.5f - averageStat);
      assert(stats.at(j).value > 0.0f && stats.at(j).value < 1.0f);
      // center around 1.0.. (0.5 .. 1.5)
      //stats.at(j).value += 0.5f;
    }
    iter++;
  }

  // defenders are underrated..
/* edit: THIS SHOULD BE CALCULATED IN AVERAGESTARTSTAT NOT IN PROFILE YOU N00B!!
  iter = defaultProfiles.begin();
  float pos = 0.0;
  while (iter != defaultProfiles.end()) {
    std::vector<Stat> &stats = iter->second;
    for (unsigned int j = 0; j < stats.size(); j++) {
      stats.at(j).value *= 1.0f + pow(pos, 1.4f) * 0.2f; // defenders are low in the map, since it's sorted by enum '0 .. 8 value'
      assert(stats.at(j).value > 0.0f && stats.at(j).value < 1.0f);
    }
    iter++;
    pos += 0.1; // 9 entries total
  }
*/
}

void GetDefaultProfile(const std::vector<WeightedPosition> &weightedPositions, std::vector<Stat> &averageProfile) {

  // total weight
  float totalWeight = 0.0f;
  for (unsigned int i = 0; i < weightedPositions.size(); i++) {
    //printf("** adding weight %f\n", weightedPositions.at(i).weight);
    totalWeight += weightedPositions.at(i).weight;
  }
  //printf("** done adding %i weights\n", weightedPositions.size());

  averageProfile.push_back((Stat){"physical_balance", 0.0});
  averageProfile.push_back((Stat){"physical_reaction", 0.0});
  averageProfile.push_back((Stat){"physical_acceleration", 0.0});
  averageProfile.push_back((Stat){"physical_velocity", 0.0});
  averageProfile.push_back((Stat){"physical_stamina", 0.0});
  averageProfile.push_back((Stat){"physical_agility", 0.0});
  averageProfile.push_back((Stat){"physical_shotpower", 0.0});
  averageProfile.push_back((Stat){"technical_standingtackle", 0.0});
  averageProfile.push_back((Stat){"technical_slidingtackle", 0.0});
  averageProfile.push_back((Stat){"technical_ballcontrol", 0.0});
  averageProfile.push_back((Stat){"technical_dribble", 0.0});
  averageProfile.push_back((Stat){"technical_shortpass", 0.0});
  averageProfile.push_back((Stat){"technical_highpass", 0.0});
  averageProfile.push_back((Stat){"technical_header", 0.0});
  averageProfile.push_back((Stat){"technical_shot", 0.0});
  averageProfile.push_back((Stat){"technical_volley", 0.0});
  averageProfile.push_back((Stat){"mental_calmness", 0.0});
  averageProfile.push_back((Stat){"mental_workrate", 0.0});
  averageProfile.push_back((Stat){"mental_resilience", 0.0});
  averageProfile.push_back((Stat){"mental_defensivepositioning", 0.0});
  averageProfile.push_back((Stat){"mental_offensivepositioning", 0.0});
  averageProfile.push_back((Stat){"mental_vision", 0.0});

  for (unsigned int i = 0; i < weightedPositions.size(); i++) {
    std::map < e_PositionName, std::vector<Stat> > ::iterator iter = defaultProfiles.find(weightedPositions.at(i).positionName);
    assert(iter != defaultProfiles.end());
    const std::vector<Stat> &stats = iter->second;
    for (unsigned int j = 0; j < stats.size(); j++) {
      //printf("|| %f, %f, %f\n", stats.at(j).value, weightedPositions.at(i).weight, totalWeight);
      averageProfile.at(j).value += stats.at(j).value * (weightedPositions.at(i).weight / totalWeight);
    }
  }

  /* verbose
  for (unsigned int i = 0; i < averageStats.size(); i++) {
    printf("%s: %f\n", averageStats.at(i).name.c_str(), averageStats.at(i).value);
  }
  */

}

std::string GetProfileString(const std::vector<Stat> &profileStats) {
  std::string result;
  for (unsigned int i = 0; i < profileStats.size(); i++) {
    result.append("<" + profileStats.at(i).name + ">" + real_to_str(profileStats.at(i).value) + "</" + profileStats.at(i).name + ">\n");
  }
  return result;
}

/* deprecated
float GetAverageStatFromValue(int age, int value) {
  // right now: overall lowest/highest value: 10/45643480

  float currentStat = clamp((float)value / 46000000.0f, 0.000001f, 0.99f);

  // player value works exponentially.. rich clubs are willing to pay lots extra for that little bit more skill
  currentStat = pow(currentStat, 0.2f);
  //printf("%i, %f\n", value, currentStat);

  // young players are 'overpriced', as in, their price is higher than the corresponding stat, since 'potential' is considered in the price
  // same for old players, the other way around. 'curve' this right
  int base = 15; // youngest (40 is oldest)
  float exaggerate = 1.5f;
  float valueToStatMultiplier[26];

  valueToStatMultiplier[15 - base] = 0.64;
  valueToStatMultiplier[16 - base] = 0.66;
  valueToStatMultiplier[17 - base] = 0.68;
  valueToStatMultiplier[18 - base] = 0.70;
  valueToStatMultiplier[19 - base] = 0.73;
  valueToStatMultiplier[20 - base] = 0.76;
  valueToStatMultiplier[21 - base] = 0.79;
  valueToStatMultiplier[22 - base] = 0.82;
  valueToStatMultiplier[23 - base] = 0.85;
  valueToStatMultiplier[24 - base] = 0.88;
  valueToStatMultiplier[25 - base] = 0.91;
  valueToStatMultiplier[26 - base] = 0.94;
  valueToStatMultiplier[27 - base] = 0.97;
  valueToStatMultiplier[28 - base] = 1.00; // toppling point - at this age the 'value' graph cuts the 'stats' graph
  valueToStatMultiplier[29 - base] = 1.04;
  valueToStatMultiplier[30 - base] = 1.08;
  valueToStatMultiplier[31 - base] = 1.12;
  valueToStatMultiplier[32 - base] = 1.16;
  valueToStatMultiplier[33 - base] = 1.20;
  valueToStatMultiplier[34 - base] = 1.24;
  valueToStatMultiplier[35 - base] = 1.28;
  valueToStatMultiplier[36 - base] = 1.32;
  valueToStatMultiplier[37 - base] = 1.35;
  valueToStatMultiplier[38 - base] = 1.38;
  valueToStatMultiplier[39 - base] = 1.41;
  valueToStatMultiplier[40 - base] = 1.43;
  for (int i = 0; i <= 40 - base; i++) {
    valueToStatMultiplier[i] -= 1.0;
    valueToStatMultiplier[i] *= exaggerate;
    valueToStatMultiplier[i] += 1.0;

    //valueToStatMultiplier[i] -= 0.9;
  }
  currentStat *= valueToStatMultiplier[age - base];

  // even bad players have SOME skills.
  //XXcurrentStat = 0.4f + currentStat * 0.6f;
  //currentStat = 0.1f + currentStat * 0.9f;

  //currentStat = clamp(currentStat, 0.01, 0.99);

  return currentStat;
}
*/

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

/*
float GetIndividualStat(float averageStat, float profileStat, float age) {


  //float stat = averageStat * profileStat;

  //float ageSkill = 1.0 - fabs( 1.0 - ((age - 15) / (28 - 15)) ); // flip twice (1.0 - ..) to be able to use fabs for changing direction @ peak age
  //float ageFactor = clamp(ageSkill, 0.0, 1.0);

  float ageFactor = clamp( float(age - 15) / float(28 - 15) , 0.0, 1.0);

  // some test thing, i guess
  // //float result = stat / (stat + 0.3f) * 1.3f;
  // float result = tanh(2.4f * stat) * 1.06f; // hyperbolic tangent
  // //float result = stat * 1.6f;

  //float agedProfileStat = tanh(2.4f * profileStat * ageFactor) * 1.06f; // hyperbolic tangent

  // even out specific skills with age (keep average around 0.5)
  ageFactor *= 0.6f;
  float agedProfileStat = (profileStat * (1.0 - ageFactor)) + (0.5 * ageFactor);

  float result = agedProfileStat;
  //float result = averageStat * agedProfileStat * 1.8f;// <-- why was I doing that? should have commented it

  return clamp(result, 0.00000001, 0.99999999);
}

float GetAverageStatFromBaseStat(float baseStat, int age, e_DevelopmentCurveType developmentCurveType) {


  float ageFactor = pow(clamp( float(age - 15) / float(28 - 15) , 0.0, 1.0), 0.5);
  return pow(baseStat, 1.0 - ageFactor * 0.9f);

  float idealAge = 27;
  float ageFactor = curve( 1.0f - NormalizedClamp(fabs(age - idealAge), 0, 13) * 0.6f , 1.0f);

  return pow(baseStat, ageFactor);
}
*/

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

  bool debug = false;//(random(0.0f, 1.0f) > 0.99f);

  int t1index = -1;
  int t2index = -1;
  if (debug) printf("-- start --\n");
  for (unsigned int i = 0; i < values.size(); i++) {
    if (debug) printf("iteration %i\n", i);
    const TemporalValue<T> *iter = &values[i];
    if (debug) printf("%lu vs. %lu ?\n", (*iter).time_ms, targetTime_ms);
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
  if (debug) printf("-- end --\n");

  // if (targetTime_ms < value1.time_ms) printf("<");
  // else if (targetTime_ms > value2.time_ms) printf(">");
  // else printf("_");

  if (debug) printf("current: %lu, target: %lu, t1: %lu, t2: %lu, t1index/t2index/size: %i/%i/%i\n", currentTime_ms, targetTime_ms, value1.time_ms, value2.time_ms, t1index, t2index, values.size());
  // if (value1.time_ms == 0) printf("1");
  // if (value2.time_ms == 0) printf("2");
  if (value1.time_ms == 0) return value2.data;
  if (value2.time_ms == 0) return value1.data;

  float bias = NormalizedClamp(targetTime_ms, value1.time_ms, std::max(value2.time_ms, value1.time_ms + 1));
  if (debug) printf("bias: %f\n", bias);
  //unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
  //int diff = value2.time_ms - value1.time_ms;
  //printf("%i to %i (%i)\n", (int)(value1.time_ms - time_ms), (int)(value2.time_ms - time_ms), diff);
  //if (random(0.0f, 1.0f) > 0.8f) printf("bias: %f\n", bias);
  //if (random(0.0f, 1.0f) > 0.8f) printf("ago: %li\n", now_ms - values.at(values.size() - 1).time_ms);

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
