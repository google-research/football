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

#include "playerdata.hpp"

#include <cmath>

#include "../utils/database.hpp"

#include "../base/utils.hpp"

#include "../main.hpp"

PlayerData::PlayerData(int playerDatabaseID) : databaseID(playerDatabaseID) {
  DatabaseResult *result = GetDB()->Query("select firstname, lastname, base_stat, profile_xml, age, skincolor, hairstyle, haircolor, height from players where id = " + int_to_str(databaseID) + " limit 1");

  std::string profileString;
  float baseStat = 0.0f;
  int age = 15;

  skinColor = int(std::round(random(1, 4)));
  hairStyle = "short01";
  hairColor = "darkblonde";
  height = 1.8f;

  for (unsigned int c = 0; c < result->data.at(0).size(); c++) {
    if (result->header.at(c).compare("firstname") == 0) firstName = result->data.at(0).at(c);
    if (result->header.at(c).compare("lastname") == 0) lastName = result->data.at(0).at(c);
    if (result->header.at(c).compare("base_stat") == 0) baseStat = atof(result->data.at(0).at(c).c_str());
    if (result->header.at(c).compare("profile_xml") == 0) profileString = result->data.at(0).at(c);
    if (result->header.at(c).compare("age") == 0) age = atoi(result->data.at(0).at(c).c_str());
    if (result->header.at(c).compare("skincolor") == 0) skinColor = atoi(result->data.at(0).at(c).c_str());
    if (result->header.at(c).compare("hairstyle") == 0) hairStyle = result->data.at(0).at(c);
    if (result->header.at(c).compare("haircolor") == 0) hairColor = result->data.at(0).at(c);
    if (result->header.at(c).compare("height") == 0) height = atof(result->data.at(0).at(c).c_str());
  }

  delete result;

  // get average stat for current age

  XMLLoader loader;
  XMLTree tree = loader.Load(profileString);

  //printf("player: %s, %s (age %i)\n", lastName.c_str(), firstName.c_str(), age);
  map_XMLTree::const_iterator iter = tree.children.begin();
  while (iter != tree.children.end()) {
    float profileStat = atof((*iter).second.value.c_str()); // profile value

    float value = CalculateStat(baseStat, profileStat, age, e_DevelopmentCurveType_Normal);
    //printf("base: %f; profile: %f; result: %f\n", baseStat, profileStat, value);

    stats.Set((*iter).first.c_str(), value);
    iter++;
  }
  UpdateValues();
}

PlayerData::PlayerData() {
  // officials, for example, use this constructor
  skinColor = int(std::round(random(1, 4)));
  hairStyle = "short01";
  hairColor = "darkblonde";
  height = 1.8f;

  stats.Set("physical_balance", 0.6);
  stats.Set("physical_reaction", 0.6);
  stats.Set("physical_acceleration", 0.6);
  stats.Set("physical_velocity", 0.6);
  stats.Set("physical_stamina", 0.6);
  stats.Set("physical_agility", 0.6);
  stats.Set("physical_shotpower", 0.6);
  stats.Set("technical_standingtackle", 0.6);
  stats.Set("technical_slidingtackle", 0.6);
  stats.Set("technical_ballcontrol", 0.6);
  stats.Set("technical_dribble", 0.6);
  stats.Set("technical_shortpass", 0.6);
  stats.Set("technical_highpass", 0.6);
  stats.Set("technical_header", 0.6);
  stats.Set("technical_shot", 0.6);
  stats.Set("technical_volley", 0.6);
  stats.Set("mental_calmness", 0.6);
  stats.Set("mental_workrate", 0.6);
  stats.Set("mental_resilience", 0.6);
  stats.Set("mental_defensivepositioning", 0.6);
  stats.Set("mental_offensivepositioning", 0.6);
  stats.Set("mental_vision", 0.6);
  UpdateValues();
}

void PlayerData::UpdateValues() {
  physical_velocity = GetStat("physical_velocity");
}


PlayerData::~PlayerData() {
}

float PlayerData::GetStat(const char *name) const {
  bool exists = stats.Exists(name);
  if (!exists) printf("Stat named '%s' does not exist!\n", name);
  assert(exists);
  return stats.GetReal(name, 1.0f);
}
