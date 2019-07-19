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

#include "referee.hpp"
#include <cmath>

#include "../scene/objectfactory.hpp"
#include "../managers/resourcemanagerpool.hpp"

#include "match.hpp"
#include "AIsupport/AIfunctions.hpp"

#include "../main.hpp"

Referee::Referee(Match *match) : match(match) {
  buffer.desiredSetPiece = e_GameMode_KickOff;
  buffer.teamID = 0;
  buffer.setpiece_teamID = 0;
  buffer.stopTime = 0;
  buffer.prepareTime = 0;
  buffer.startTime = buffer.prepareTime + 2000;
  buffer.restartPos = GetScenarioConfig().ball_position;
  buffer.taker = 0;
  buffer.endPhase = true;
  buffer.active = true;

  foul.foulPlayer = 0;
  foul.foulType = 0;
  foul.advantage = false;
  foul.foulTime = 0;
  foul.hasBeenProcessed = true;

  afterSetPieceRelaxTime_ms = 0;


  // for usage in destructor
  scene3D = GetScene3D();
}

Referee::~Referee() {
}

void Referee::Process() {
  //printf("%i", match->GetMatchState());

  if (match->IsInPlay() && !match->IsInSetPiece()) {

    Vector3 ballPos = match->GetBall()->Predict(0);

    // goal kick / corner

    if (fabs(ballPos.coords[0]) > pitchHalfW + lineHalfW + 0.11) {

      foul.advantage = false;
      bool isFoul = false;
      if (!match->IsGoalScored()) isFoul = CheckFoul(); else foul.foulType = 0;
      if (isFoul == false) {

        match->StopPlay();

        // corner, goal kick or kick off?
        signed int lastSide = -1;
        Team *lastTouchTeam = match->GetLastTouchTeam();
        if (lastTouchTeam == 0) lastTouchTeam = match->GetTeam(0);
        lastSide = lastTouchTeam->GetSide();

        if (match->IsGoalScored()) {
          buffer.desiredSetPiece = e_GameMode_KickOff;
          buffer.stopTime = match->GetActualTime_ms();
          // Number of ms for replay.
          buffer.prepareTime = match->GetActualTime_ms() + 500;
          // Number of ms for kickoff.
          buffer.startTime = buffer.prepareTime + 500;
          buffer.restartPos = Vector3(0, 0, 0);
          buffer.teamID = 0;//abs(match->GetLastGoalTeamID() - 1);
          buffer.setpiece_teamID = abs(match->GetLastGoalTeamID() - 1);
          match->SetMatchPhase(e_MatchPhase_1stHalf);
        } else if ((ballPos.coords[0] > 0 && lastSide > 0) || (ballPos.coords[0] < 0 && lastSide < 0)) {
          buffer.desiredSetPiece = e_GameMode_Corner;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          buffer.startTime = buffer.prepareTime + 2000;
          float y = ballPos.coords[1];
          if (y > 0) y = pitchHalfH; else
                     y = -pitchHalfH;
          buffer.restartPos = Vector3(pitchHalfW * lastSide, y, 0);
          buffer.teamID = abs(lastTouchTeam->GetID() - 1);

        } else {
          buffer.desiredSetPiece = e_GameMode_GoalKick;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          buffer.startTime = buffer.prepareTime + 2000;
          buffer.restartPos = Vector3(pitchHalfW * 0.92 * -lastSide, 0, 0);
          buffer.teamID = abs(lastTouchTeam->GetID() - 1);
        }

        buffer.active = true;
      }
    }


    // over sideline

    if (afterSetPieceRelaxTime_ms == 0) {
      if (fabs(ballPos.coords[1]) > pitchHalfH + lineHalfW + 0.11) {
        foul.advantage = false;
        if (!CheckFoul()) {
          match->StopPlay();
          Team *lastTouchTeam = match->GetLastTouchTeam();
          if (lastTouchTeam == 0) lastTouchTeam = match->GetTeam(0);
          buffer.teamID = abs(lastTouchTeam->GetID() - 1);
          buffer.desiredSetPiece = e_GameMode_ThrowIn;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          buffer.startTime = buffer.prepareTime + 2000;
          buffer.restartPos.coords[0] = clamp(ballPos.coords[0], -pitchHalfW + 0.6f, pitchHalfW - 0.6f);
          if (ballPos.coords[1] >  0) buffer.restartPos.coords[1] = pitchHalfH;
          if (ballPos.coords[1] <= 0) buffer.restartPos.coords[1] = -pitchHalfH;
          buffer.restartPos.coords[2] = 0;
          buffer.active = true;
        }
      }
    }

    CheckFoul();

  } else { // not in play, maybe something needs to happen?

    if (!match->IsInPlay() && !match->IsInSetPiece() && buffer.active == true) {

      if (buffer.prepareTime == match->GetActualTime_ms()) {

        if (buffer.endPhase == true) {
          if (match->GetMatchPhase() == e_MatchPhase_PreMatch) {
            match->SetMatchPhase(e_MatchPhase_1stHalf);
          } else {
            // game over conditions
            if (match->GetMatchPhase() == e_MatchPhase_1stExtraTime) {
              if (match->GetScore(0) != match->GetScore(1)) {
                match->GameOver();
                return;
              }
            }
            if (match->GetMatchPhase() == e_MatchPhase_Penalties) {
              match->GameOver();
              return;
            }
            match->sig_OnMatchPhaseChange(match);
          }
          buffer.endPhase = false;
        }

        PrepareSetPiece(buffer.desiredSetPiece);
      }

      if (buffer.startTime == match->GetActualTime_ms()) {
        // blow whistle and wait for set piece taker to touch the ball
        match->StartPlay();
        match->StartSetPiece();
      }
    }
  }

  if (match->IsInSetPiece()) {
    // check if set piece has been taken
    if (buffer.desiredSetPiece == e_GameMode_KickOff ||
        (buffer.taker->TouchAnim() && !buffer.taker->TouchPending())) {
      buffer.active = false;
      match->StopSetPiece();
      match->GetTeam(0)->GetController()->PrepareSetPiece(e_GameMode_Normal, match->GetTeam(1), -1, -1);
      match->GetTeam(1)->GetController()->PrepareSetPiece(e_GameMode_Normal, match->GetTeam(0), -1, -1);
      afterSetPieceRelaxTime_ms = 400;
      foul.foulPlayer = 0;
      foul.foulType = 0;

      if (match->GetMatchPhase() == e_MatchPhase_PreMatch) {
        match->SetMatchPhase(e_MatchPhase_1stHalf);
      }
    }
  }

  if (afterSetPieceRelaxTime_ms > 0) afterSetPieceRelaxTime_ms -= 10;
}

