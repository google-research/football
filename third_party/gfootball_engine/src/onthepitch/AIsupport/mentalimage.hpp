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

#ifndef _HPP_AISUPPORT_MENTALIMAGE
#define _HPP_AISUPPORT_MENTALIMAGE

#include "../../defines.hpp"

#include "../../base/math/vector3.hpp"

#include "../../gamedefines.hpp"

using namespace blunted;

class Match;
class PlayerBase;

class MentalImage {
 public:
  MentalImage() { DO_VALIDATION;}
  MentalImage(Match *match);
  void Mirror(bool team_0, bool team_1, bool ball);
  PlayerImage GetPlayerImage(PlayerBase* player) const;
  std::vector<PlayerImagePosition> GetTeamPlayerImages(int teamID) const;
  void UpdateBallPredictions();
  Vector3 GetBallPrediction(int time_ms) const;
  int GetTimeStampNeg_ms() const;
  void ProcessState(EnvState* state, Match* match);

  std::vector<PlayerImage> players;
  std::vector<Vector3> ballPredictions;
  unsigned int timeStamp_ms = 0;
  float maxDistanceDeviation = 2.5f;
  float maxMovementDeviation = walkVelocity;
  bool ballPredictions_mirrored = false;
 private:
  Match *match = nullptr;
};

#endif
