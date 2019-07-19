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

#ifndef _HPP_MATCH
#define _HPP_MATCH

#include "../framework/scheduler.hpp"

#include "team.hpp"
#include "ball.hpp"
#include "referee.hpp"
#include "officials.hpp"

#include "../data/matchdata.hpp"
#include "player/humanoid/animcollection.hpp"
#include "AIsupport/mentalimage.hpp"

#include "../menu/menutask.hpp"

#include "../utils/gui2/widgets/caption.hpp"
#include "../menu/ingame/scoreboard.hpp"
#include "../menu/ingame/radar.hpp"

#include "../scene/objects/camera.hpp"
#include "../scene/objects/light.hpp"

#include "../types/messagequeue.hpp"
#include "../types/command.hpp"

#include <boost/circular_buffer.hpp>

#include <fstream>
#include <iostream>

struct PlayerBounce {
  Player *opp;
  float force = 0.0f;
};

class Match {

  public:
    Match(MatchData *matchData, const std::vector<IHIDevice*> &controllers);
    virtual ~Match();

    void Exit();

    void SetRandomSunParams();
    void RandomizeAdboards(boost::intrusive_ptr<Node> stadiumNode);
    void UpdateControllerSetup();
    void SpamMessage(const std::string &msg, int time_ms = 3000);
    int GetScore(int teamID) { return matchData->GetGoalCount(teamID); }
    Ball *GetBall() { return ball; }
    Team *GetTeam(int teamID) { return teams[teamID]; }
    Player *GetPlayer(int playerID);
    void GetAllTeamPlayers(int teamID, std::vector<Player*> &players);
    void GetActiveTeamPlayers(int teamID, std::vector<Player*> &players);
    void GetOfficialPlayers(std::vector<PlayerBase*> &players);
    boost::shared_ptr<AnimCollection> GetAnimCollection();

    const MentalImage *GetMentalImage(int history_ms);
    void UpdateLatestMentalImageBallPredictions();

    void ResetSituation(const Vector3 &focusPos);

    bool GetPause() { return pause; }
    void SetMatchPhase(e_MatchPhase newMatchPhase);
    e_MatchPhase GetMatchPhase() const { return matchPhase; }

    void StartPlay() { inPlay = true; }
    void StopPlay() { inPlay = false; }
    bool IsInPlay() const { return inPlay; }

    void StartSetPiece() { inSetPiece = true; }
    void StopSetPiece() { inSetPiece = false; }
    bool IsInSetPiece() const { return inSetPiece; }
    Referee *GetReferee() { return referee; }
    Officials *GetOfficials() { return officials; }

    void SetGoalScored(bool onOff) { if (onOff == false) ballIsInGoal = false; goalScored = onOff; }
    bool IsGoalScored() const { return goalScored; }
    int GetLastGoalTeamID() const { return lastGoalTeamID; }
    void SetLastTouchTeamID(int id, e_TouchType touchType = e_TouchType_Intentional_Kicked) { lastTouchTeamIDs[touchType] = id; lastTouchTeamID = id; referee->BallTouched(); }
    int GetLastTouchTeamID(e_TouchType touchType) const { return lastTouchTeamIDs[touchType]; }
    int GetLastTouchTeamID() const { return lastTouchTeamID; }
    Team *GetLastTouchTeam() {
      if (lastTouchTeamID != -1)
        return teams[lastTouchTeamID];
      else
        return teams[0];
    }
    Player *GetLastTouchPlayer() {
      if (GetLastTouchTeam())
        return GetLastTouchTeam()->GetLastTouchPlayer();
      else
        return 0;
    }
    float GetLastTouchBias(int decay_ms, unsigned long time_ms = 0) { if (GetLastTouchTeam()) return GetLastTouchTeam()->GetLastTouchBias(decay_ms, time_ms); else return 0; }
    bool IsBallInGoal() const { return ballIsInGoal; }

