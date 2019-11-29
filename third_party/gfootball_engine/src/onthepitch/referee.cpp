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
#include "match.hpp"
#include "AIsupport/AIfunctions.hpp"

#include "../main.hpp"

void Foul::ProcessState(EnvState *state) {
  state->process(foulPlayer);
  state->process(foulVictim);
  state->process(foulType);
  state->process(advantage);
  state->process(foulTime);
  if (state->getConfig()->reverse_team_processing) {
    foulPosition.Mirror();
  }
  state->process(foulPosition);
  if (state->getConfig()->reverse_team_processing) {
    foulPosition.Mirror();
  }
  state->process(hasBeenProcessed);
}

void RefereeBuffer::ProcessState(EnvState *state) {
  DO_VALIDATION;
  state->process(active);
  state->process((void*) &desiredSetPiece, sizeof(desiredSetPiece));
  if (state->getConfig()->reverse_team_processing) {
    teamID = 1 - teamID;
  }
  state->process(teamID);
  if (state->getConfig()->reverse_team_processing) {
    teamID = 1 - teamID;
  }
  state->process(setpiece_team);
  state->process(stopTime);
  state->process(prepareTime);
  state->process(startTime);
  if (state->getConfig()->reverse_team_processing) {
    restartPos.Mirror();
  }
  state->process(restartPos);
  if (state->getConfig()->reverse_team_processing) {
    restartPos.Mirror();
  }
  state->process(taker);
  state->process(endPhase);
}

Referee::Referee(Match *match, bool animations) : match(match), animations(animations) {
  DO_VALIDATION;
  buffer.desiredSetPiece = e_GameMode_KickOff;
  buffer.teamID = match->FirstTeam();
  buffer.setpiece_team = match->GetTeam(match->FirstTeam());
  buffer.stopTime = 0;
  buffer.prepareTime = 0;
  buffer.startTime = 2000;
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
}

Referee::~Referee() { DO_VALIDATION; }

