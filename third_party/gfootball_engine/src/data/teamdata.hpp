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

#ifndef _HPP_TEAMDATA
#define _HPP_TEAMDATA

#include "../defines.hpp"
#include "../base/properties.hpp"

#include "../gamedefines.hpp"
#include "playerdata.hpp"

struct TeamTactics {

  TeamTactics() {
  }

  Properties factoryProperties;
  Properties userProperties;

  Properties humanReadableNames;
  Properties descriptions;

};

class TeamData {

  public:
    TeamData(int teamDatabaseID, int playersTeamDatabaseID,
             const std::vector<FormationEntry>& f);
    virtual ~TeamData();

    std::string GetName() { return name; }
    std::string GetShortName() { return shortName; }
    std::string GetLogoUrl() { return logo_url; }
    std::string GetKitUrl() { return kit_url; }
    Vector3 GetColor1() { return color1; }
    Vector3 GetColor2() { return color2; }

    const TeamTactics &GetTactics() const { return tactics; }
    TeamTactics &GetTacticsWritable() { return tactics; }

    FormationEntry GetFormationEntry(int num);
    void SetFormationEntry(int num, FormationEntry entry);

    // vector index# is entry in formation[index#]
    int GetPlayerNum() { return playerData.size(); }
    PlayerData *GetPlayerData(int num) { return playerData.at(num); }

   protected:
    int databaseID = 0;

    std::string name;
    std::string shortName;
    std::string logo_url;
    std::string kit_url;
    Vector3 color1, color2;

    TeamTactics tactics;

    std::vector<FormationEntry> formation;
    std::vector<PlayerData*> playerData;

};

#endif
