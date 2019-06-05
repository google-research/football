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

#ifndef _HPP_PLAYERDATA
#define _HPP_PLAYERDATA

#include "../defines.hpp"

#include "../gamedefines.hpp"
#include "../utils.hpp"

#include "../base/properties.hpp"

class PlayerData {

  public:
    PlayerData(int playerDatabaseID);
    PlayerData();
    virtual ~PlayerData();

    void UpdateName(const std::string& first, const std::string& last) {
      firstName = first;
      lastName = last;
    }
    std::string GetFirstName() const { return firstName; }
    std::string GetLastName() const { return lastName; }
    int GetDatabaseID() const { return databaseID; }
    float GetStat(const char *name)  const;
    float get_physical_velocity() const { return physical_velocity; }

    int GetSkinColor() const { return skinColor; }

    std::string GetHairStyle() const { return hairStyle; }
    void SetHairStyle(const std::string& style) { hairStyle = style; }

    std::string GetHairColor() const { return hairColor; }
    float GetHeight() const { return height; }

    // Player mesh body model. Possible values: 0 or 1.
    int GetModelId() const { return model_id; }
    void SetModelId(const int id) { model_id = id; }

  private:
    void UpdateValues();
    float physical_velocity = 0.0;
  protected:
    int databaseID = 0;
    Properties stats;

    int skinColor = 0;
    std::string hairStyle;
    std::string hairColor;
    float height = 0.0f;
    std::string firstName;
    std::string lastName;
    int model_id = 0;

};

#endif
