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

#ifndef _HPP_FOOTBALL_ONTHEPITCH_PLAYERCONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_PLAYERCONTROLLER

#include "icontroller.hpp"

class PlayerController : public IController {

  public:
    PlayerController(Match *match);
    virtual ~PlayerController() {};

    virtual void Process();

    virtual void SetPlayer(PlayerBase *player);
    Player *CastPlayer();
    Team *GetTeam() { return team; }
    Team *GetOppTeam() { return oppTeam; }

    const MentalImage *GetMentalImage() { return _mentalImage; }

    virtual int GetReactionTime_ms();

    float GetLastSwitchBias();

    float GetFadingTeamPossessionAmount() { return fadingTeamPossessionAmount; }

    void AddDefensiveComponent(Vector3 &desiredPosition, float bias, int forcedOppID = -1);
    Vector3 GetDefendPosition(Player *opp, float distance = 0.0f);

    virtual void Reset();

  protected:
    float OppBetweenBallAndMeDot();
    float CouldWinABallDuelLikeliness();
    virtual void _Preprocess();
    virtual void _SetInput(const Vector3 &inputDirection, float inputVelocityFloat) { this->inputDirection = inputDirection; this->inputVelocityFloat = inputVelocityFloat; }
    virtual void _KeeperDeflectCommand(PlayerCommandQueue &commandQueue, bool onlyPickupAnims = false);
    virtual void _SetPieceCommand(PlayerCommandQueue &commandQueue);
    virtual void _BallControlCommand(PlayerCommandQueue &commandQueue, bool idleTurnToOpponentGoal = false, bool knockOn = false, bool stickyRunDirection = false, bool keepCurrentBodyDirection = false);
    virtual void _TrapCommand(PlayerCommandQueue &commandQueue, bool idleTurnToOpponentGoal = false, bool knockOn = false);
    virtual void _InterfereCommand(PlayerCommandQueue &commandQueue, bool byAnyMeans = false);
    virtual void _SlidingCommand(PlayerCommandQueue &commandQueue);
    virtual void _MovementCommand(PlayerCommandQueue &commandQueue, bool forceMagnet = false, bool extraHaste = false);

    Vector3 inputDirection;
    float inputVelocityFloat = 0.0f;

    Player *_oppPlayer = nullptr;
    float _timeNeeded_ms = 0;
    const MentalImage *_mentalImage = nullptr;

    void _CalculateSituation();

    // only really useful for human gamers, after switching player
    unsigned long lastSwitchTime_ms = 0;
    unsigned int lastSwitchTimeDuration_ms = 0;

    Team *team = nullptr;
    Team *oppTeam = nullptr;

    bool hasPossession = false;
    bool hasUniquePossession = false;
    bool teamHasPossession = false;
    bool teamHasUniquePossession = false;
    bool oppTeamHasPossession = false;
    bool oppTeamHasUniquePossession = false;
    bool hasBestPossession = false;
    bool teamHasBestPossession = false;
    float possessionAmount = 0.0f;
    float teamPossessionAmount = 0.0f;
    float fadingTeamPossessionAmount = 0.0f;
    int timeNeededToGetToBall = 0;
    int oppTimeNeededToGetToBall = 0;
    bool hasBestChanceOfPossession = false;
};

#endif
