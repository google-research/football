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

#include "../scene/objectfactory.hpp"

#include "../main.hpp"

HumanGamer::HumanGamer(Team *team, AIControlledKeyboard *hid)
    : team(team), hid(hid), controller(team->GetMatch(), hid) {
  DO_VALIDATION;
  SetSelectedPlayer(0);
}

HumanGamer::~HumanGamer() {
  DO_VALIDATION;
  if (selectedPlayer) {
    DO_VALIDATION;
    selectedPlayer->SetExternalController(0);
  }
}

void HumanGamer::SetSelectedPlayer(Player *player) {
  DO_VALIDATION;
  if (player && player->ExternalController()) {
    return;
  }
  if (selectedPlayer) {
    DO_VALIDATION;
    if (selectedPlayer == player) return;
    selectedPlayer->SetExternalController(0);
  }
  if (player) {
    DO_VALIDATION;
    selectedPlayer = player;
    selectedPlayer->SetExternalController(this);
  } else {
    selectedPlayer = 0;
  }
}

void HumanGamer::ProcessState(EnvState *state) {
  DO_VALIDATION;
  state->process(selectedPlayer);
  state->process(team);
  state->setValidate(false);
  state->process(hid);
  state->setValidate(true);
  controller.PreProcess(team->GetMatch(), hid);
  controller.ProcessState(state);
}
