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

#include "teamAIcontroller.hpp"

#include <cmath>

#include "AIsupport/AIfunctions.hpp"

#include "team.hpp"
#include "match.hpp"

#include "../misc/hungarian.h"

#include "../main.hpp"

bool ReverseSortTacticalOpponentInfo(const TacticalOpponentInfo &a, const TacticalOpponentInfo &b) {
  return a.dangerFactor > b.dangerFactor;
}

TeamAIController::TeamAIController(Team *team) : team(team) {
  match = team->GetMatch();
  taker = 0;

  depth = 0.45f;
  width = 0.95f;

  setPieceType = e_GameMode_Normal;

  offsideTrapX = 0;

  endApplyAttackingRun_ms = 0;
  attackingRunPlayer = 0;
  endApplyTeamPressure_ms = 0;
  teamPressurePlayer = 0;
  endApplyKeeperRush_ms = 0;
  forwardSupportPlayer = 0;

  baseTeamTactics.Set("position_offense_depth_factor", 0.9f);
  baseTeamTactics.Set("position_defense_depth_factor", 0.75f);
  baseTeamTactics.Set("position_offense_width_factor", 0.9f);
  baseTeamTactics.Set("position_defense_width_factor", 0.8f);
  baseTeamTactics.Set("position_offense_ownhalf_factor", 0.52f);
  baseTeamTactics.Set("position_defense_ownhalf_factor", 0.54f);
  baseTeamTactics.Set("position_offense_midfieldfocus", 0.6f);
  baseTeamTactics.Set("position_defense_midfieldfocus", 0.5f);
  baseTeamTactics.Set("position_offense_midfieldfocus_strength", 0.35f);
  baseTeamTactics.Set("position_defense_midfieldfocus_strength", 0.35f);
  baseTeamTactics.Set("position_offense_sidefocus_strength", 0.1f); // take possession factor more seriously (high) or ball position (low)
  baseTeamTactics.Set("position_defense_sidefocus_strength", 0.4f);
  baseTeamTactics.Set("position_offense_microfocus_strength", 0.7f);
  baseTeamTactics.Set("position_defense_microfocus_strength", 0.8f);

  // how much [-1 * this value .. +1 * this value] offset we can add to the above tactics.
  teamTacticsModMultipliers.Set("position_offense_depth_factor", 0.1f);
  teamTacticsModMultipliers.Set("position_defense_depth_factor", 0.1f);
  teamTacticsModMultipliers.Set("position_offense_width_factor", 0.1f);
  teamTacticsModMultipliers.Set("position_defense_width_factor", 0.1f);
  teamTacticsModMultipliers.Set("position_offense_midfieldfocus", 0.3f);
  teamTacticsModMultipliers.Set("position_defense_midfieldfocus", 0.3f);
  teamTacticsModMultipliers.Set("position_offense_sidefocus_strength", 0.1f);
  teamTacticsModMultipliers.Set("position_defense_sidefocus_strength", 0.1f);
  teamTacticsModMultipliers.Set("position_offense_microfocus_strength", 0.15f);
  teamTacticsModMultipliers.Set("position_defense_microfocus_strength", 0.15f);

  offensivenessBias = 0.5f;

  UpdateTactics();
}

TeamAIController::~TeamAIController() {
}

Player *SelectAttackingRunPlayer(Team *team) {
  Player *possessionPlayer = team->GetDesignatedTeamPossessionPlayer();

  Vector3 offenseFocusPos = possessionPlayer->GetPosition() + Vector3(-team->GetSide() * 26.0f, 0, 0);

  Player *attackingRunPlayer = AI_GetClosestPlayer(team, offenseFocusPos, true, possessionPlayer);
  return attackingRunPlayer;
}

