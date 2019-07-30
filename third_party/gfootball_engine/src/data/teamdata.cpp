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
// this work is public domain. the code is undocumented, scruffy, untested, and
// should generally not be used for anything important. i do not offer support,
// so don't ask. to be used for inspiration :)

#include "teamdata.hpp"

#include <boost/algorithm/string.hpp>

#include "../base/utils.hpp"
#include "../main.hpp"

Vector3 GetDefaultRolePosition(e_PlayerRole role) {
  switch (role) {
    case e_PlayerRole_GK:
      return Vector3(-1.0, 0.0, 0);
      break;

    case e_PlayerRole_CB:
      return Vector3(-1.0, 0.0, 0);
      break;
    case e_PlayerRole_LB:
      return Vector3(-0.8, 0.8, 0);
      break;
    case e_PlayerRole_RB:
      return Vector3(-0.8, -0.8, 0);
      break;

    case e_PlayerRole_DM:
      return Vector3(-0.5, 0.0, 0);
      break;
    case e_PlayerRole_CM:
      return Vector3(0.0, 0.0, 0);
      break;
    case e_PlayerRole_LM:
      return Vector3(0.0, 1.0, 0);
      break;
    case e_PlayerRole_RM:
      return Vector3(0.0, -1.0, 0);
      break;
    case e_PlayerRole_AM:
      return Vector3(0.5, 0.0, 0);
      break;

    case e_PlayerRole_CF:
      return Vector3(1.0, 0.0, 0);
      break;

    default:
      return Vector3(0.0, 0.0, 0);
      break;
  }
}

