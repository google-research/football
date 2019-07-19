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

#ifndef _HPP_PLAYER
#define _HPP_PLAYER

#include "humanoid/humanoid.hpp"
#include "playerbase.hpp"

#include "../../utils/gui2/widgets/caption.hpp"

#include "../../menu/menutask.hpp"

class Match;
class Team;
class ElizaController;

struct TacticalPlayerSituation {
  float forwardSpaceRating = 0.0f;
  float toGoalSpaceRating = 0.0f;
  float spaceRating = 0.0f;
  float forwardRating = 0.0f;
};

class Player : public PlayerBase {

  public:
    Player(Team *team, PlayerData *playerData);
    virtual ~Player();

    Humanoid *CastHumanoid();
    ElizaController *CastController();

    int GetTeamID() const;
    Team *GetTeam();

    // get ready for some action
    virtual void Activate(boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection, bool lazyPlayer);
    // go back to bench/take a shower
    virtual void Deactivate();

    bool TouchPending() { return CastHumanoid()->TouchPending(); }
    bool TouchAnim() { return CastHumanoid()->TouchAnim(); }
    Vector3 GetTouchPos() { return CastHumanoid()->GetTouchPos(); }
    int GetTouchFrame() { return CastHumanoid()->GetTouchFrame(); }
    int GetCurrentFrame() { return CastHumanoid()->GetCurrentFrame(); }

    void SelectRetainAnim() { CastHumanoid()->SelectRetainAnim(); }

    inline e_FunctionType GetCurrentFunctionType() { return CastHumanoid()->GetCurrentFunctionType(); }
    FormationEntry GetFormationEntry();
    inline void SetDynamicFormationEntry(FormationEntry entry) { dynamicFormationEntry = entry; }
    inline FormationEntry GetDynamicFormationEntry() { return dynamicFormationEntry; }
    inline void SetManMarkingID(int id) { manMarkingID = id; }
    inline int GetManMarkingID() { return manMarkingID; }

    bool HasPossession() const;
    bool HasBestPossession() const;
    bool HasUniquePossession() const;
    inline int GetPossessionDuration_ms() const { return possessionDuration_ms; }
    inline int GetTimeNeededToGetToBall_ms() const { return timeNeededToGetToBall_ms; }
    inline int GetTimeNeededToGetToBall_optimistic_ms() const { return timeNeededToGetToBall_optimistic_ms; }
    inline int GetTimeNeededToGetToBall_previous_ms() const { return timeNeededToGetToBall_previous_ms; }
    void SetDesiredTimeToBall_ms(int ms) { desiredTimeToBall_ms = ms; }
    int GetDesiredTimeToBall_ms() const { return clamp(desiredTimeToBall_ms, timeNeededToGetToBall_ms, 1000000.0f); }
    bool AllowLastDitch(bool includingPossessionAmount = true) const;

    void TriggerControlledBallCollision() { triggerControlledBallCollision = true; }
    bool IsControlledBallCollisionTriggered() { return triggerControlledBallCollision; }
    void ResetControlledBallCollisionTrigger() { triggerControlledBallCollision = false; }

    float GetAverageVelocity(float timePeriod_sec); // is reset on ResetSituation() calls

    void UpdatePossessionStats(bool onInterval = true);

    float GetClosestOpponentDistance() const;

    const TacticalPlayerSituation &GetTacticalSituation() { return tacticalSituation; }

    virtual void Process();
    virtual void PreparePutBuffers(unsigned long snapshotTime_ms);
    virtual void FetchPutBuffers(unsigned long putTime_ms);
    void Put2D();
    void Hide2D();

    void GiveYellowCard(unsigned long giveTime_ms) { cards++; cardEffectiveTime_ms = giveTime_ms; }
    void GiveRedCard(unsigned long giveTime_ms) {
      cards += 3;
      cardEffectiveTime_ms = giveTime_ms;
    }

    bool HasCards() {
      return cards > 0;
    }

    void SendOff();

    float GetStaminaStat() const;
    virtual float GetStat(PlayerStat name) const;

    virtual void ResetSituation(const Vector3 &focusPos);

  protected:
    void _CalculateTacticalSituation();

    Team *team = nullptr;

    signed int manMarkingID = 0;

    FormationEntry dynamicFormationEntry;

    bool hasPossession = false;
    bool hasBestPossession = false;
    bool hasUniquePossession = false;
    int possessionDuration_ms = 0;
    unsigned int timeNeededToGetToBall_ms = 0;
    unsigned int timeNeededToGetToBall_optimistic_ms = 0;
    unsigned int timeNeededToGetToBall_previous_ms = 0;

    bool triggerControlledBallCollision = false;

    TacticalPlayerSituation tacticalSituation;

    bool buf_nameCaptionShowCondition = false;
    std::string buf_nameCaption;
    std::string buf_debugCaption;
    Vector3 buf_nameCaptionPos;
    Vector3 buf_debugCaptionPos;
    Vector3 buf_playerColor;
    Vector3 buf_debugCaptionColor;

    bool fetchedbuf_nameCaptionShowCondition = false;
    bool fetchedbuf_debugCaptionShowCondition = false;
    std::string fetchedbuf_nameCaption;
    std::string fetchedbuf_debugCaption;
    Vector3 fetchedbuf_nameCaptionPos;
    Vector3 fetchedbuf_debugCaptionPos;
    Vector3 fetchedbuf_playerColor;
    Vector3 fetchedbuf_debugCaptionColor;

    Gui2Caption *nameCaption = nullptr;
    Gui2Caption *debugCaption = nullptr;

    boost::shared_ptr<MenuTask> menuTask;

    int desiredTimeToBall_ms = 0;
    int cards = 0; // 1 == 1 yellow; 2 == 2 yellow; 3 == 1 red; 4 == 1 yellow, 1 red

    unsigned long cardEffectiveTime_ms = 0;

};

#endif