void TeamAIController::Process() {

  if (match->GetActualTime_ms() % 1000 == 0) UpdateTactics();

  CalculateSituation();

  float startDistance = 30.0f + 20.0f * offensivenessBias; // distance from goal where we start holding the opponents (but are likely to fallback somewhat)
  float forceDistance = 6.0f; // minimum distance from goal - try to 'hold' the opp there

  float deepestDanger = (pitchHalfW - startDistance) * team->GetSide();

  // ball as max
  float adaptedBallX = match->GetBall()->Predict(0).coords[0];
  // when far away from our goal (startDistance), drop back more easily. closer to forceDistance, don't buckle.
  adaptedBallX *= team->GetSide(); // > 0 == on our half (easier to work with)
  float offsetX = 20.0f + 10.0f * (1.0f - offensivenessBias); // when ball is this distance away from startDistance, we start falling back more towards forceDistance.
  float startToForcedBias = NormalizedClamp(adaptedBallX, pitchHalfW - startDistance - offsetX, pitchHalfW - forceDistance); // where, between startDistance and forceDistance, is the ball? 0 .. 1
  adaptedBallX += offsetX * (1.0f - startToForcedBias); // the fall back intensity gradually diminishes when getting closer to forceDistance
  adaptedBallX *= team->GetSide(); // back to absolute space
  if (adaptedBallX * team->GetSide() > deepestDanger * team->GetSide()) deepestDanger = adaptedBallX;

  // ballfuture as max
  if (match->GetBall()->Predict(700).coords[0] * team->GetSide() > deepestDanger * team->GetSide()) deepestDanger = match->GetBall()->Predict(700).coords[0];

  // opp as max
  Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();
  float cautionDistance = 4.0f * team->GetSide();
  if ((opp->GetPosition().coords[0] + opp->GetMovement().coords[0] * 0.15f + cautionDistance) * team->GetSide() > deepestDanger * team->GetSide()) deepestDanger = opp->GetPosition().coords[0] + opp->GetMovement().coords[0] * 0.1f + cautionDistance;

  // slacking teammate as max
  float lineX = AI_GetOffsideLine(match, match->GetMentalImage(0), abs(team->GetID()));
  float allowSlackDistance = 4.0f; // despite teammates slacking behind line this much, just hold the line
  if (lineX * team->GetSide() - allowSlackDistance > deepestDanger * team->GetSide()) {
    //printf("previous: %f\n", deepestDanger);
    deepestDanger = lineX - allowSlackDistance * team->GetSide();
    //printf("now:      %f\n", deepestDanger);
  }

  /*
  if (team->GetID() == 0) SetRedDebugPilon(Vector3(deepestDanger, 0, 0));
  if (team->GetID() == 1) SetYellowDebugPilon(Vector3(deepestDanger, 0, 0));
  */

  offsideTrapX = deepestDanger;
  // disable
  //offsideTrapX = pitchHalfW * team->GetSide();


  // calculate who's dangerous

  std::vector<Player*> players;
  match->GetActiveTeamPlayers(abs(team->GetID() - 1), players);

  Vector3 mostDangerousPos = Vector3((pitchHalfW - 2.0) * team->GetSide(), 0, 0);
  mostDangerousPos = mostDangerousPos * 0.8f + match->GetBall()->Predict(100).Get2D() * 0.2f;

  tacticalOpponentInfo.clear();

  for (unsigned int i = 0; i < players.size(); i++) {
    TacticalOpponentInfo info;
    info.player = players[i];

    info.dangerFactor = 1.0 - NormalizedClamp((players[i]->GetPosition() - mostDangerousPos).GetLength(), 0, pitchHalfW * 2);

    // player on ball is most dangerous
    info.dangerFactor *= 0.95f;
    if (players[i] == match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer()) info.dangerFactor += 0.05f;

    tacticalOpponentInfo.push_back(info);
  }

  // sort list
  std::sort(tacticalOpponentInfo.begin(), tacticalOpponentInfo.end(), ReverseSortTacticalOpponentInfo);


  // team pressure

/* DISABLED, interferes with other defense AI code for now
  if (team->GetHumanGamerCount() == 0) {
    if (match->GetBestPossessionTeamID() != team->GetID()) {

      bool opponentFreeRun = false;
      Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetDesignatedTeamPossessionPlayer();
      Vector3 dangerPos = Vector3(pitchHalfW * team->GetSide(), opp->GetPosition().coords[1] * 0.5, 0);
      float oppDangerDistance = (opp->GetPosition() - dangerPos).GetLength();
      Vector3 adaptedGoalPos = Vector3(pitchHalfW * team->GetSide(), 0, 0) * 0.5 + opp->GetPosition() * 0.5;
      float oppGoalDistance = (opp->GetPosition() - adaptedGoalPos).GetLength();
      Player *p = AI_GetClosestPlayer(team, adaptedGoalPos, false);
      float usGoalDistance = (p->GetPosition() - adaptedGoalPos).GetLength();
      if (usGoalDistance > oppGoalDistance - 3.2) opponentFreeRun = true;

      bool closeEnemy = false;
      if (oppDangerDistance < 20) closeEnemy = true;

      if (opponentFreeRun || closeEnemy) {
        ApplyTeamPressure();
      }
    }
  }
*/


  // trigger attacking runs

  if (match->GetActualTime_ms() % 500 == 0 && endApplyAttackingRun_ms <= match->GetActualTime_ms()) {
    if (team->GetHumanGamerCount() < 2) { // with >= 2 human players, one can do the running manually
      if (match->GetBestPossessionTeam() == team) {

        float neededRating = 0.5f;

        // from a certain distance, running is not very useful (can't pass that far)
        Player *runner = SelectAttackingRunPlayer(team);
        if (runner) {
          float distance = (runner->GetPosition() - team->GetDesignatedTeamPossessionPlayer()->GetPosition()).GetLength();
          float distanceRating =
              std::pow(1.0f - NormalizedClamp(distance, 0, 40), 0.5f);

          // more likely to run when there's less defenders in front
          std::vector<Player*> opponents;
          Vector3 spot = runner->GetPosition() * Vector3(1.0f, 0.8f, 0.0f) + Vector3(team->GetSide() * 10.0f, 0, 0);
          AI_GetClosestPlayers(team->GetMatch()->GetTeam(abs(team->GetID() - 1)), spot, false, opponents, 4);
          float oppDensityRating = 1.0f;
          for (unsigned int i = 0; i < opponents.size(); i++) {
            float oppDistance = (opponents[i]->GetPosition() - spot).GetLength();
            float oppDistanceRatingInv = std::pow(
                curve(1.0f - NormalizedClamp(oppDistance, 0, 15), 1.0f), 0.5f);
            oppDensityRating -= oppDistanceRatingInv * 0.3f; // subtractive!
          }

          float runConditionsRating = distanceRating * oppDensityRating;

          if (runConditionsRating >= neededRating) {
            ApplyAttackingRun();
          }
        }

      }
    }
  }

  if (match->GetActualTime_ms() % 1500 == 0) {
    forwardSupportPlayer = AI_GetClosestPlayer(team, team->GetDesignatedTeamPossessionPlayer()->GetPosition() * Vector3(1.0f, 1.0f, 0.0f) + Vector3(-team->GetSide() * 1.5f, 0, 0), false, team->GetDesignatedTeamPossessionPlayer());
  }

}

float mixup(float base, const std::string &varname, e_PlayerRole role) {

  float value = -100.0f;

  if (role == e_PlayerRole_CB) {
    if (varname.compare("position_offense_width_factor") == 0) value = 0.2f; // wider defense
  }

  if (role == e_PlayerRole_LB || role == e_PlayerRole_RB) {
    if (varname.compare("position_defense_ownhalf_factor") == 0) value = -0.075f; // go forward
    if (varname.compare("position_offense_width_factor") == 0) value = 0.2f; // wider defense
    if (varname.compare("position_offense_ownhalf_factor") == 0) value = -0.1f; // go forward
  }

  if (role == e_PlayerRole_LM || role == e_PlayerRole_RM) {
    // wingers stay high up to offer counter-attack support
    if (varname.compare("position_defense_ownhalf_factor") == 0) value = -0.05f;
    if (varname.compare("position_offense_ownhalf_factor") == 0) value = -0.1f; // go forward
  }

  if (role == e_PlayerRole_AM) {
    // attackers stay high up to offer counter-attack support
    if (varname.compare("position_defense_depth_factor") == 0) value = 0.125f;
  }

  if (role == e_PlayerRole_CF) {
    // strikers stay high up to offer counter-attack support
    if (varname.compare("position_defense_depth_factor") == 0) value = 0.125f;
  }

  if (value > -100.0f) {

    // bias version
    //float rolebias = 0.5f;
    //return clamp(value * rolebias + base * (1.0f - rolebias), 0.0f, 1.0f);

    // offset version
    return clamp(base + value, 0.0f, 1.0f);

  } else {

    return base;

  }

}