TeamData::TeamData(int teamDatabaseID, int playersTeamDatabaseID,
                   const std::vector<FormationEntry> &f)
    : databaseID(teamDatabaseID) {
  formation.resize(f.empty() ? playerNum : f.size());
  int player_count = formation.size();

  std::string formationString;
  std::string factoryFormationString;
  std::string factoryTacticsString;
  std::string tacticsString;

  color1.Set(0, 0, 0);
  color2.Set(255, 255, 255);

  bool national = false;

  switch (databaseID) {
    case 3:
      national = 0;
      name = "Frequentists United";
      logo_url = "images_teams/primeradivision/fcbarcelona_logo.png";
      kit_url = "images_teams/primeradivision/fcbarcelona";
      formationString =
          "<p1><position> -1.0,  0.0 "
          "</position><role>GK</role></p1><p2><position> -0.7,  "
          "0.75</position><role>LB</role></p2><p3><position> -1.0,  "
          "0.25</position><role>CB</role></p3><p4><position> -1.0, "
          "-0.25</position><role>CB</role></p4><p5><position> -0.7, "
          "-0.75</position><role>RB</role></p5><p6><position>  0.0,  0.5 "
          "</position><role>CM</role></p6><p7><position> -0.2,  0.0 "
          "</position><role>CM</role></p7><p8><position>  0.0, -0.5 "
          "</position><role>CM</role></p8><p9><position>  0.6,  0.75 "
          "</position><role>LM</role></p9><p10><position> 1.0,  0.0 "
          "</position><role>CF</role></p10><p11><position> 0.6, -0.75 "
          "</position><role>RM</role></p11>";
      factoryFormationString =
          "<p1><position> -1.0,  0.0 "
          "</position><role>GK</role></p1><p2><position> -0.7,  "
          "0.75</position><role>LB</role></p2><p3><position> -1.0,  "
          "0.25</position><role>CB</role></p3><p4><position> -1.0, "
          "-0.25</position><role>CB</role></p4><p5><position> -0.7, "
          "-0.75</position><role>RB</role></p5><p6><position>  0.0,  0.5 "
          "</position><role>CM</role></p6><p7><position> -0.2,  0.0 "
          "</position><role>CM</role></p7><p8><position>  0.0, -0.5 "
          "</position><role>CM</role></p8><p9><position>  0.6,  0.75 "
          "</position><role>LM</role></p9><p10><position> 1.0,  0.0 "
          "</position><role>CF</role></p10><p11><position> 0.6, -0.75 "
          "</position><role>RM</role></p11>";
      tacticsString =
          "<dribble_centermagnet>0.720000</"
          "dribble_centermagnet><dribble_offensiveness>0.500000</"
          "dribble_offensiveness><position_defense_depth_factor>0.300000</"
          "position_defense_depth_factor><position_defense_microfocus_strength>"
          "0.960000</"
          "position_defense_microfocus_strength><position_defense_"
          "midfieldfocus>0.960000</"
          "position_defense_midfieldfocus><position_defense_sidefocus_strength>"
          "0.160000</"
          "position_defense_sidefocus_strength><position_defense_width_factor>"
          "0.700000</"
          "position_defense_width_factor><position_offense_depth_factor>0."
          "340000</"
          "position_offense_depth_factor><position_offense_microfocus_strength>"
          "0.920000</"
          "position_offense_microfocus_strength><position_offense_"
          "midfieldfocus>0.880000</"
          "position_offense_midfieldfocus><position_offense_sidefocus_strength>"
          "0.880000</"
          "position_offense_sidefocus_strength><position_offense_width_factor>"
          "0.740000</position_offense_width_factor>";
      factoryTacticsString =
          "<dribble_centermagnet>0.720000</"
          "dribble_centermagnet><dribble_offensiveness>0.500000</"
          "dribble_offensiveness><position_defense_depth_factor>0.300000</"
          "position_defense_depth_factor><position_defense_microfocus_strength>"
          "0.960000</"
          "position_defense_microfocus_strength><position_defense_"
          "midfieldfocus>0.960000</"
          "position_defense_midfieldfocus><position_defense_sidefocus_strength>"
          "0.160000</"
          "position_defense_sidefocus_strength><position_defense_width_factor>"
          "0.700000</"
          "position_defense_width_factor><position_offense_depth_factor>0."
          "340000</"
          "position_offense_depth_factor><position_offense_microfocus_strength>"
          "0.920000</"
          "position_offense_microfocus_strength><position_offense_"
          "midfieldfocus>0.880000</"
          "position_offense_midfieldfocus><position_offense_sidefocus_strength>"
          "0.880000</"
          "position_offense_sidefocus_strength><position_offense_width_factor>"
          "0.740000</position_offense_width_factor>";
      shortName = "FRQ";
      color1 = Vector3(255, 100, 100);
      color2 = Vector3(100, 100, 255);
      break;
    case 8:
      national = 0;
      name = "Real Bayesians";
      logo_url = "images_teams/primeradivision/realmadrid_logo.png";
      kit_url = "images_teams/primeradivision/realmadrid";
      formationString =
          "<p1><position>-1.0,  "
          "0.0</position><role>GK</role></p1><p2><position>-0.7,  "
          "0.75</position><role>LB</role></p2><p3><position>-1.0,  "
          "0.25</position><role>CB</role></p3><p4><position>-1.0, "
          "-0.25</position><role>CB</role></p4><p5><position>-0.7, "
          "-0.75</position><role>RB</role></p5><p6><position>-0.2,  "
          "0.3</position><role>CM</role></p6><p7><position>-0.2, "
          "-0.3</position><role>CM</role></p7><p8><position> 0.7,  "
          "0.9</position><role>LM</role></p8><p9><position> 0.2,  "
          "0.0</position><role>AM</role></p9><p10><position>0.7, "
          "-0.9</position><role>RM</role></p10><p11><position>1.0,  "
          "0.0</position><role>CF</role></p11>";
      factoryFormationString =
          "<p1><position>-1.0,  "
          "0.0</position><role>GK</role></p1><p2><position>-0.7,  "
          "0.75</position><role>LB</role></p2><p3><position>-1.0,  "
          "0.25</position><role>CB</role></p3><p4><position>-1.0, "
          "-0.25</position><role>CB</role></p4><p5><position>-0.7, "
          "-0.75</position><role>RB</role></p5><p6><position>-0.2,  "
          "0.3</position><role>CM</role></p6><p7><position>-0.2, "
          "-0.3</position><role>CM</role></p7><p8><position> 0.7,  "
          "0.9</position><role>LM</role></p8><p9><position> 0.2,  "
          "0.0</position><role>AM</role></p9><p10><position>0.7, "
          "-0.9</position><role>RM</role></p10><p11><position>1.0,  "
          "0.0</position><role>CF</role></p11>";
      tacticsString =
          "<dribble_centermagnet>0.500000</"
          "dribble_centermagnet><dribble_offensiveness>0.900000</"
          "dribble_offensiveness><position_defense_depth_factor>0.180000</"
          "position_defense_depth_factor><position_defense_microfocus_strength>"
          "0.380000</"
          "position_defense_microfocus_strength><position_defense_"
          "midfieldfocus>0.660000</"
          "position_defense_midfieldfocus><position_defense_sidefocus_strength>"
          "0.760000</"
          "position_defense_sidefocus_strength><position_defense_width_factor>"
          "0.540000</"
          "position_defense_width_factor><position_offense_depth_factor>0."
          "900000</"
          "position_offense_depth_factor><position_offense_microfocus_strength>"
          "0.140000</"
          "position_offense_microfocus_strength><position_offense_"
          "midfieldfocus>0.740000</"
          "position_offense_midfieldfocus><position_offense_sidefocus_strength>"
          "0.960000</"
          "position_offense_sidefocus_strength><position_offense_width_factor>"
          "0.940000</position_offense_width_factor>";
      factoryTacticsString =
          "<dribble_centermagnet>0.500000</"
          "dribble_centermagnet><dribble_offensiveness>0.900000</"
          "dribble_offensiveness><position_defense_depth_factor>0.180000</"
          "position_defense_depth_factor><position_defense_microfocus_strength>"
          "0.380000</"
          "position_defense_microfocus_strength><position_defense_"
          "midfieldfocus>0.660000</"
          "position_defense_midfieldfocus><position_defense_sidefocus_strength>"
          "0.760000</"
          "position_defense_sidefocus_strength><position_defense_width_factor>"
          "0.540000</"
          "position_defense_width_factor><position_offense_depth_factor>0."
          "900000</"
          "position_offense_depth_factor><position_offense_microfocus_strength>"
          "0.140000</"
          "position_offense_microfocus_strength><position_offense_"
          "midfieldfocus>0.740000</"
          "position_offense_midfieldfocus><position_offense_sidefocus_strength>"
          "0.960000</"
          "position_offense_sidefocus_strength><position_offense_width_factor>"
          "0.940000</position_offense_width_factor>";
      shortName = "RBA";
      color1 = Vector3(255, 255, 255);
      color2 = Vector3(50, 50, 126);
      break;
  }

  if (shortName.compare("") == 0) {
    shortName = name;
    shortName.erase(remove_if(shortName.begin(), shortName.end(), isspace),
                    shortName.end());
    shortName = boost::to_upper_copy(shortName.substr(0, 3));
  }

  logo_url = "databases/default/" + logo_url;
  kit_url = "databases/default/" + kit_url;

  // team formation
  XMLLoader loader;
  XMLTree tree = loader.Load(formationString);
  map_XMLTree::const_iterator iter = tree.children.begin();

  while (iter != tree.children.end()) {
    for (int num = 0; num < player_count; num++) {
      if ((*iter).first == "p" + int_to_str(num + 1)) {
        formation[num].databasePosition = GetVectorFromString(
            (*iter).second.children.find("position")->second.value);
        formation[num].role = GetRoleFromString(
            (*iter).second.children.find("role")->second.value);

        // combine custom positions with hardcoded formation positions belonging
        // to certain roles. this way, more extreme user formation settings are
        // 'normalized' somewhat.
        formation[num].position =
            formation[num].databasePosition * 0.6f +
            GetDefaultRolePosition(formation[num].role) * 0.4f;

        // GetVectorFromString((*iter).second.children.find("position")->second.value).Print();
        // printf("%s\n",
        // (*iter).second.children.find("role")->second.value.c_str());
      }
    }

    iter++;
  }

  // make sure players have some personal space, don't step in each other's aura
  // ;)
  float minDistanceFraction = 0.5f;  // remember the range is 2 (-1 to 1)
  unsigned int maxIterations = 10;
  unsigned int iterations = 0;
  bool changed = true;

  while (changed && iterations < maxIterations) {
    Vector3 offset[player_count];

    changed = false;
    for (int p1 = 0; p1 < player_count - 1; p1++) {
      if (formation[p1].role == e_PlayerRole_GK) continue;

      for (int p2 = p1 + 1; p2 < player_count; p2++) {
        if (formation[p2].role == e_PlayerRole_GK) continue;

        Vector3 diff = (formation[p1].position - formation[p2].position);
        // if (diff.GetLength() < 0.1f) diff = Vector3(0, 0.1, 0);

        if (diff.GetLength() < minDistanceFraction) {
          changed = true;

          float distanceFactor =
              1.0f - (diff.GetLength() / minDistanceFraction);

          offset[p1] += diff.GetNormalized(Vector3(0, 1, 0)) *
                        minDistanceFraction * distanceFactor * 0.5f;
          offset[p2] -= diff.GetNormalized(Vector3(0, 1, 0)) *
                        minDistanceFraction * distanceFactor * 0.5f;
        }
      }
    }

    if (changed) {
      for (int p = 0; p < player_count; p++) {
        formation[p].position += offset[p];
        formation[p].position.coords[0] =
            clamp(formation[p].position.coords[0], -1, 1);
        formation[p].position.coords[1] =
            clamp(formation[p].position.coords[1], -1, 1);
      }
    }

    iterations++;
  }

  for (int x = 0; x < player_count; x++) {
    if (f.empty()) {
      formation[x].start_position = formation[x].position;
    } else {
      formation[x].start_position = f[x].start_position;
      formation[x].lazy = f[x].lazy;
      formation[x].role = f[x].role;
    }
  }

  // team tactics

  tree = loader.Load(tacticsString);

  iter = tree.children.begin();
  while (iter != tree.children.end()) {
    tactics.userProperties.Set((*iter).first,
                               atof((*iter).second.value.c_str()));
    iter++;
  }
  // factory tactics

  tree = loader.Load(factoryTacticsString);

  iter = tree.children.begin();
  while (iter != tree.children.end()) {
    tactics.factoryProperties.Set((*iter).first.c_str(),
                                  atof((*iter).second.value.c_str()));
    // printf("value name: %s, value: %f\n", (*iter).first.c_str(),
    // atof((*iter).second.value.c_str()));
    iter++;
  }

  // load players
  playerData.push_back(new PlayerData(398));
  playerData.push_back(new PlayerData(11));
  playerData.push_back(new PlayerData(254));
  playerData.push_back(new PlayerData(320));
  playerData.push_back(new PlayerData(103));
  playerData.push_back(new PlayerData(188));
  playerData.push_back(new PlayerData(74));
  playerData.push_back(new PlayerData(332));
  playerData.push_back(new PlayerData(290));
  playerData.push_back(new PlayerData(391));
  playerData.push_back(new PlayerData(264));
  playerData.resize(player_count);
}

TeamData::~TeamData() {
  for (int i = 0; i < (signed int)playerData.size(); i++) {
    delete playerData[i];
  }
}

FormationEntry TeamData::GetFormationEntry(int num) {
  assert(num >= 0 && num < formation.size());
  return formation[num];
}

void TeamData::SetFormationEntry(int num, FormationEntry entry) {
  formation[num] = entry;
}
