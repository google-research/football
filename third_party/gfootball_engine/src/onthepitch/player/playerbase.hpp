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

#ifndef _HPP_PLAYERBASE
#define _HPP_PLAYERBASE

#include "humanoid/humanoidbase.hpp"
#include "../../data/playerdata.hpp"
#include "controller/icontroller.hpp"

#include "../../scene/scene3d/node.hpp"

class Match;
class HumanController;

class PlayerBase {

  public:
    PlayerBase(Match *match, PlayerData *playerData);
    virtual ~PlayerBase();

    inline int GetID() const { return id; }
    inline int GetStableID() const { return stable_id; }
    inline const PlayerData* GetPlayerData() { return playerData; }

    inline bool IsActive() { return isActive; }

    // get ready for some action
    virtual void Activate(boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection, bool lazyPlayer) = 0;
    // go back to bench/take a shower
    virtual void Deactivate();

    void ResetPosition(const Vector3 &newPos, const Vector3 &focusPos) { humanoid->ResetPosition(newPos, focusPos); }
    void OffsetPosition(const Vector3 &offset) { humanoid->OffsetPosition(offset); }

    inline int GetFrameNum() { return humanoid->GetFrameNum(); }
    inline int GetFrameCount() { return humanoid->GetFrameCount(); }

    inline Vector3 GetPosition() const { return humanoid->GetPosition(); }
    inline Vector3 GetGeomPosition() const { return humanoid->GetGeomPosition(); }
    inline Vector3 GetDirectionVec() const { return humanoid->GetDirectionVec(); }
    inline Vector3 GetBodyDirectionVec() const { return humanoid->GetBodyDirectionVec(); }
    inline Vector3 GetMovement() const { return humanoid->GetMovement(); }
    inline radian GetRelBodyAngle() const {
      return humanoid->GetRelBodyAngle();
    }
    inline e_Velocity GetEnumVelocity() const { return humanoid->GetEnumVelocity(); }
    inline float GetFloatVelocity() const { return EnumToFloatVelocity(humanoid->GetEnumVelocity()); }
    inline e_FunctionType GetCurrentFunctionType() const { return humanoid->GetCurrentFunctionType(); }
    inline e_FunctionType GetPreviousFunctionType() const { return humanoid->GetPreviousFunctionType(); }

    void TripMe(const Vector3 &tripVector, int tripType) { humanoid->TripMe(tripVector, tripType); }

    void RequestCommand(PlayerCommandQueue &commandQueue);
    IController *GetController();
    void SetExternalController(HumanController *externalController);
    HumanController *GetExternalController();

    boost::intrusive_ptr<Node> GetHumanoidNode() { return humanoid->GetHumanoidNode(); }
    boost::intrusive_ptr<Node> GetFullbodyNode() { return humanoid->GetFullbodyNode(); }

    float GetDecayingPositionOffsetLength() { return humanoid->GetDecayingPositionOffsetLength(); }

    virtual void Process();
    virtual void PreparePutBuffers(unsigned long snapshotTime_ms);
    virtual void FetchPutBuffers(unsigned long putTime_ms);
    void Put();

    bool NeedsModelUpdate() { return humanoid->NeedsModelUpdate(); }
    void UpdateFullbodyModel() { humanoid->UpdateFullbodyModel(); }

    virtual float GetStat(PlayerStat name) const;
    float GetVelocityMultiplier() const;
    float GetMaxVelocity() const;

    const Anim *GetCurrentAnim() { return humanoid->GetCurrentAnim(); }

    void SetLastTouchTime_ms(unsigned long touchTime_ms) { this->lastTouchTime_ms = touchTime_ms; }
    unsigned long GetLastTouchTime_ms() { return lastTouchTime_ms; }
    void SetLastTouchType(e_TouchType touchType) { this->lastTouchType = touchType; }
    e_TouchType GetLastTouchType() { return lastTouchType; }
    float GetLastTouchBias(int decay_ms, unsigned long time_ms = 0);

    const NodeMap &GetNodeMap() { return humanoid->GetNodeMap(); }

    float GetFatigueFactorInv() const { return fatigueFactorInv; }
    void RelaxFatigue(float howMuch) {
      fatigueFactorInv += howMuch;
      fatigueFactorInv = clamp(fatigueFactorInv, 0.01f, 1.0f);
    }

    virtual void ResetSituation(const Vector3 &focusPos);
    static void resetPlayerCount() {
      stablePlayerCount = 0;
    }

  protected:
    Match *match;

    const PlayerData* const playerData;
    const int id = 0;
    const int stable_id = 0;

    HumanoidBase *humanoid;
    IController *controller;
    HumanController *externalController;

    bool isActive = false;

    unsigned long lastTouchTime_ms = 0;
    e_TouchType lastTouchType;

    static int playerCount;
    static int stablePlayerCount;

    float fatigueFactorInv = 0.0f;

    std::vector<Vector3> positionHistoryPerSecond; // resets too (on ResetSituation() calls)

};

#endif