Vector3 TeamAIController::GetAdaptedFormationPosition(Player *player, bool useDynamicFormationPosition) {

  bool toggle_yFocus = true;
  bool toggle_microFocus = true;
  bool toggle_midfieldFocus = true;

  e_PlayerRole role;
  if (useDynamicFormationPosition) role = player->GetDynamicFormationEntry().role;
                              else role = player->GetFormationEntry().role;

  Vector3 focalPoint = match->GetDesignatedPossessionPlayer()->GetPosition();
  float urgencyBias = 1.0f - NormalizedClamp((focalPoint - player->GetPosition()).GetLength(), 2.0f, 30.0f);
  float ballX = match->GetBall()->GetAveragePosition(3500 * (1.0f - urgencyBias * 0.7f)).coords[0];
  float ballY = match->GetBall()->GetAveragePosition(4000 * (1.0f - urgencyBias * 0.5f)).coords[1];

  float offense_depthFactor           = mixup( liveTeamTactics.GetReal("position_offense_depth_factor"),        "position_offense_depth_factor", role);
  float defense_depthFactor           = mixup( liveTeamTactics.GetReal("position_defense_depth_factor"),        "position_defense_depth_factor", role);
  float offense_widthFactor           = mixup( liveTeamTactics.GetReal("position_offense_width_factor"),        "position_offense_width_factor", role);
  float defense_widthFactor           = mixup( liveTeamTactics.GetReal("position_defense_width_factor"),        "position_defense_width_factor", role);
  float offense_ownHalfFactor         = mixup( liveTeamTactics.GetReal("position_offense_ownhalf_factor"),      "position_offense_ownhalf_factor", role);
  float defense_ownHalfFactor         = mixup( liveTeamTactics.GetReal("position_defense_ownhalf_factor"),      "position_defense_ownhalf_factor", role);
  float offense_midfieldFocus         = mixup( liveTeamTactics.GetReal("position_offense_midfieldfocus"),       "position_offense_midfieldfocus", role);
  float defense_midfieldFocus         = mixup( liveTeamTactics.GetReal("position_defense_midfieldfocus"),       "position_defense_midfieldfocus", role);
  float offense_midfieldFocusStrength = mixup( liveTeamTactics.GetReal("position_offense_midfieldfocus_strength"), "position_offense_midfieldfocus_strength", role);
  float defense_midfieldFocusStrength = mixup( liveTeamTactics.GetReal("position_defense_midfieldfocus_strength"), "position_defense_midfieldfocus_strength", role);
  float offense_sideFocusStrength     = mixup( liveTeamTactics.GetReal("position_offense_sidefocus_strength"),  "position_offense_sidefocus_strength", role);
  float defense_sideFocusStrength     = mixup( liveTeamTactics.GetReal("position_defense_sidefocus_strength"),  "position_defense_sidefocus_strength", role);
  float offense_microFocusStrength    = mixup( liveTeamTactics.GetReal("position_offense_microfocus_strength"), "position_offense_microfocus_strength", role);
  float defense_microFocusStrength    = mixup( liveTeamTactics.GetReal("position_defense_microfocus_strength"), "position_defense_microfocus_strength", role);

  offense_sideFocusStrength += (-0.5 + AI_GetMindSet(role)) * 0.2f;
  defense_sideFocusStrength += ( 0.5 - AI_GetMindSet(role)) * 0.2f;
  offense_sideFocusStrength = clamp(offense_sideFocusStrength + -0.3f + (       offensivenessBias) * 0.3f, 0.0f, 1.0f);
  defense_sideFocusStrength = clamp(defense_sideFocusStrength + -0.3f + (1.0f - offensivenessBias) * 0.3f, 0.0f, 1.0f);

  float possessionAmountBias = NormalizedClamp(fadingTeamPossessionAmount - 0.5f, 0.3f, 0.7f);
  // also take the ball's position as part of the possessionBias equation
  float ballBias = NormalizedClamp(((ballX / pitchHalfW) * -team->GetSide()), -0.7f, 0.7f); // 0 == own half, 1 == opponent half
  // if possessionBias is unclear (near 0.5), take ballBias more seriously as indicator of possession.
  float ballBiasBias = 1.0f - fabs(possessionAmountBias * 2.0f - 1.0f); // biasception
  ballBiasBias *= 0.6f; // don't take ballBias too serious - we need defenders to somewhat keep watch when possession team is unclear, even when the ball is forward
  float possessionBias = possessionAmountBias * (1.0f - ballBiasBias) +
                         ballBias * ballBiasBias;

  possessionBias = clamp(possessionBias + (offensivenessBias - 0.5f) * 0.3f, 0.0f, 1.0f);

  // offense can be a bit more relaxed
  focalPoint = (match->GetBall()->GetAveragePosition(3000).Get2D() * 1.0f + focalPoint * 0.0f) * possessionBias +
               (match->GetBall()->GetAveragePosition(2000).Get2D() * 0.5f + focalPoint * 0.5f) * (1.0f - possessionBias);

  float adaptedDepth = depth * (offense_depthFactor * possessionBias +
                                defense_depthFactor * (1.0f - possessionBias));
  float adaptedWidth = width * (offense_widthFactor * possessionBias +
                                defense_widthFactor * (1.0f - possessionBias));

  float offsetX = pitchHalfW * team->GetSide() * ((offense_ownHalfFactor * 2.0f - 1.0f) * possessionBias +
                                                  (defense_ownHalfFactor * 2.0f - 1.0f) * (1.0f - possessionBias));

  float sideFocusStrength = (offense_sideFocusStrength * possessionBias +
                             defense_sideFocusStrength * (1.0f - possessionBias));

  float sideFocus = possessionBias * 2.0f - 1.0f; // -1 == own side, 1 == opp side

  float sideX = 0.2f * sideFocus * -team->GetSide() * pitchHalfW +
                0.8f * -match->GetAveragePossessionSide(6000) * pitchHalfW;
  // exaggerate: sideX = clamp(sideX * 2.0f, -pitchHalfW, pitchHalfW);
  float centerX = clamp((ballX * (1.0f - sideFocusStrength) + sideX * sideFocusStrength) + offsetX, -pitchHalfW, pitchHalfW);

  // center of width
  float centerY = clamp(ballY, -pitchHalfH, pitchHalfH);

  // leave space for actual depth (50% depth on both sides of center)
  float adaptCenterToFitDepthBias = 0.95f; // the lower this value, the more players will move with the centerX, and be clamped to the edges of the pitch
  float adaptCenterToFitWidthBias = 0.9f; // the lower this value, the more players will move with the centerY, and be clamped to the edges of the pitch
  centerX *= ( (1.0f - adaptedDepth) / 1.0f ) * adaptCenterToFitDepthBias + (1.0f - adaptCenterToFitDepthBias);
  centerY *= ( (1.0f - adaptedWidth) / 1.0f ) * adaptCenterToFitWidthBias + (1.0f - adaptCenterToFitWidthBias);

  float backXBound = centerX - adaptedDepth * pitchHalfW * -team->GetSide();
  float frontXBound = centerX + adaptedDepth * pitchHalfW * -team->GetSide();
  float lowYBound = centerY - adaptedWidth * pitchHalfH;
  float highYBound = centerY + adaptedWidth * pitchHalfH;

  // if (player->GetTeam()->GetID() == 0) {
  //   SetGreenDebugPilon(Vector3(backXBound, 0, 0));
  //   SetBlueDebugPilon(Vector3(frontXBound, 0, 0));
  // }

  // maybe setting offside trap this way doesn't work too well: after all, defensive positioning stuff is done after this, thereby diminishing trap sometimes.
  // on the other hand, maybe this is a good basic trap setup, and we could apply the trap with the applyoffsidetrap function after the defensive positioning as well. (done from the def/mid strategies code)
  if (backXBound * team->GetSide() > GetOffsideTrapX() * team->GetSide()) backXBound = GetOffsideTrapX();

  float xFocus = 0.0f;
  float xFocusStrength = 0.0f;
  float yFocus = ballY * 1.0f;
  float yFocusStrength = 0.5f * possessionBias +
                         0.2f * (1.0f - possessionBias);

  float defendFactor = (1.0f - AI_GetMindSet(role)) * (1.0f - possessionBias);
  Vector3 microFocus = focalPoint;// * Vector3(1.0f, 0.8f + (1.0f - defendFactor) * 0.2f, 0.0f); // defenders stay in the middle more
  Vector3 defensiveFocusPos = Vector3(clamp(microFocus.coords[0] + team->GetSide() * 2.0f, -pitchHalfW, backXBound * team->GetSide()), microFocus.coords[1] * 0.9f, 0);
  microFocus = (Vector3(clamp(microFocus.coords[0] - team->GetSide() * 1.0f, -pitchHalfW, pitchHalfW), microFocus.coords[1] * 0.9f, 0)) * possessionBias +
               (defensiveFocusPos) * (1.0f - possessionBias);
  float microFocusStrength = (offense_microFocusStrength * possessionBias +
                              defense_microFocusStrength * (1.0f - possessionBias));

  // on own half in possession: little microfocus strength. on own half not in possession: lots of microfocus. other half: the other way around.
  float microFocusSideBias = NormalizedClamp((ballX / pitchHalfW) * -team->GetSide(), -0.7f, 0.7f); // ball on own half == lower values
  microFocusSideBias = microFocusSideBias * 0.7f + 0.3f;
  float autoMicroFocusStrength =
      (std::pow(microFocusSideBias, 0.8f) * possessionBias +
       (std::pow(1.0f - microFocusSideBias, 0.6f)) * (1.0f - possessionBias));
  microFocusStrength = microFocusStrength * (0.2f + 0.8f * autoMicroFocusStrength);
  //if (team->GetID() == 0) printf("microfocusstrength: %f\n", microFocusStrength);

  // midfield focus. higher == more on opponent's half
  float manualMidfieldFocus = (offense_midfieldFocus * possessionBias +
                               defense_midfieldFocus * (1.0f - possessionBias));
  float autoMidfieldFocus = NormalizedClamp((ballX / pitchHalfW) * -team->GetSide(), -0.8f, 0.8f); // midfield follows ball
  //autoMidfieldFocus *= 0.5f + 0.5f * possessionBias; // auto defensive stance on no possession (deemed too defensive)
  float midfieldFocus = manualMidfieldFocus * 0.7f + autoMidfieldFocus * 0.3f;
  float midfieldFocusStrength = (offense_midfieldFocusStrength * possessionBias +
                                 defense_midfieldFocusStrength * (1.0f - possessionBias));
  if (!toggle_yFocus) yFocusStrength = 0.0f;
  if (!toggle_microFocus) microFocusStrength = 0.0f;
  if (!toggle_midfieldFocus) midfieldFocusStrength = 0.0f;

  Vector3 desiredPos = AI_GetAdaptedFormationPosition(team->GetMatch(), player, backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, microFocus, microFocusStrength, midfieldFocus, midfieldFocusStrength, useDynamicFormationPosition);

  desiredPos.coords[0] = clamp(desiredPos.coords[0], -pitchHalfW, pitchHalfW);
  desiredPos.coords[1] = clamp(desiredPos.coords[1], -pitchHalfH, pitchHalfH);

  return desiredPos;
}

