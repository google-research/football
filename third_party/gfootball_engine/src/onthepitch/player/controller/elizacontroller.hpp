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

#ifndef _HPP_FOOTBALL_ONTHEPITCH_ELIZACONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_ELIZACONTROLLER

#include "playercontroller.hpp"

#include "../../../gamedefines.hpp"

struct Prerequisites;
class Strategy;
class MentalImage;
class Team;
class Player;

class ElizaController : public PlayerController {

  public:
    ElizaController(Match *match, bool lazyPlayer);
    virtual ~ElizaController();

    virtual void RequestCommand(PlayerCommandQueue &commandQueue);
    virtual void Process();
    virtual Vector3 GetDirection();
    virtual float GetFloatVelocity();

    void LoadStrategies();

    float GetLazyVelocity(float desiredVelocityFloat);
    Vector3 GetSupportPosition_ForceField(const MentalImage *mentalImage,
                                          const Vector3 &basePosition,
                                          bool makeRun = false);

    virtual void Reset();

  protected:
    void GetOnTheBallCommands(std::vector<PlayerCommand> &commandQueue, Vector3 &rawInputDirection, float &rawInputVelocity);

    void _AddPass(std::vector<PlayerCommand> &commandQueue, Player *target, e_FunctionType passType);
    void _AddPanicPass(std::vector<PlayerCommand> &commandQueue);
    float _GetPassingOdds(Player *targetPlayer, e_FunctionType passType, const std::vector<PlayerImagePosition> &opponentPlayerImages, float ballVelocityMultiplier = 1.0f);
    float _GetPassingOdds(const Vector3 &target, e_FunctionType passType, const std::vector<PlayerImagePosition> &opponentPlayerImages, float ballVelocityMultiplier = 1.0f);
    void _AddCelebration(std::vector<PlayerCommand> &commandQueue);

    Strategy *defenseStrategy;
    Strategy *midfieldStrategy;
    Strategy *offenseStrategy;
    Strategy *goalieStrategy;

    Vector3 lastDesiredDirection;
    float lastDesiredVelocity = 0.0f;
    const bool lazyPlayer = false;

};

#endif