void Referee::PrepareSetPiece(e_GameMode setPiece) {
  // position players for set piece situation

  match->ResetSituation(buffer.restartPos);

  match->GetTeam(0)->GetController()->PrepareSetPiece(setPiece, match->GetTeam(1), buffer.setpiece_teamID, buffer.teamID);
  match->GetTeam(1)->GetController()->PrepareSetPiece(setPiece, match->GetTeam(0), buffer.setpiece_teamID, buffer.teamID);

  buffer.taker = match->GetTeam(buffer.teamID)->GetController()->GetPieceTaker();
  // Reset offside state.
  offsidePlayers.clear();
}

void Referee::AlterSetPiecePrepareTime(unsigned long newTime_ms) {
  if (buffer.active) {
    buffer.prepareTime = newTime_ms;
    buffer.startTime = buffer.prepareTime + 2000;
  }
}

void Referee::BallTouched() {

  if (!GetScenarioConfig().offsides) {
    return;
  }
  // check for offside player receiving the ball

  int lastTouchTeamID = match->GetLastTouchTeamID();
  if (lastTouchTeamID == -1) return; // shouldn't happen really ;)
  if (match->IsInPlay() && !match->IsInSetPiece() && buffer.active == false && match->GetTeam(1 - lastTouchTeamID)->GetActivePlayersCount() > 1) { // disable if only 1 player: that's debug mode with only keeper
    auto ballOwner = match->GetLastTouchPlayer();
    if (offsidePlayers.count(ballOwner)) {
      foul.advantage = false;
      if (!CheckFoul()) {
        // uooooga uooooga offside!
        match->StopPlay();
        buffer.desiredSetPiece = e_GameMode_FreeKick;
        buffer.stopTime = match->GetActualTime_ms();
        buffer.prepareTime = match->GetActualTime_ms() + 2000;
        buffer.startTime = buffer.prepareTime + 2000;
        buffer.restartPos = ballOwner->GetPosition();
        buffer.teamID = 1 - lastTouchTeamID;
        buffer.active = true;
        match->SpamMessage("offside!");
      }
    }
  }

  offsidePlayers.clear();

  if (match->IsInPlay() && (buffer.active == false ||
      (buffer.active == true &&
          buffer.desiredSetPiece != e_GameMode_ThrowIn &&
          buffer.desiredSetPiece != e_GameMode_Corner))) {
    // check for offside players at moment of touch
    MentalImage mentalImage(match);
    mentalImage.TakeSnapshot();
    float offside = AI_GetOffsideLine(match, &mentalImage, 1 - lastTouchTeamID);
    std::vector<Player*> players;
    Team *team = match->GetTeam(lastTouchTeamID);
    team->GetActivePlayers(players);
    // Offside debug line:
    // SetRedDebugPilon(Vector3(offside, -20, 0));
    // SetBlueDebugPilon(Vector3(offside, 0, 0));
    // SetGreenDebugPilon(Vector3(offside, 20, 0));
    for (auto player : players) {
      if (player != team->GetLastTouchPlayer()) {
        if (player->GetPosition().coords[0] * team->GetSide() < offside * team->GetSide()) {
          offsidePlayers.insert(player);
        }
      }
    }
  }

}

