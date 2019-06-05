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

#ifndef _HPP_FOOTBALL_ONTHEPITCH_HUMANCONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_HUMANCONTROLLER

#include "playercontroller.hpp"

#include "../../../hid/ihidevice.hpp"

class Player;

class HumanController : public PlayerController {

  public:
    HumanController(Match *match, IHIDevice *hid);
    virtual ~HumanController();

    virtual void SetPlayer(PlayerBase *player);

    virtual void RequestCommand(PlayerCommandQueue &commandQueue);
    virtual void Process();
    virtual Vector3 GetDirection();
    virtual float GetFloatVelocity();

    virtual int GetReactionTime_ms();

    IHIDevice *GetHIDevice() { return hid; }

    int GetActionMode() { return actionMode; }

    virtual void Reset();

  protected:

    void _GetHidInput(Vector3 &rawInputDirection, float &rawInputVelocityFloat);

    IHIDevice *hid;

    // set when a contextual button (example: pass/defend button) is pressed
    // once this is set and the button stays pressed, it stays the same
    // 0: undefined, 1: off-the-ball button active, 2: on-the-ball button active/action queued
    int actionMode = 0;

    e_ButtonFunction actionButton;
    int actionBufferTime_ms = 0;
    int gauge_ms = 0;

    // stuff to keep track of analog stick (or keys even) so that we can use a direction once it's been pointed in for a while, instead of directly
    Vector3 previousDirection;
    Vector3 steadyDirection;
    int lastSteadyDirectionSnapshotTime_ms = 0;

};

#endif