void TeamAIController::CalculateDynamicRoles() {

  std::vector<Player*> players;
  team->GetActivePlayers(players);

  std::vector<Player*>::iterator iter = players.begin();
  while (iter != players.end()) {
    if ((*iter)->GetFormationEntry().role == e_PlayerRole_GK) {
      players.erase(iter);
      break;
    }
  }

  unsigned int playerNum = players.size();

  // collect adapted formation positions
  std::vector<Vector3> adaptedFormationPositions;
  for (unsigned int y = 0; y < playerNum; y++) {
    adaptedFormationPositions.push_back(GetAdaptedFormationPosition(players.at(y), false));
  }

  // first make a sorted list on all possible distances between players and formation targets
  std::vector<int> distances;
  for (unsigned int x = 0; x < playerNum; x++) {
    for (unsigned int y = 0; y < playerNum; y++) {
      const Vector3 &playerPos = players.at(x)->GetPosition() + players.at(x)->GetMovement() * 0.5;
      const Vector3 &formationPos = adaptedFormationPositions.at(y);
      float distance = (playerPos - formationPos).GetLength();
      distances.push_back(int(std::round(distance * 10)));
    }
  }

  std::sort(distances.begin(), distances.end());

  for (unsigned int i = playerNum; i < distances.size(); i += 5) {

    // libhungarian by Cyrill Stachniss, 2004

    hungarian_problem_t p;

    int r[playerNum * playerNum];

    for (unsigned int x = 0; x < playerNum; x++) {
      for (unsigned int y = 0; y < playerNum; y++) {
        const Vector3 &playerPos = players.at(x)->GetPosition() + players.at(x)->GetMovement() * 0.5;
        const Vector3 &formationPos = adaptedFormationPositions.at(y);
        float cost = (playerPos - formationPos).GetLength();
        int intCost = int(std::round(cost * 10));
        if (intCost >= distances[i]) intCost = 50000;
        r[x + y * playerNum] = intCost;
      }
    }

    int** m = array_to_matrix(r, playerNum, playerNum);

    /* initialize the hungarian_problem using the cost matrix*/
    int matrix_size = hungarian_init(&p, m, playerNum, playerNum, HUNGARIAN_MODE_MINIMIZE_COST);

    //fprintf(stderr, "assignment matrix has %d rows and %d columns.\n\n", matrix_size, matrix_size);

    /* some output */
    //fprintf(stderr, "cost-matrix:");
    //hungarian_print_costmatrix(&p);

    /* solve the assignement problem */
    int totalCost = hungarian_solve(&p);

    /* some output */
    //fprintf(stderr, "assignment:");
    //hungarian_print_assignment(&p);

    bool ready = false;
    if (totalCost != -1 && (totalCost < 50000 || i >= distances.size() - 5)) {
      //printf("total cost: %i\n", totalCost);
      // assign dynamic role with best cost
      for (unsigned int x = 0; x < playerNum; x++) {
        for (unsigned int y = 0; y < playerNum; y++) {
          if ((&p)->assignment[y][x] == 1) {
            FormationEntry formationEntry = players.at(y)->GetFormationEntry();
            players.at(x)->SetDynamicFormationEntry(formationEntry);
          }
        }
      }

      ready = true;
    }

    /* free used memory */
    hungarian_free(&p);
    for (unsigned int blah = 0; blah < playerNum; blah++) {
      free(m[blah]);
    }
    free(m);

    if (ready) break;
  }

}

