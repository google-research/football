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

#ifndef _HPP_TEAM_AICONTROLLER
#define _HPP_TEAM_AICONTROLLER

#include "../gamedefines.hpp"

#include "../base/properties.hpp"

class Match;
class Team;

struct TacticalOpponentInfo {
  Player *player;
  float dangerFactor = 0.0f;
};

class TeamAIController {

  public:
    TeamAIController(Team *team);
    virtual ~TeamAIController();

    void Process();

    Vector3 GetAdaptedFormationPosition(Player *player, bool useDynamicFormationPosition = true);
    void CalculateDynamicRoles();
    float CalculateMarkingQuality(Player *player, Player *opp);
    void CalculateManMarking();
    void ApplyOffsideTrap(Vector3 &position) const;
    float GetOffsideTrapX() const { return offsideTrapX; }
    void PrepareSetPiece(e_GameMode setPiece, Team* other_team,
                         int kickoffTakerTeamId, int takerTeamID);
    Player *GetPieceTaker() { return taker; }
    e_GameMode GetSetPieceType() { return setPieceType; }
    void ApplyAttackingRun(Player *manualPlayer = 0);
    void ApplyTeamPressure();
    void ApplyKeeperRush();
    void CalculateSituation();

    void UpdateTactics();

    unsigned long GetEndApplyAttackingRun_ms() { return endApplyAttackingRun_ms; }
    Player *GetAttackingRunPlayer() { return attackingRunPlayer; }

    unsigned long GetEndApplyTeamPressure_ms() { return endApplyTeamPressure_ms; }
    Player *GetTeamPressurePlayer() { return teamPressurePlayer; }

    Player *GetForwardSupportPlayer() { return forwardSupportPlayer; }

    unsigned long GetEndApplyKeeperRush_ms() { return endApplyKeeperRush_ms; }

    const std::vector<TacticalOpponentInfo> &GetTacticalOpponentInfo() { return tacticalOpponentInfo; }

    void Reset();

  protected:

    Match *match;
    Team *team;
    Player *taker;
    e_GameMode setPieceType;

    Properties baseTeamTactics;
    Properties teamTacticsModMultipliers;
    Properties liveTeamTactics;

    float offensivenessBias = 0.0f;

    bool teamHasPossession = false;
    bool teamHasUniquePossession = false;
    bool oppTeamHasPossession = false;
    bool oppTeamHasUniquePossession = false;
    bool teamHasBestPossession = false;
    float teamPossessionAmount = 0.0f;
    float fadingTeamPossessionAmount = 0.0f;
    int timeNeededToGetToBall = 0;
    int oppTimeNeededToGetToBall = 0;

    float depth = 0.0f;
    float width = 0.0f;

    float offsideTrapX = 0.0f;

    unsigned long endApplyAttackingRun_ms = 0;
    Player *attackingRunPlayer;
    unsigned long endApplyTeamPressure_ms = 0;
    Player *teamPressurePlayer;
    unsigned long endApplyKeeperRush_ms = 0;

    Player *forwardSupportPlayer; // sort of like the attacking run player, but more for a forward offset for a player close to the action, to support the player in possession

    std::vector<TacticalOpponentInfo> tacticalOpponentInfo;

};

#endif