    Team* GetBestPossessionTeam();

    Player *GetDesignatedPossessionPlayer() { return designatedPossessionPlayer; }
    Player *GetBallRetainer() { return ballRetainer; }
    void SetBallRetainer(Player *retainer) { ballRetainer = retainer; }

    float GetAveragePossessionSide(int time_ms) const { return possessionSideHistory->GetAverage(time_ms); }

    unsigned long GetIterations() const { return iterations; }
    unsigned long GetMatchTime_ms() const { return matchTime_ms; }
    unsigned long GetActualTime_ms() const { return actualTime_ms; }

    void GameOver();

    void UpdateIngameCamera();


    boost::intrusive_ptr<Camera> GetCamera() { return camera; }

    void GetState(SharedInfo* state);
    void Get();
    void Process();
    void PreparePutBuffers();
    void FetchPutBuffers();
    void Put();

    boost::intrusive_ptr<Node> GetDynamicNode();

    void FollowCamera(Quaternion &orientation, Quaternion &nodeOrientation, Vector3 &position, float &FOV, const Vector3 &targetPosition, float zoom);

    void SetAutoUpdateIngameCamera(bool autoUpdate = true) { if (autoUpdate != autoUpdateIngameCamera) { camPos.clear(); autoUpdateIngameCamera = autoUpdate; } }

    int GetReplaySize_ms();

    MatchData* GetMatchData() { return matchData; }

    float GetMatchDurationFactor() const { return matchDurationFactor; }
    float GetMatchDifficulty() const { return matchDifficulty; }
    bool GetUseMagnet() const { return _useMagnet; }

    const std::vector<Vector3> &GetAnimPositionCache(Animation *anim) const;

    void UploadGoalNetting();

    unsigned long GetPreviousProcessTime_ms() { return previousProcessTime_ms; } // always around 10ms, not a very useful function, probably
    unsigned long GetPreviousPreparePutTime_ms() { return previousPreparePutTime_ms; }
    unsigned long GetPreviousPutTime_ms() { return previousPutTime_ms; }

    //void AddMissingAnim(const MissingAnim &someAnim);

    // not sure about how signals work in this game at the moment. whole menu/game thing needs a rethink, i guess
    boost::signals2::signal<void(Match*)> sig_OnMatchPhaseChange;
    boost::signals2::signal<void(Match*)> sig_OnShortReplayMoment;
    boost::signals2::signal<void(Match*)> sig_OnExtendedReplayMoment;
    boost::signals2::signal<void(Match*)> sig_OnGameOver;
    boost::signals2::signal<void(Match*)> sig_OnCreatedMatch;
    boost::signals2::signal<void(Match*)> sig_OnExitedMatch;

  private:
    bool CheckForGoal(signed int side);

    void CalculateBestPossessionTeamID();
    void CheckHumanoidCollisions();
    void CheckHumanoidCollision(Player *p1, Player *p2, std::vector<PlayerBounce> &p1Bounce, std::vector<PlayerBounce> &p2Bounce);
    void CheckBallCollisions();

    void PrepareGoalNetting();
    void UpdateGoalNetting(bool ballTouchesNet = false);

    // for stuff like animation smoothing, we could need the time elapsed since last Put() and such
    unsigned long previousProcessTime_ms = 0;
    unsigned long previousPreparePutTime_ms = 0;
    unsigned long previousPutTime_ms = 0;
    int timeSincePreviousProcess_ms = 0;
    int timeSincePreviousPreparePut_ms = 0;
    int timeSincePreviousPut_ms = 0;

    MatchData *matchData;
    Team *teams[2];

    Officials *officials;

    boost::intrusive_ptr<Node> dynamicNode;

    boost::intrusive_ptr<Node> cameraNode;
    boost::intrusive_ptr<Camera> camera;
    boost::intrusive_ptr<Node> sunNode;

