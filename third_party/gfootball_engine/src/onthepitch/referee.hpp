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

#ifndef _HPP_REFEREE
#define _HPP_REFEREE

#include <set>

#include "../defines.hpp"
#include "../gamedefines.hpp"

#include "../scene/scene3d/scene3d.hpp"

using namespace blunted;

class Match;

struct RefereeBuffer {
  // Referee has pending action to execute.
  bool active = false;
  e_GameMode desiredSetPiece;
  signed int teamID = 0;
  signed int setpiece_teamID = 0;
  unsigned long stopTime = 0;
  unsigned long prepareTime = 0;
  unsigned long startTime = 0;
  Vector3 restartPos;
  Player *taker;
  bool endPhase = false;
};

struct Foul {
  Player *foulPlayer;
  Player *foulVictim;
  int foulType = 0; // 0: nothing, 1: foul, 2: yellow, 3: red
  bool advantage = false;
  unsigned long foulTime = 0;
  Vector3 foulPosition;
  bool hasBeenProcessed = false;
};

class Referee {

  public:
    Referee(Match *match);
    virtual ~Referee();

    void Process();

    void PrepareSetPiece(e_GameMode setPiece);

    const RefereeBuffer &GetBuffer() { return buffer; };

    void AlterSetPiecePrepareTime(unsigned long newTime_ms);

    void BallTouched();
    void TripNotice(Player *tripee, Player *tripper, int tackleType); // 1 == standing tackle resulting in little trip, 2 == standing tackle resulting in fall, 3 == sliding tackle
    bool CheckFoul();

    Player *GetCurrentFoulPlayer() { return foul.foulPlayer; }
    int GetCurrentFoulType() { return foul.foulType; }

  protected:
    Match *match;

    boost::shared_ptr<Scene3D> scene3D;

    RefereeBuffer buffer;

    int afterSetPieceRelaxTime_ms = 0; // throw-ins cause immediate new throw-ins, because ball is still outside the lines at the moment of throwing ;)

    // Players on offside position at the time of the last ball touch.
    std::set<Player*> offsidePlayers;

    Foul foul;
};

#endif
