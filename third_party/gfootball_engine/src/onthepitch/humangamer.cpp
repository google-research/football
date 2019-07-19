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

#include "humangamer.hpp"

#include "team.hpp"

#include "../managers/resourcemanagerpool.hpp"

#include "../scene/objectfactory.hpp"

#include "../main.hpp"

HumanGamer::HumanGamer(Team *team, IHIDevice *hid, e_PlayerColor color) : team(team), hid(hid), playerColor(color) {
  controller = new HumanController(team->GetMatch(), hid);

  std::vector<Player*> activePlayers;
  team->GetActivePlayers(activePlayers);
  selectedPlayer = 0;
  SetSelectedPlayerID(-1);
}

HumanGamer::~HumanGamer() {
  delete controller;

  if (selectedPlayer) {
    selectedPlayer->SetExternalController(0);
  }
}

int HumanGamer::GetSelectedPlayerID() const {
  if (selectedPlayer) return selectedPlayer->GetID(); else return -1;
}

void HumanGamer::SetSelectedPlayerID(int id) {
  if (selectedPlayer) {
    if (selectedPlayer->GetID() == id) return;
    selectedPlayer->SetExternalController(0);
  }
  if (id != -1) {
    selectedPlayer = team->GetPlayer(id);
    selectedPlayer->SetExternalController(controller);
  } else {
    selectedPlayer = 0;
  }
}