float TeamAIController::CalculateMarkingQuality(Player *player, Player *opp) {



  Vector3 oppPosition = opp->GetPosition() + opp->GetMovement() * 0.1f;
  Vector3 playerPosition = player->GetPosition() + player->GetMovement() * 0.1f;

  // draw virtual line from 'left' to 'right' of player, perpendicular to the goal. anything 'above' this line he can catch up with. well,
  // not if it's too much to the side of this line / closer to goal; there just isn't enough time then to catch up.

  Vector3 goalPos = Vector3(pitchHalfW * team->GetSide(), 0, 0);
  Vector3 toGoal = goalPos - playerPosition;
  float lineLength = clamp((toGoal).GetLength(), 4.0f, 14.0f);
  Line line;
  Vector3 toGoalNorm = toGoal.GetNormalized(Vector3(team->GetSide(), 0, 0));
  Vector3 safetyVec = -toGoalNorm * 0.5f;
  line.SetVertex(0, playerPosition + safetyVec + toGoalNorm.GetRotated2D(-0.5 * pi) * lineLength); // left of player (seen from goal)
  line.SetVertex(1, playerPosition + safetyVec + toGoalNorm.GetRotated2D( 0.5 * pi) * lineLength); // right of player (seen from goal)

  bool oppIsOnRightSideOfLine = line.WhatSide(oppPosition); // notice the descriptive variable name
  float u = 0.0f; // 'position' on line, v0 == 0 .. v1 == 1
  float oppFromLineDistance = line.GetDistanceToPoint(oppPosition, u);

  float adaptedOppFromLineDistance = oppFromLineDistance;
  if (oppIsOnRightSideOfLine) adaptedOppFromLineDistance = fabs(oppFromLineDistance - 2.0f); // we put the 'best spot' a bit further away from the line

  volatile float oppFromLineDistanceFactor = pow(NormalizedClamp(adaptedOppFromLineDistance, 0.0f, 60.0f), 0.5f);
  float oppOnLineDistanceFactor = pow(clamp( fabs(u * 2.0f - 1.0f) , 0.0f, 1.0f), 0.5f);

  float result = 1.0f;

  // opponent further away from the line == bad (well.. not bad.. but not very useful either. probably, another team mate is there, if not, can always catch up when opp gets closer)
  result -= oppFromLineDistanceFactor * 0.5f;
  // opponent further away on the line == bad
  result -= oppOnLineDistanceFactor * 0.5f;

  result = clamp(result, 0.0f, 1.0f);

  // argh, he's passed us already!
  if (!oppIsOnRightSideOfLine) result *= 0.6f;

  // now add a bit of good old fashioned distancerating so that there'll still be some definition if result is now 0.0f
  float oppDistance = 1.0 - NormalizedClamp((playerPosition - oppPosition).GetLength(), 0.0, pitchHalfW * 2.0f);
  result = result * 0.8f + oppDistance * 0.2f;
  return result;
}

void TeamAIController::CalculateManMarking() {

  // new method

  int numMarkedOpponents = 3;//std::min(match->GetTeam(0)->GetActivePlayerCount(), match->GetTeam(1)->GetActivePlayerCount());

  const std::vector<TacticalOpponentInfo> &oppInfo = GetTacticalOpponentInfo();

  std::vector<Player*> players;
  team->GetActivePlayers(players);

  // reset previous man marking
  for (unsigned int i = 0; i < players.size(); i++) {
    players[i]->SetManMarkingID(-1);
  }

  // most dangerous opponent gets closest player to cover him, and so on
  // (oppInfo is already sorted, most dangerous first. this is done in this->process() which is called earlier from team->process())
  for (unsigned int opp = 0; opp < (unsigned int)std::min((signed int)oppInfo.size(), numMarkedOpponents); opp++) {

    Player *closestPlayer = 0;
    float bestMarkingQuality = -1.0f;
    std::vector<Player*>::iterator closestPlayerIter;
    std::vector<Player*>::iterator iter = players.begin();

    Player *oppPlayer = oppInfo.at(opp).player;

    // find closest player for this opponent
    while (iter != players.end()) {

      if ((*iter)->GetFormationEntry().role != e_PlayerRole_GK) {

        float markingQuality = CalculateMarkingQuality((*iter), oppPlayer);

        if (markingQuality > bestMarkingQuality) {
          closestPlayer = *iter;
          bestMarkingQuality = markingQuality;
          closestPlayerIter = iter;
        }
      }

      iter++;
    }

    if (closestPlayer) {
      //printf("oppID: %i\n", oppPlayer->GetID());
      closestPlayer->SetManMarkingID(oppPlayer->GetID());
      players.erase(closestPlayerIter);
    }

    if (players.size() == 0) break;
  }
  //printf("\n");
}

void TeamAIController::ApplyOffsideTrap(Vector3 &position) const {

  // binary 'hard clamp' method
  //if (position.coords[0] * team->GetSide() > offsideTrapX * team->GetSide()) position.coords[0] = offsideTrapX;

  // smooth version

  // area, centered around offsideTrapX, that will be compressed.
  // so the area from [offsideTrapX - areaLength .. offsideTrapX + areaLength] will be compressed into [offsideTrapX .. offsideTrapX + areaLength]
  float areaHalfLength = 2.0f;//4.0f;
  float absPosX = position.coords[0] * team->GetSide();
  float absOffsideTrapX = offsideTrapX * team->GetSide();

  if (absPosX > absOffsideTrapX - areaHalfLength) {

    float areaFront = absOffsideTrapX - areaHalfLength;
    float posFromAreaFront = absPosX - areaFront;
    float posFactor = posFromAreaFront / (areaHalfLength * 2.0f);
    posFactor = clamp(posFactor, 0.0f, 1.0f); // 0.0f == most forward, 1.0f == deepest players
    //posFactor = pow(posFactor, 0.5f); // don't impact players on the non-offside side of the offsideline as much as the other way around

    float absResultPosX = areaFront + areaHalfLength * posFactor; // this compresses the 2 * areaHalfLength into 1 * areaHalfLength

    position.coords[0] = absResultPosX * team->GetSide();
  }

}

