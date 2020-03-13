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

#include "mentalimage.hpp"

#include "../../main.hpp"
#include "../match.hpp"

MentalImage::MentalImage(Match* match)
    : timeStamp_ms(match->GetActualTime_ms()), match(match) {
  DO_VALIDATION;
  timeStamp_ms = match->GetActualTime_ms();
  std::vector<Player*> allPlayers;
  match->GetTeam(match->FirstTeam())->GetActivePlayers(allPlayers);
  match->GetTeam(match->SecondTeam())->GetActivePlayers(allPlayers);
  players.resize(allPlayers.size());

  for (int playerCounter = 0; playerCounter < (signed int)allPlayers.size();
       playerCounter++) {
    DO_VALIDATION;

    Player *player = allPlayers.at(playerCounter);

    PlayerImage& playerImage = players[playerCounter];
    playerImage.player = player;
    playerImage.position = player->GetPosition();
    playerImage.directionVec = player->GetDirectionVec();
    playerImage.velocity = player->GetEnumVelocity();
    playerImage.movement = player->GetMovement();
    playerImage.role = player->GetDynamicFormationEntry().role;
  }

  UpdateBallPredictions();
}

void MentalImage::Mirror(bool team_0, bool team_1, bool ball) {
  for (auto& i : players) {
    if (i.player->GetTeamID() == 0 ? team_0 : team_1) {
      i.Mirror();
    }
  }
  if (ball) {
    ballPredictions_mirrored = !ballPredictions_mirrored;
    for (auto& i : ballPredictions) {
      i.Mirror();
    }
  }
}

int MentalImage::GetTimeStampNeg_ms() const { return match->GetActualTime_ms() - timeStamp_ms; }

void MentalImage::ProcessState(EnvState* state, Match* match) {
  this->match = match;
  state->process(timeStamp_ms);
  state->process(maxDistanceDeviation);
  state->process(maxMovementDeviation);
  int size = players.size();
  state->process(size);
  players.resize(size);
  for (auto& p : players) {
    p.ProcessState(state);
  }
  size = ballPredictions.size();
  state->process(size);
  ballPredictions.resize(size);
  for (auto& b : ballPredictions) {
    if (state->getConfig()->reverse_team_processing &&
        !ballPredictions_mirrored) {
      b.Mirror();
    }
    state->process(b);
    if (state->getConfig()->reverse_team_processing &&
        !ballPredictions_mirrored) {
      b.Mirror();
    }
  }
}

PlayerImage MentalImage::GetPlayerImage(PlayerBase* p) const {
  for (auto& player : players) {
    DO_VALIDATION;
    if (player.player == p) {
      DO_VALIDATION;
      PlayerImage newImage = player;
      Vector3 extrapolation = player.movement * GetTimeStampNeg_ms() * 0.001f;
      newImage.position = player.position + extrapolation;
      newImage.position = newImage.position.EnforceMaximumDeviation(newImage.player->GetPosition(), maxDistanceDeviation);
      newImage.movement = newImage.movement.EnforceMaximumDeviation(newImage.player->GetMovement(), maxMovementDeviation);
      return newImage;
    }
  }

  // failsafe
  return players[0];
}

std::vector<PlayerImagePosition> MentalImage::GetTeamPlayerImages(int teamID) const {
  std::vector<PlayerImagePosition> result;
  result.reserve(11);
  for (auto& player : players) {
    DO_VALIDATION;
    if (player.player->IsActive() && player.player->GetTeamID() == teamID) {
      DO_VALIDATION;
      Vector3 extrapolation = player.movement * GetTimeStampNeg_ms() * 0.001f;
      Vector3 position = player.position + extrapolation;
      position = position.EnforceMaximumDeviation(player.player->GetPosition(), maxDistanceDeviation);
      Vector3 movement = player.movement.EnforceMaximumDeviation(player.player->GetMovement(), maxMovementDeviation); // new
      result.emplace_back(position, movement, player.role);
    }
  }
  return result;
}

void MentalImage::UpdateBallPredictions() {
  DO_VALIDATION;
  match->GetBall()->GetPredictionArray(ballPredictions);
}

Vector3 MentalImage::GetBallPrediction(int time_ms) const {

  int index = time_ms + match->GetActualTime_ms() - timeStamp_ms;
  if (index >= ballPredictionSize_ms) index = ballPredictionSize_ms - 10;
  index = index / 10;
  if (index < 0) index = 0;

  Vector3 mentalResult = ballPredictions[index];
  Vector3 realResult = match->GetBall()->Predict(time_ms);

  // let there be a maximum difference between the two. why?
  // when a ball gets a wholly new movement, this prediction is obviously far off reality, while some variables are not,
  // like the player->gettimeneededtogettoball, since that is based on non-delayed vars.
  // a solution would be to have a reaction-time-corrected version of everything, but that is, for now, too complicated.
  // maybe one day rebuild the whole timeneeded/tactics calculations system

  Vector3 result = mentalResult.EnforceMaximumDeviation(realResult, maxDistanceDeviation);

  return result;
}