    boost::intrusive_ptr<Node> stadiumNode;
    boost::intrusive_ptr<Node> goalsNode;

    // camera user settings
    float cameraUserZoom = 0.0f;
    float cameraUserHeight = 0.0f;
    float cameraUserFOV = 0.0f;
    float cameraUserAngleFactor = 0.0f;

    const std::vector<IHIDevice*> &controllers;

    Ball *ball = nullptr;

    std::vector<MentalImage*> mentalImages; // [index] == index * 10 ms ago ([0] == now)

    Gui2ScoreBoard *scoreboard;
    Gui2Radar *radar;
    Gui2Caption *messageCaption;
    unsigned long messageCaptionRemoveTime_ms = 0;
    unsigned long iterations = 0;
    TaskSequenceInfo gameSequenceInfo;
    unsigned long matchTime_ms = 0;
    unsigned long actualTime_ms = 0;
    unsigned long buf_matchTime_ms = 0;
    unsigned long buf_actualTime_ms = 0;
    unsigned long fetchedbuf_matchTime_ms = 0;
    unsigned long fetchedbuf_actualTime_ms = 0;
    unsigned long goalScoredTimer = 0;

    bool pause = false;
    e_MatchPhase matchPhase = e_MatchPhase_PreMatch; // 0 - first half; 1 - second half; 2 - 1st extra time; 3 - 2nd extra time; 4 - penalties
    bool inPlay = false;
    bool inSetPiece = false; // Whether game is in special mode (corner etc...)
    bool goalScored = false; // true after goal scored, false again after next match state change
    bool ballIsInGoal = false;
    int lastGoalTeamID = 0;
    Player *lastGoalScorer;
    int lastTouchTeamIDs[e_TouchType_SIZE];
    int lastTouchTeamID = 0;
    Team* bestPossessionTeam = 0;
    Player *designatedPossessionPlayer;
    Player *ballRetainer;

    bool gameOver = false;

    boost::intrusive_ptr<Node> fullbodyNode;
    boost::intrusive_ptr<Node> fullbody2Node;

    ValueHistory<float> *possessionSideHistory;

    bool autoUpdateIngameCamera = false;

    // camera
    Quaternion cameraOrientation;
    Quaternion cameraNodeOrientation;
    Vector3 cameraNodePosition;
    float cameraFOV = 0.0f;
    float cameraNearCap = 0.0f;
    float cameraFarCap = 0.0f;

    TemporalSmoother<Quaternion> buf_cameraOrientation;
    TemporalSmoother<Quaternion> buf_cameraNodeOrientation;
    TemporalSmoother<Vector3> buf_cameraNodePosition;
    TemporalSmoother<float> buf_cameraFOV;
    float buf_cameraNearCap = 0.0f;
    float buf_cameraFarCap = 0.0f;
    Quaternion fetchedbuf_cameraOrientation;
    Quaternion fetchedbuf_cameraNodeOrientation;
    Vector3 fetchedbuf_cameraNodePosition;
    float fetchedbuf_cameraFOV = 0.0f;
    float fetchedbuf_cameraNearCap = 0.0f;
    float fetchedbuf_cameraFarCap = 0.0f;

    int fetchedbuf_timeDelta = 0;

    unsigned int lastBodyBallCollisionTime_ms = 0;

    std::deque<Vector3> camPos;

    Referee *referee;

    boost::shared_ptr<MenuTask> menuTask;

    boost::shared_ptr<Scene3D> scene3D;

    bool resetNetting = false;
    bool nettingHasChanged = false;

    float excitement = 0.0f;

    Vector3 previousBallPos;

    float matchDurationFactor = 0.0f;

    std::vector<Vector3> nettingMeshesSrc[2];
    std::vector<float*> nettingMeshes[2];

    float matchDifficulty = 0.0f;
    // Whether to use magnet logic (that automatically pushes active player
    // towards the ball).
    bool _useMagnet = true;
};

#endif