void Referee::TripNotice(Player *tripee, Player *tripper, int tackleType) {

  if (buffer.active) return;

  if (tackleType == 2) { // standing tackle
    if (tripee->GetTeam()->GetFadingTeamPossessionAmount() > 1.1 &&
        (tripper->GetCurrentFunctionType() == e_FunctionType_Interfere || tripper->GetCurrentFunctionType() == e_FunctionType_Sliding) &&
        (tripee->GetPosition() - match->GetBall()->Predict(0).Get2D()).GetLength() < 2.0 &&
        tripper->GetTeam()->GetID() != tripee->GetTeam()->GetID()) {
      // uooooga uooooga foul!
      foul.foulType = 1;
      foul.advantage = true;
      foul.foulPlayer = tripper;
      foul.foulVictim = tripee;
      foul.foulTime = match->GetActualTime_ms();
      foul.foulPosition = tripee->GetPosition();
      foul.hasBeenProcessed = false;
    }

  } else if (tackleType == 3 && (tripper != foul.foulPlayer || foul.foulType == 0)) { // sliding tackle

    if (match->GetActualTime_ms() - tripper->GetLastTouchTime_ms() > 600 &&
        tripper->GetCurrentFunctionType() == e_FunctionType_Sliding &&
        tripper->GetTeam()->GetID() != tripee->GetTeam()->GetID() && (match->GetBall()->Predict(0) - tripee->GetPosition()).GetLength() < 8.0) {
      float severity = 1.0;
      if (tripper->TouchAnim()) {
        severity = std::pow(clamp(fabs(tripper->GetTouchFrame() -
                                       tripper->GetCurrentFrame()) /
                                      tripper->GetTouchFrame(),
                                  0.0, 1.0),
                            0.7) *
                   0.5;
        severity += NormalizedClamp((match->GetBall()->Predict(0) - tripper->GetTouchPos()).GetLength(), 0.0, 2.0) * 0.5;
      }
      // from behind?
      severity += (tripee->GetPosition() - tripper->GetPosition()).GetNormalized(0).GetDotProduct(tripee->GetDirectionVec()) * 0.5 + 0.5;

      if (severity > 1.0) {
        // uooooga uooooga foul!
        //printf("sliding! %lu ms ago\n", match->GetActualTime_ms() - tripper->GetLastTouchTime_ms());
        foul.foulType = 1;
        foul.advantage = false;
        foul.foulPlayer = tripper;
        foul.foulVictim = tripee;
        foul.foulTime = match->GetActualTime_ms();
        foul.foulPosition = tripee->GetPosition();
        foul.hasBeenProcessed = false;
        if (severity > 1.4) foul.foulType = 2;
        if (severity > 2.0) {
          foul.foulType = 3;
        }
      }
    }

  }
}

bool Referee::CheckFoul() {

  bool penalty = false;
  if (foul.foulType != 0) {
    if (fabs(foul.foulPosition.coords[1]) < 20.15 - lineHalfW && foul.foulPosition.coords[0] * -foul.foulVictim->GetTeam()->GetSide() > pitchHalfW - 16.5 + lineHalfW) penalty = true;
  }

  if (foul.advantage) {
    if (penalty) {
      foul.advantage = false;
    } else {
      if (match->GetActualTime_ms() - 600 > foul.foulTime) {
        if (match->GetActualTime_ms() - 3000 > foul.foulTime) {
          // cancel foul, advantage took long enough

          foul.foulPlayer = 0;
          foul.foulType = 0;
        } else {
          // calculate if there's advantage still
          if (foul.foulVictim->GetTeam()->GetFadingTeamPossessionAmount() < 1.0) {
            foul.advantage = false;
          }
        }
      }
    }
  }

  if (foul.foulType != 0 && foul.advantage == false && !foul.hasBeenProcessed) {

    match->StopPlay();
    if (!penalty) {
      buffer.desiredSetPiece = e_GameMode_FreeKick;
      buffer.stopTime = match->GetActualTime_ms();
      buffer.prepareTime = match->GetActualTime_ms() + 2000;
      if (foul.foulType >= 2) buffer.prepareTime += 10000;
      buffer.startTime = buffer.prepareTime + 2000;
      buffer.restartPos = foul.foulPosition;
    } else {
      buffer.desiredSetPiece = e_GameMode_Penalty;
      buffer.stopTime = match->GetActualTime_ms();
      buffer.prepareTime = match->GetActualTime_ms() + 2000;
      if (foul.foulType >= 2) buffer.prepareTime += 10000;
      buffer.startTime = buffer.prepareTime + 2000;
      buffer.restartPos = Vector3((pitchHalfW - 11.0) * foul.foulPlayer->GetTeam()->GetSide(), 0, 0);
    }
    buffer.teamID = foul.foulVictim->GetTeam()->GetID();
    buffer.active = true;
    std::string spamMessage = "foul!";
    if (foul.foulType == 2) {
      spamMessage.append(" yellow card");
      foul.foulPlayer->GiveYellowCard(match->GetActualTime_ms() + 6000); // need to find out proper moment
    }
    if (foul.foulType == 3) {
      spamMessage.append(" red card!!!");
      foul.foulPlayer->GiveRedCard(match->GetActualTime_ms() + 6000); // need to find out proper moment
    }
    match->SpamMessage(spamMessage);

    foul.hasBeenProcessed = true;

    return true;
  }

  return false;
}
