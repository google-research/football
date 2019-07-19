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

#include "matchdata.hpp"
#include "../main.hpp"
#include <vector>
#include <algorithm>

class PlayerDesc {
 public:
  PlayerDesc(const std::string& firstName, const std::string& lastName, int value)
      : firstName_(firstName), lastName_(lastName), value_(value) {}
  
  std::string getFirstName() { return firstName_; }
  std::string getLastName() { return lastName_; }
  int getValue() { return value_; }

 private:
  std::string firstName_;
  std::string lastName_;
  int value_;
};

MatchData::MatchData(int team1DatabaseID, int team2DatabaseID) {
  auto& scenario = GetScenarioConfig();
  teamData[0] = new TeamData(team1DatabaseID, team1DatabaseID,
                             scenario.left_team);
  teamData[1] = new TeamData(team2DatabaseID, scenario.symmetrical_teams
      ? team1DatabaseID : team2DatabaseID, scenario.right_team);

  std::vector<PlayerDesc> names = {
      PlayerDesc("Ada", "Lovelace", 1),
      PlayerDesc("Alan", "Turing", 0),
      PlayerDesc("Albert", "Einstein", 0),
      PlayerDesc("Lisa", "Meitner", 1),
      PlayerDesc("Gerty", "Radnitz Cori", 1),
      PlayerDesc("Grace", "Hopper", 1),
      PlayerDesc("Leonardo", "da Vinci", 0),
      PlayerDesc("Maria", "Mayer", 1),
      PlayerDesc("Irene", "Joliot-Curie", 1),
      PlayerDesc("Archimedinho", "Archimedinho", 0),
      PlayerDesc("Anita", "Borg", 1),
      PlayerDesc("Isaac", "Newton", 0),
      PlayerDesc("Stefan", "Banach", 0),
      PlayerDesc("Jane", "Goodall", 1),
      PlayerDesc("Leonhard", "Euler", 0),
      PlayerDesc("Marie", "Curie", 1),
      PlayerDesc("Nicolaus", "Copernicus", 0),
      PlayerDesc("Pythagoras", "Pythagoras", 0),
      PlayerDesc("Richard", "Feynman", 0),
      PlayerDesc("Rosalind", "Franklin", 1),
      PlayerDesc("Sophie", "Germain", 1),
      PlayerDesc("Thomas", "Edison", 0),
  };
  // The players below didn't make it to the game.. so sad.
  //      PlayerDesc("Hugo", "Steinhaus", 0),
  //      PlayerDesc("Paul", "Erdos", 0),
  //      PlayerDesc("John", "Nash", 0),
  //      PlayerDesc("John", "von Neumann", 0),
  //      PlayerDesc("Charles", "Darwin", 0),
  //      PlayerDesc("Stephen", "Hawking", 0),
  //      PlayerDesc("Louis", "Pasteur", 0),
  //      PlayerDesc("Niels", "Bohr", 0),
  //      PlayerDesc("Alfred", "Nobel", 0),
  //      PlayerDesc("Alessandro", "Volta", 0),
  //      PlayerDesc("Alfred", "Tarski", 0),
  //      PlayerDesc("Merit-Ptah", "Merit-Ptah", 1),
  //      PlayerDesc("Hertha", "Ayrton", 1),
  //      PlayerDesc("Caroline", "Herschel", 1),
  //      PlayerDesc("Candace", "Pert", 1),
  for (int x = 0; x < scenario.left_team.size(); x++) {
    PlayerData* player = teamData[0]->GetPlayerData(x);
    player->UpdateName(names[x].getFirstName(), names[x].getLastName());
    if (names[x].getValue()) {
      player->SetHairStyle("long02");
      player->SetModelId(1);
    }
  }
  for (int x = 0; x < scenario.right_team.size(); x++) {
    PlayerData* player = teamData[1]->GetPlayerData(x);
    player->UpdateName(names[x+11].getFirstName(), names[x+11].getLastName());
    if (names[x+11].getValue()) {
      player->SetHairStyle("long02");
      player->SetModelId(1);
    }
  }
  goalCount[0] = 0;
  goalCount[1] = 0;

  shots[0] = 0;
  shots[1] = 0;

  possession60seconds = 0.0f;
}

MatchData::~MatchData() {
  delete teamData[0];
  delete teamData[1];
}

void MatchData::AddPossessionTime_10ms(int teamID) {
  if (teamID == 0) possession60seconds = std::max(possession60seconds - 0.01f, -60.0f);
  else if (teamID == 1) possession60seconds = std::min(possession60seconds + 0.01f, 60.0f);
}