void Referee::Process() {
  DO_VALIDATION;
  if (match->IsInPlay() && !match->IsInSetPiece()) {
    DO_VALIDATION;

    Vector3 ballPos = match->GetBall()->Predict(0);
    // We process corner setup in not mirrored setup.
    if (GetScenarioConfig().reverse_team_processing) {
      ballPos.Mirror();
    }

    // goal kick / corner

    if (fabs(ballPos.coords[0]) > pitchHalfW + lineHalfW + 0.11 ||
        match->IsGoalScored()) {
      DO_VALIDATION;

      foul.advantage = false;
      bool isFoul = false;
      if (!match->IsGoalScored()) isFoul = CheckFoul(); else foul.foulType = 0;
      if (isFoul == false) {
        DO_VALIDATION;

        match->StopPlay();

        // corner, goal kick or kick off?
        Team *lastTouchTeam = match->GetLastTouchTeam();
        if (lastTouchTeam == 0) lastTouchTeam = match->GetTeam(GetScenarioConfig().reverse_team_processing ? 1 : 0);
        signed int lastSide = lastTouchTeam->GetStaticSide();

        if (match->IsGoalScored()) {
          DO_VALIDATION;
          buffer.desiredSetPiece = e_GameMode_KickOff;
          buffer.stopTime = match->GetActualTime_ms();
          // Number of ms for replay.
          buffer.prepareTime = match->GetActualTime_ms() + 500;
          if (!animations) {
            match->BumpActualTime_ms(400);
          }
          // Number of ms for kickoff.
          buffer.startTime = buffer.prepareTime + 500;
          buffer.restartPos = Vector3(0, 0, 0);
          buffer.teamID = match->FirstTeam();
          buffer.setpiece_team = match->GetLastGoalTeam()->Opponent();
          match->SetMatchPhase(e_MatchPhase_1stHalf);
        } else if ((ballPos.coords[0] > 0 && lastSide > 0) ||
                   (ballPos.coords[0] < 0 && lastSide < 0)) {
          DO_VALIDATION;
          buffer.desiredSetPiece = e_GameMode_Corner;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          if (!animations) {
            match->BumpActualTime_ms(1900);
          }
          buffer.startTime = buffer.prepareTime + 2000;
          float y = ballPos.coords[1];
          if (y > 0) y = pitchHalfH; else
                     y = -pitchHalfH;
          buffer.restartPos = Vector3(pitchHalfW * lastSide, y, 0);
          buffer.teamID = 1 - lastTouchTeam->GetID();
        } else {
          buffer.desiredSetPiece = e_GameMode_GoalKick;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          if (!animations) {
            match->BumpActualTime_ms(1900);
          }
          buffer.startTime = buffer.prepareTime + 2000;
          buffer.restartPos = Vector3(pitchHalfW * 0.92 * -lastSide, 0, 0);
          buffer.teamID = 1 - lastTouchTeam->GetID();
        }

        buffer.active = true;
      }
    }

    // over sideline

    if (afterSetPieceRelaxTime_ms == 0) {
      DO_VALIDATION;
      if (fabs(ballPos.coords[1]) > pitchHalfH + lineHalfW + 0.11) {
        DO_VALIDATION;
        foul.advantage = false;
        if (!CheckFoul()) {
          DO_VALIDATION;
          match->StopPlay();
          Team *lastTouchTeam = match->GetLastTouchTeam();
          if (lastTouchTeam == 0) lastTouchTeam = match->GetTeam(0);
          buffer.teamID = 1 - lastTouchTeam->GetID();
          buffer.desiredSetPiece = e_GameMode_ThrowIn;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          if (!animations) {
            match->BumpActualTime_ms(1900);
          }
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

  } else {  // not in play, maybe something needs to happen?

    if (!match->IsInPlay() && !match->IsInSetPiece() && buffer.active == true) {
      DO_VALIDATION;

      if (buffer.prepareTime == match->GetActualTime_ms()) {
        DO_VALIDATION;
        if (buffer.endPhase == true) {
          if (match->GetMatchPhase() == e_MatchPhase_PreMatch) {
            match->SetMatchPhase(e_MatchPhase_1stHalf);
          }
          buffer.endPhase = false;
        }

        randomize(GetScenarioConfig().game_engine_random_seed);
        PrepareSetPiece(buffer.desiredSetPiece);
        DO_VALIDATION;
      }

      if (buffer.startTime == match->GetActualTime_ms()) {
        DO_VALIDATION;
        // blow whistle and wait for set piece taker to touch the ball
        match->StartPlay();
        match->StartSetPiece();
      }
    }
  }

  if (match->IsInSetPiece()) {
    DO_VALIDATION;
    // check if set piece has been taken
    if (buffer.desiredSetPiece == e_GameMode_KickOff ||
        (buffer.taker->TouchAnim() && !buffer.taker->TouchPending())) {
      DO_VALIDATION;
      buffer.active = false;
      match->StopSetPiece();
      match->GetTeam(0)->GetController()->PrepareSetPiece(e_GameMode_Normal, match->GetTeam(1), -1, -1);
      match->GetTeam(1)->GetController()->PrepareSetPiece(e_GameMode_Normal, match->GetTeam(0), -1, -1);
      afterSetPieceRelaxTime_ms = 400;
      foul.foulPlayer = 0;
      foul.foulType = 0;

      if (match->GetMatchPhase() == e_MatchPhase_PreMatch) {
        DO_VALIDATION;
        match->SetMatchPhase(e_MatchPhase_1stHalf);
      }
    }
  }

  if (afterSetPieceRelaxTime_ms > 0) afterSetPieceRelaxTime_ms -= 10;
}

void Referee::PrepareSetPiece(e_GameMode setPiece) {
  DO_VALIDATION;
  // position players for set piece situation
  match->ResetSituation(GetScenarioConfig().reverse_team_processing
                            ? -buffer.restartPos
                            : buffer.restartPos);

  match->GetTeam(match->FirstTeam())
      ->GetController()
      ->PrepareSetPiece(setPiece, match->GetTeam(match->SecondTeam()),
                        buffer.setpiece_team->GetID(), buffer.teamID);
  match->GetTeam(match->SecondTeam())
      ->GetController()
      ->PrepareSetPiece(setPiece, match->GetTeam(match->FirstTeam()),
                        buffer.setpiece_team->GetID(), buffer.teamID);

  buffer.taker = match->GetTeam(buffer.teamID)->GetController()->GetPieceTaker();
  // Reset offside state.
  offsidePlayers.clear();
}

void Referee::AlterSetPiecePrepareTime(unsigned long newTime_ms) {
  DO_VALIDATION;
  if (buffer.active) {
    DO_VALIDATION;
    buffer.prepareTime = newTime_ms;
    buffer.startTime = buffer.prepareTime + 2000;
  }
}

void Referee::BallTouched() {
  DO_VALIDATION;

  if (!GetScenarioConfig().offsides) {
    DO_VALIDATION;
    return;
  }
  // check for offside player receiving the ball

  int lastTouchTeamID = match->GetLastTouchTeamID();
  if (lastTouchTeamID == -1) return; // shouldn't happen really ;)
  if (match->IsInPlay() && !match->IsInSetPiece() && buffer.active == false &&
      match->GetTeam(1 - lastTouchTeamID)->GetActivePlayersCount() > 1) {
    DO_VALIDATION;  // disable if only 1 player: that's debug mode with only
                    // keeper
    auto ballOwner = match->GetLastTouchPlayer();
    for (auto p : offsidePlayers) {
      DO_VALIDATION;
      if (p == ballOwner) {
        DO_VALIDATION;
        foul.advantage = false;
        if (!CheckFoul()) {
          DO_VALIDATION;
          // uooooga uooooga offside!
          match->StopPlay();
          buffer.desiredSetPiece = e_GameMode_FreeKick;
          buffer.stopTime = match->GetActualTime_ms();
          buffer.prepareTime = match->GetActualTime_ms() + 2000;
          if (!animations) {
            match->BumpActualTime_ms(1900);
          }
          buffer.startTime = buffer.prepareTime + 2000;
          buffer.restartPos = ballOwner->GetPitchPosition();
          buffer.teamID = 1 - lastTouchTeamID;
          buffer.active = true;
          match->SpamMessage("offside!");
        }
      }
    }
  }

  offsidePlayers.clear();

  if (match->IsInPlay() &&
      (buffer.active == false ||
       (buffer.active == true && buffer.desiredSetPiece != e_GameMode_ThrowIn &&
        buffer.desiredSetPiece != e_GameMode_Corner))) {
    DO_VALIDATION;
    // check for offside players at moment of touch
    MentalImage mentalImage(match);
    float offside = AI_GetOffsideLine(match, &mentalImage, 1 - lastTouchTeamID);
    std::vector<Player*> players;
    Team *team = match->GetTeam(lastTouchTeamID);
    team->GetActivePlayers(players);
    for (auto player : players) {
      DO_VALIDATION;
      if (player != team->GetLastTouchPlayer()) {
        DO_VALIDATION;
        if (player->GetPosition().coords[0] * team->GetDynamicSide() <
            offside * team->GetDynamicSide()) {
          DO_VALIDATION;
          offsidePlayers.push_back(player);
        }
      }
    }
  }
}

void Referee::TripNotice(Player *tripee, Player *tripper, int tackleType) {
  DO_VALIDATION;

  if (buffer.active) return;

  if (tackleType == 2) {
    DO_VALIDATION;  // standing tackle
    if (tripee->GetTeam()->GetFadingTeamPossessionAmount() > 1.1 &&
        (tripper->GetCurrentFunctionType() == e_FunctionType_Interfere ||
         tripper->GetCurrentFunctionType() == e_FunctionType_Sliding) &&
        (tripee->GetPosition() - match->GetBall()->Predict(0).Get2D())
                .GetLength() < 2.0 &&
        tripper->GetTeam()->GetID() != tripee->GetTeam()->GetID()) {
      DO_VALIDATION;
      // uooooga uooooga foul!
      foul.foulType = 1;
      foul.advantage = true;
      foul.foulPlayer = tripper;
      foul.foulVictim = tripee;
      foul.foulTime = match->GetActualTime_ms();
      foul.foulPosition = tripee->GetPitchPosition();
      foul.hasBeenProcessed = false;
    }

  } else if (tackleType == 3 &&
             (tripper != foul.foulPlayer || foul.foulType == 0)) {
    DO_VALIDATION;  // sliding tackle

    if (match->GetActualTime_ms() - tripper->GetLastTouchTime_ms() > 600 &&
        tripper->GetCurrentFunctionType() == e_FunctionType_Sliding &&
        tripper->GetTeam()->GetID() != tripee->GetTeam()->GetID() &&
        (match->GetBall()->Predict(0) - tripee->GetPosition()).GetLength() <
            8.0) {
      DO_VALIDATION;
      float severity = 1.0;
      if (tripper->TouchAnim()) {
        DO_VALIDATION;
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
        DO_VALIDATION;
        // uooooga uooooga foul!
        //printf("sliding! %lu ms ago\n", match->GetActualTime_ms() - tripper->GetLastTouchTime_ms());
        foul.foulType = 1;
        foul.advantage = false;
        foul.foulPlayer = tripper;
        foul.foulVictim = tripee;
        foul.foulTime = match->GetActualTime_ms();
        foul.foulPosition = tripee->GetPitchPosition();
        foul.hasBeenProcessed = false;
        if (severity > 1.4) foul.foulType = 2;
        if (severity > 2.0) {
          DO_VALIDATION;
          foul.foulType = 3;
        }
      }
    }
  }
}

void Referee::ProcessState(EnvState *state) {
  DO_VALIDATION;
  buffer.ProcessState(state);
  state->process(afterSetPieceRelaxTime_ms);
  int size = offsidePlayers.size();
  state->process(size);
  offsidePlayers.resize(size);
  for (auto &i : offsidePlayers) {
    DO_VALIDATION;
    state->process(i);
  }
  foul.ProcessState(state);
}

bool Referee::CheckFoul() {
  DO_VALIDATION;

  bool penalty = false;
  if (foul.foulType != 0) {
    DO_VALIDATION;
    if (fabs(foul.foulPosition.coords[1]) < 20.15 - lineHalfW &&
        foul.foulPosition.coords[0] *
                -foul.foulVictim->GetTeam()->GetStaticSide() >
            pitchHalfW - 16.5 + lineHalfW)
      penalty = true;
  }

  if (foul.advantage) {
    DO_VALIDATION;
    if (penalty) {
      DO_VALIDATION;
      foul.advantage = false;
    } else {
      if (match->GetActualTime_ms() - 600 > foul.foulTime) {
        DO_VALIDATION;
        if (match->GetActualTime_ms() - 3000 > foul.foulTime) {
          DO_VALIDATION;
          // cancel foul, advantage took long enough

          foul.foulPlayer = 0;
          foul.foulType = 0;
        } else {
          // calculate if there's advantage still
          if (foul.foulVictim->GetTeam()->GetFadingTeamPossessionAmount() <
              1.0) {
            DO_VALIDATION;
            foul.advantage = false;
          }
        }
      }
    }
  }

  if (foul.foulType != 0 && foul.advantage == false && !foul.hasBeenProcessed) {
    DO_VALIDATION;

    match->StopPlay();
    if (!penalty) {
      DO_VALIDATION;
      buffer.desiredSetPiece = e_GameMode_FreeKick;
      buffer.stopTime = match->GetActualTime_ms();
      buffer.prepareTime = match->GetActualTime_ms() + 2000;
      if (!animations) {
        match->BumpActualTime_ms(1900);
      }
      if (foul.foulType >= 2) {
        buffer.prepareTime += 10000;
        if (!animations) {
          match->BumpActualTime_ms(10000);
        }

      }
      buffer.startTime = buffer.prepareTime + 2000;
      buffer.restartPos = foul.foulPosition;
    } else {
      buffer.desiredSetPiece = e_GameMode_Penalty;
      buffer.stopTime = match->GetActualTime_ms();
      buffer.prepareTime = match->GetActualTime_ms() + 2000;
      if (!animations) {
        match->BumpActualTime_ms(1900);
      }
      if (foul.foulType >= 2) {
        buffer.prepareTime += 10000;
        if (!animations) {
          match->BumpActualTime_ms(10000);
        }
      }
      buffer.startTime = buffer.prepareTime + 2000;
      buffer.restartPos = Vector3(
          (pitchHalfW - 11.0) * foul.foulPlayer->GetTeam()->GetStaticSide(), 0,
          0);
    }
    buffer.teamID = foul.foulVictim->GetTeam()->GetID();
    buffer.active = true;
    std::string spamMessage = "foul!";
    if (foul.foulType == 2) {
      DO_VALIDATION;
      spamMessage.append(" yellow card");
      foul.foulPlayer->GiveYellowCard(match->GetActualTime_ms() + 6000); // need to find out proper moment
    }
    if (foul.foulType == 3) {
      DO_VALIDATION;
      spamMessage.append(" red card!!!");
      foul.foulPlayer->GiveRedCard(match->GetActualTime_ms() + 6000); // need to find out proper moment
    }
    match->SpamMessage(spamMessage);

    foul.hasBeenProcessed = true;

    return true;
  }

  return false;
}