void TeamAIController::PrepareSetPiece(e_GameMode setPiece, Team* other_team,
    int kickoffTakerTeamId, int takerTeamID) {
  setPieceType = setPiece;

  if (takerTeamID == -1) assert(setPieceType == e_GameMode_Normal);
  if (setPieceType == e_GameMode_Normal) return;
  std::vector<Player*> players;
  team->GetActivePlayers(players);

  std::vector<Player*>::iterator iter = players.begin();
  while (iter != players.end()) {
    Vector3 focus = match->GetBall()->Predict(0).Get2D();
    if ((*iter)->GetFormationEntry().role == e_PlayerRole_GK) {
      if (setPiece == e_GameMode_KickOff) {
        focus.coords[0] = 0;
        focus.coords[1] = 0;
      }
      (*iter)->ResetPosition(Vector3(pitchHalfW * team->GetSide() * 0.98, 0, 0), focus);
      iter = players.erase(iter);
    } else {
      iter++;
    }
  }

  Match *match = team->GetMatch();

  bool isTakerTeam = takerTeamID == team->GetID() ? true : false;
  Team *takerTeam = isTakerTeam ? team : match->GetTeam(abs(team->GetID() - 1));

  if (isTakerTeam) team->SetFadingTeamPossessionAmount(1.5);
              else team->SetFadingTeamPossessionAmount(0.5);

  switch (setPiece) {

    case e_GameMode_KickOff:
      for (unsigned int i = 0; i < players.size(); i++) {
        Vector3 basePos = players[i]->GetFormationEntry().position * Vector3(-team->GetSide() * pitchHalfW * 0.6, -team->GetSide() * pitchHalfH * 0.6, 0);
        basePos.coords[1] += random(-2.0f, 2.0f); // to stop people from bumping into each other and such
        basePos.coords[0] *= 0.5;
        basePos.coords[0] += (pitchHalfW * 0.2) * team->GetSide();
        if (basePos.coords[0] * team->GetSide() < 0.5) basePos.coords[0] = 0.5 * team->GetSide(); // not allowed to stand on opp side
        if (basePos.GetLength() < 9.4) { // not allowed to stand in center spot
          basePos.Normalize(Vector3(team->GetSide(), 0, 0));
          basePos *= 9.4;
        }
        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());

        // supporting players
        if (isTakerTeam) {
          std::vector<Player*> result;
          AI_GetClosestPlayers(team, Vector3(0), false, result, 2);
          for (unsigned int i = 0; i < result.size(); i++) {
            result[i]->ResetPosition(Vector3(0, i * 1.4 * team->GetSide(), 0), match->GetBall()->Predict(0).Get2D());
          }
        }
      }
    break;

    case e_GameMode_GoalKick:
      for (unsigned int i = 0; i < players.size(); i++) {
        float backXBound, frontXBound, lowYBound, highYBound;
        if (isTakerTeam) {
          backXBound = team->GetSide() * pitchHalfW * 0.5;
          frontXBound = -team->GetSide() * pitchHalfW * 0.2;
        } else {
          backXBound = team->GetSide() * pitchHalfW * 0.4;
          frontXBound = -team->GetSide() * pitchHalfW * 0.1;
        }
        lowYBound = -pitchHalfH * 0.7;
        highYBound = pitchHalfH * 0.7;
        Vector3 basePos = AI_GetAdaptedFormationPosition(match, players[i], backXBound, frontXBound, lowYBound, highYBound, 0, 0, 0, 0, 0, 0, 0, 0, false);
        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
      break;

    case e_GameMode_Corner:
      for (unsigned int i = 0; i < players.size(); i++) {
        float backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, midfieldFocus, midfieldFocusStrength;
        Vector3 ballPos = match->GetBall()->Predict(0).Get2D();
        if (isTakerTeam) {
          backXBound = -team->GetSide() * pitchHalfW * 0.2;
          frontXBound = -team->GetSide() * pitchHalfW * 0.96;
          xFocus = frontXBound * 0.85;
          xFocusStrength = 0.7f;
          yFocus = ballPos.coords[1] * 0.1f;
          yFocusStrength = 0.7f;
          midfieldFocus = 0.9f;
          midfieldFocusStrength = 0.5f;
        } else {
          backXBound = team->GetSide() * pitchHalfW * 0.98;
          frontXBound = team->GetSide() * pitchHalfW * 0.5;
          xFocus = backXBound * 0.94;
          xFocusStrength = 0.8f;
          yFocus = ballPos.coords[1] * 0.1f;
          yFocusStrength = 0.9f;
          midfieldFocus = 0.1f;
          midfieldFocusStrength = 0.7f;
        }
        lowYBound = -pitchHalfH * 0.6;
        highYBound = pitchHalfH * 0.6;
        Vector3 basePos = AI_GetAdaptedFormationPosition(match, players[i], backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, Vector3(ballPos.coords[0] * 0.95f, ballPos.coords[1] * 0.1f, 0), 0.9, midfieldFocus, midfieldFocusStrength, false);
        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
      break;

    case e_GameMode_ThrowIn:
      for (unsigned int i = 0; i < players.size(); i++) {
        float backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength;
        Vector3 ballPos = match->GetBall()->Predict(0).Get2D();
        if (isTakerTeam) {
          backXBound = clamp(ballPos.coords[0] + 30 * team->GetSide(), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(ballPos.coords[0] + 20 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocus = clamp(ballPos.coords[0] + 4 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocusStrength = 0.4;
          yFocus = ballPos.coords[1] * 0.996;
          yFocusStrength = 0.6;
        } else {
          backXBound = clamp(ballPos.coords[0] + 30 * team->GetSide(), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(ballPos.coords[0] + 15 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocus = clamp(ballPos.coords[0] + 16 * team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocusStrength = 0.2;
          yFocus = ballPos.coords[1] * 0.95;
          yFocusStrength = 0.5;
        }
        lowYBound = -pitchHalfH * 0.75f + ballPos.coords[1] * 0.25f;
        highYBound = pitchHalfH * 0.75f + ballPos.coords[1] * 0.25f;
        Vector3 basePos = AI_GetAdaptedFormationPosition(match, players[i], backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, ballPos, 0.7, 0, 0, false);
        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
      break;

    case e_GameMode_FreeKick:
      for (unsigned int i = 0; i < players.size(); i++) {
        float backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength;
        Vector3 ballPos = match->GetBall()->Predict(0).Get2D();
        if (isTakerTeam) {
          float xOffset = clamp((ballPos.coords[0] * -team->GetSide()) / pitchHalfW, -1.0, 1.0) * 0.5 + 0.5; // 0 == close to our goal, 1 == far from our goal
          backXBound = clamp(team->GetSide() * pitchHalfW * (0.7 - xOffset * 0.7), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(team->GetSide() * pitchHalfW * (-0.3 - xOffset * 0.7), -pitchHalfW, pitchHalfW);
          //printf("fb: %f %f\n", backXBound, frontXBound);
          xFocus = clamp(ballPos.coords[0] + 10 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocusStrength = 0.5 + xOffset * 0.2;
          yFocus = ballPos.coords[1] * 0.4;
          yFocusStrength = 0.6 + xOffset * 0.2;
        } else {
          float xOffset = clamp((ballPos.coords[0] * -team->GetSide()) / pitchHalfW, -1.0, 1.0) * 0.5 + 0.5; // 0 == close to our goal, 1 == far from our goal
          //xOffset *= 40.0;
          backXBound = clamp(team->GetSide() * pitchHalfW * (1.0 - xOffset * 0.6), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(team->GetSide() * pitchHalfW * (0.5 - xOffset * 0.8), -pitchHalfW, pitchHalfW);
          xFocus = clamp(ballPos.coords[0] + 20 * team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocusStrength = 0.6 - xOffset * 0.4;
          yFocus = ballPos.coords[1] * 0.2;
          yFocusStrength = 0.8 - xOffset * 0.4;
        }
        lowYBound = -pitchHalfH * 0.7;
        highYBound = pitchHalfH * 0.7;
        Vector3 basePos = AI_GetAdaptedFormationPosition(match, players[i], backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, ballPos, 0.4, 0, 0, false);

        // keep distance
        if (!isTakerTeam) {
          if ((basePos - match->GetBall()->Predict(0).Get2D()).GetLength() < 9.15) {
            basePos = match->GetBall()->Predict(0).Get2D() + (basePos - match->GetBall()->Predict(0).Get2D()).GetNormalized() * 9.15;
          }
        }

        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }

      // wall
      if (!isTakerTeam && (match->GetBall()->Predict(0).Get2D() - Vector3(team->GetSide() * pitchHalfW, 0, 0)).GetLength() < 40.0) {
        std::vector<Player*> result;
        AI_GetClosestPlayers(team, match->GetBall()->Predict(0).Get2D(), false, result, 3);
        for (unsigned int i = 0; i < result.size(); i++) {
          Vector3 toGoal = (Vector3(team->GetSide() * pitchHalfW, 0, 0) - match->GetBall()->Predict(0).Get2D()).GetNormalized(0);
          toGoal += Vector3(0, 1.0 - i, 0) * 0.07;
          toGoal.Normalize();
          result[i]->ResetPosition(match->GetBall()->Predict(0).Get2D() + toGoal * 9.15f, match->GetBall()->Predict(0).Get2D());
        }
      }

      break;

    case e_GameMode_Penalty:
      for (unsigned int i = 0; i < players.size(); i++) {
        float backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength;
        Vector3 ballPos = match->GetBall()->Predict(0).Get2D();
        if (isTakerTeam) {
          backXBound = clamp(ballPos.coords[0] + 50 * team->GetSide(), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(ballPos.coords[0] + 10 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocus = ballPos.coords[0];
          xFocusStrength = 0.6;
          yFocus = 0.0;
          yFocusStrength = 0.8;
        } else {
          backXBound = clamp(ballPos.coords[0] + 11 * team->GetSide(), -pitchHalfW, pitchHalfW);
          frontXBound = clamp(ballPos.coords[0] + 20 * -team->GetSide(), -pitchHalfW, pitchHalfW);
          xFocus = ballPos.coords[0];
          xFocusStrength = 1.0;
          yFocus = 0.0;
          yFocusStrength = 1.0;
        }
        lowYBound = -pitchHalfH * 0.8;
        highYBound = pitchHalfH * 0.8;
        Vector3 basePos = AI_GetAdaptedFormationPosition(match, players[i], backXBound, frontXBound, lowYBound, highYBound, xFocus, xFocusStrength, yFocus, yFocusStrength, 0, 0, 0, 0, false);

        // outside the box
        signed int penaltySide = (match->GetBall()->Predict(0).coords[0] < 0) ? -1 : 1;
        if (basePos.coords[0] * penaltySide > pitchHalfW - 16.5 - 0.5) basePos.coords[0] = (pitchHalfW - 16.5 - 0.5) * penaltySide;

        // outside penalty arc as well
        if ((basePos - match->GetBall()->Predict(0).Get2D()).GetLength() < 9.15 + 0.5) {
          basePos = match->GetBall()->Predict(0).Get2D() + (basePos - match->GetBall()->Predict(0).Get2D()).GetNormalized() * (9.15 + 0.5);
        }

        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
      break;

    default:
      for (unsigned int i = 0; i < players.size(); i++) {
        Vector3 basePos = players[i]->GetFormationEntry().position * Vector3(-team->GetSide() * pitchHalfW * 0.7, -team->GetSide() * pitchHalfH * 0.7, 0);

        players[i]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
      break;

  }

  auto& config = GetGameConfig();
  if (setPiece == e_GameMode_KickOff) {
    auto formation_players = team->GetAllPlayers();
    auto players_to_position = team->GetAllPlayers();
    if (GetScenarioConfig().kickoff_for_goal_loosing_team &&
        match->GetMatchTime_ms() > 0 &&
        (GetScenarioConfig().LeftTeamOwnsBall() ^ kickoffTakerTeamId == 0)) {
      formation_players = other_team->GetAllPlayers();
    }
    assert(formation_players.size() == players_to_position.size());
    for (int x = 0; x < formation_players.size(); x++) {
      if (players_to_position[x]->IsActive()) {
        Vector3 basePos = formation_players[x]->GetFormationEntry().start_position *
            Vector3(-team->GetSide() * pitchHalfW, -team->GetSide() * pitchHalfH, 0);
        players_to_position[x]->ResetPosition(basePos, match->GetBall()->Predict(0).Get2D());
      }
    }
  }

  if (isTakerTeam) {
    auto ball_pos = match->GetBall()->Predict(0).Get2D();
    std::vector<Player*> players;
    AI_GetClosestPlayers(team, match->GetBall()->Predict(0).Get2D(), false, players, 2);
    taker = players[0];
    if (setPiece == e_GameMode_KickOff) {
      // Do nothing
    } else if (setPiece == e_GameMode_ThrowIn) {
      players[0]->ResetPosition(ball_pos + match->GetBall()->Predict(0).Get2D().GetNormalized(Vector3(0, -team->GetSide(), 0)) * 0.3f, ball_pos);
    } else if (setPiece == e_GameMode_FreeKick) {
      taker->ResetPosition(ball_pos + Vector3(team->GetSide(), 0, 0) * 2.3f, ball_pos);
    } else {
      taker->ResetPosition(ball_pos + match->GetBall()->Predict(0).Get2D().GetNormalized(Vector3(0, -team->GetSide(), 0)) * 2.3f, ball_pos);
    }
    if (setPiece == e_GameMode_ThrowIn) {
      taker->SelectRetainAnim();
    }
    if (setPiece == e_GameMode_Penalty) {
      taker->ResetPosition(ball_pos + Vector3(team->GetSide(), 0, 0) * 3.0, ball_pos);
    }

  } else taker = 0;
}

void TeamAIController::ApplyAttackingRun(Player *manualPlayer) {
  endApplyAttackingRun_ms = match->GetActualTime_ms() + 4000;//1500;

  attackingRunPlayer = manualPlayer ? manualPlayer : SelectAttackingRunPlayer(team);
}

void TeamAIController::ApplyTeamPressure() {
  endApplyTeamPressure_ms = match->GetActualTime_ms() + 500;

  Player *opp = match->GetTeam(abs(team->GetID() - 1))->GetBestPossessionPlayer();
  Vector3 opponentPos = opp->GetPosition() + opp->GetMovement() * 0.24f;

  teamPressurePlayer = AI_GetClosestPlayer(team, opponentPos + Vector3(team->GetSide() * 1.0f, 0, 0), true, team->GetGoalie());

  if (teamPressurePlayer) {
    // switch man marking
    int prevMarking = teamPressurePlayer->GetManMarkingID();
    teamPressurePlayer->SetManMarkingID(opp->GetID());

    //SetRedDebugPilon(teamPressurePlayer->GetPosition());
  }
}

void TeamAIController::ApplyKeeperRush() {
  endApplyKeeperRush_ms = match->GetActualTime_ms() + 300;
}

void TeamAIController::CalculateSituation() {
  teamHasPossession = team->HasPossession();
  teamHasUniquePossession = team->HasUniquePossession();
  oppTeamHasPossession = match->GetTeam(abs(team->GetID() - 1))->HasPossession();
  oppTeamHasUniquePossession = match->GetTeam(abs(team->GetID() - 1))->HasUniquePossession();
  teamHasBestPossession = match->GetBestPossessionTeam() == team;
  teamPossessionAmount = team->GetTeamPossessionAmount();
  fadingTeamPossessionAmount = team->GetFadingTeamPossessionAmount();
  timeNeededToGetToBall = team->GetTimeNeededToGetToBall_ms();
  oppTimeNeededToGetToBall = match->GetTeam(abs(team->GetID() - 1))->GetTimeNeededToGetToBall_ms();
}

void TeamAIController::UpdateTactics() {
  const TeamTactics &teamTactics = team->GetTeamData()->GetTactics();
  TeamTactics &teamTacticsWritable = team->GetTeamData()->GetTacticsWritable();

  const Properties &userTacticsModifiers = teamTactics.userProperties;

  int goals = match->GetMatchData()->GetGoalCount(team->GetID());
  int oppGoals = match->GetMatchData()->GetGoalCount(abs(team->GetID() - 1));

  liveTeamTactics = baseTeamTactics;

  // when trailing, we need goals. when leading, defend lead
  float goalFactor = clamp(0.5 + (oppGoals - goals) * 0.25f, 0.0f, 1.0f);
  // time still to play matters - get more desperate towards the end
  float timeFactor = 0.5f + 0.5f * clamp(match->GetMatchTime_ms() / 6300000.0f, 0.0f, 1.0f);
  //printf("timefactor: %f\n", timeFactor);

  float offenseBias = clamp(0.5f + (goalFactor - 0.5f) * (timeFactor * 1.0f), 0.0f, 1.0f);


  float possessionFactor = match->GetMatchData()->GetPossessionFactor_60seconds();
  float recentPossessionBias = 1.0f - fabs(possessionFactor - team->GetID());
  // if (team->GetID() == 0) printf("team 0 recentPossessionBias: %f (possessionFactor: %f)\n", recentPossessionBias, possessionFactor);
  // if (team->GetID() == 1) printf("team 1 recentPossessionBias: %f (possessionFactor: %f)\n", recentPossessionBias, possessionFactor);

  offensivenessBias = offenseBias * 0.5f + recentPossessionBias * 0.5f;
  //if (team->GetID() == 0) printf("T1 offensivenessBias: %f\n", offensivenessBias);
  //if (team->GetID() == 1) printf("T2 offensivenessBias: %f\n", offensivenessBias);

  //autoBias = clamp(fabs(offenseBias - 0.5f) * 2.0f, 0.0f, 1.0f); // don't use too much autobias when things are calm (= offenseBias being around 0.5)

  const map_Properties *userMods = userTacticsModifiers.GetProperties();
  map_Properties::const_iterator iter = userMods->begin();
  while (iter != userMods->end()) {
    float multiplier = teamTacticsModMultipliers.GetReal(iter->first.c_str(), 0.0f);

    float userOffset = atof(iter->second.c_str());
    float offset = userOffset;

    // feed back to teamdata (todo: hasn't this system been effectively disabled? if so, delete it to avoid confusion)
    //teamTacticsWritable.liveProperties.Set(iter->first.c_str(), offset);

    offset = (offset - 0.5f) * 2.0f * multiplier;

    float baseValue = baseTeamTactics.GetReal(iter->first.c_str(), -1.0f);
    // printf("value: %s\n", iter->first.c_str());
    if (baseValue >= 0.0f) { // not all user mods from teamdata are used in this class (for example, individual settings like dribble stuff), ignore them
      liveTeamTactics.Set(iter->first.c_str(), clamp(baseValue + offset, 0.0f, 1.0f));
    }
    iter++;
  }

}

void TeamAIController::Reset() {
  taker = 0;

  setPieceType = e_GameMode_Normal;

  offsideTrapX = 0;

  endApplyAttackingRun_ms = 0;
  attackingRunPlayer = 0;
  endApplyTeamPressure_ms = 0;
  teamPressurePlayer = 0;
  endApplyKeeperRush_ms = 0;

  teamHasPossession = false;
  teamHasUniquePossession = false;
  oppTeamHasPossession = false;
  oppTeamHasUniquePossession = false;
  teamHasBestPossession = false;
  teamPossessionAmount = 1.0f;
  fadingTeamPossessionAmount = 1.0f;
  timeNeededToGetToBall = 100;
  oppTimeNeededToGetToBall = 100;
}
