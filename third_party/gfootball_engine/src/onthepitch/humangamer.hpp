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

#ifndef _HPP_HUMANGAMER
#define _HPP_HUMANGAMER

#include "../defines.hpp"

#include "../scene/scene3d/scene3d.hpp"

#include "player/controller/humancontroller.hpp"
#include "../hid/ihidevice.hpp"

using namespace blunted;

class Team;

enum e_PlayerColor {
  e_PlayerColor_Blue,
  e_PlayerColor_Green,
  e_PlayerColor_Red,
  e_PlayerColor_Yellow,
  e_PlayerColor_Purple,
  e_PlayerColor_Default
};

class HumanGamer {

  public:
    HumanGamer(Team *team, IHIDevice *hid, e_PlayerColor color);
    virtual ~HumanGamer();

    int GetSelectedPlayerID() const;
    Player *GetSelectedPlayer() const { return selectedPlayer; }
    void SetSelectedPlayerID(int id);
    IHIDevice *GetHIDevice() { return hid; }

    e_PlayerColor GetPlayerColor() const { return playerColor; }

  protected:
    Team *team;
    IHIDevice *hid;
    HumanController *controller;

    e_PlayerColor playerColor;
    Player *selectedPlayer;

};

#endif
