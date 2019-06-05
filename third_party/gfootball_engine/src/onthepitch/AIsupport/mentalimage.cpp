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

#include "../match.hpp"

MentalImage::MentalImage(Match *match) : match(match) {
  timeStampNeg_ms = 0;
  maxDistanceDeviation = 2.5f; // if reality is this much (or more) off from mental image, enforce as maximum offset
  maxMovementDeviation = walkVelocity;
}

MentalImage::~MentalImage() {
}

void MentalImage::TakeSnapshot() {
  players.clear();

  std::vector<Player*> allPlayers;
  match->GetTeam(0)->GetActivePlayers(allPlayers);
  match->GetTeam(1)->GetActivePlayers(allPlayers);

  for (int playerCounter = 0; playerCounter < (signed int)allPlayers.size(); playerCounter++) {

    Player *player = allPlayers.at(playerCounter);

    PlayerImage playerImage;
    playerImage.teamID = player->GetTeamID();
    playerImage.side = player->GetTeam()->GetSide();
    playerImage.playerID = player->GetID();
    playerImage.player = player;
    playerImage.position = player->GetPosition();
    playerImage.directionVec = player->GetDirectionVec();
    playerImage.bodyDirectionVec = player->GetBodyDirectionVec();
    playerImage.velocity = player->GetFloatVelocity();
    playerImage.movement = player->GetMovement();
    playerImage.formationEntry = player->GetFormationEntry();
    playerImage.dynamicFormationEntry = player->GetDynamicFormationEntry();
    players.push_back(playerImage);
  }

  UpdateBallPredictions();
}

PlayerImage MentalImage::GetPlayerImage(int playerID) const {

  for (unsigned int playerCounter = 0; playerCounter < players.size(); playerCounter++) {

    if (players.at(playerCounter).playerID == playerID) {
      PlayerImage newImage = players.at(playerCounter);
      Vector3 extrapolation = players.at(playerCounter).movement * GetTimeStampNeg_ms() * 0.001f;
      newImage.position = players.at(playerCounter).position + extrapolation;
      newImage.position = newImage.position.EnforceMaximumDeviation(newImage.player->GetPosition(), maxDistanceDeviation);
      newImage.movement = newImage.movement.EnforceMaximumDeviation(newImage.player->GetMovement(), maxMovementDeviation);
      return newImage;
    }
  }

  // failsafe
  return players.at(0);
}

void MentalImage::GetTeamPlayerImages(int teamID, int exceptPlayerID, std::vector<PlayerImage> &playerImages) const {
  for (unsigned int playerCounter = 0; playerCounter < players.size(); playerCounter++) {
    Player *player = match->GetPlayer(players.at(playerCounter).playerID);
    assert(player);
    if (player->IsActive() && player->GetTeamID() == teamID && player->GetID() != exceptPlayerID) {
      PlayerImage newImage = players.at(playerCounter);
      Vector3 extrapolation = players.at(playerCounter).movement * GetTimeStampNeg_ms() * 0.001f;
      newImage.position = players.at(playerCounter).position + extrapolation;
      newImage.position = newImage.position.EnforceMaximumDeviation(newImage.player->GetPosition(), maxDistanceDeviation);
      newImage.movement = newImage.movement.EnforceMaximumDeviation(newImage.player->GetMovement(), maxMovementDeviation); // new
      playerImages.push_back(newImage);
    }
  }
}

void MentalImage::UpdateBallPredictions() {
  match->GetBall()->GetPredictionArray(ballPredictions);
}

Vector3 MentalImage::GetBallPrediction(unsigned int time_ms) const {

  unsigned int index = time_ms + timeStampNeg_ms;
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
