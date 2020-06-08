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

#ifndef _HPP_TEAM
#define _HPP_TEAM

#include "player/player.hpp"
#include "../data/teamdata.hpp"
#include "teamAIcontroller.hpp"
#include "humangamer.hpp"

class Match;

class Team {

  public:
    Team(int id, Match *match, TeamData *teamData, float aiDifficulty);
    void Mirror();
    bool isMirrored() { DO_VALIDATION;
      return mirrored;
    }
    bool onOriginalSide() { DO_VALIDATION;
      return id == 0 ? (side == -1) : (side == 1);
    }

    virtual ~Team();

    void Exit();

    void InitPlayers(boost::intrusive_ptr<Node> fullbodyNode,
                     std::map<Vector3, Vector3> &colorCoords);

    Match *GetMatch() { DO_VALIDATION; return match; }
    TeamAIController *GetController() { DO_VALIDATION; return teamController; }
    boost::intrusive_ptr<Node> GetSceneNode() { DO_VALIDATION; return teamNode; }

    int GetID() const { return id; }
    inline signed int GetDynamicSide() { DO_VALIDATION;
      return side;
    }
    inline signed int GetStaticSide() { DO_VALIDATION;
      return id == 0 ? -1 : 1;
    }
    const TeamData *GetTeamData() { DO_VALIDATION; return teamData; }

    FormationEntry GetFormationEntry(void* player);
    void SetFormationEntry(Player* player, FormationEntry entry);
    float GetAiDifficulty() const { return aiDifficulty; }
    const std::vector<Player *> &GetAllPlayers() { return players; }
    void GetAllPlayers(std::vector<Player*> &allPlayers) { DO_VALIDATION;
      allPlayers.insert(allPlayers.end(), players.begin(), players.end());
    }
    void GetActivePlayers(std::vector<Player *> &activePlayers);
    int GetActivePlayersCount() const;
    Player *MainSelectedPlayer() { return mainSelectedPlayer; }

    unsigned int GetHumanGamerCount() {
      int count = 0;
      for (auto& g: humanGamers) { DO_VALIDATION;
        if (!g->GetHumanController()->Disabled()) {
          count++;
        }
      }
      return count;
    }
    void GetHumanControllers(std::vector<HumanGamer*>& v) {
      for (auto& g: humanGamers) { DO_VALIDATION;
        v.push_back(g.get());
      }
    }
    void AddHumanGamers(const std::vector<AIControlledKeyboard*>& controllers);
    void DeleteHumanGamers();
    e_PlayerColor GetPlayerColor(PlayerBase* player);
    int HumanControlledToBallDistance();

    bool HasPossession() const;
    bool HasUniquePossession() const;
    int GetTimeNeededToGetToBall_ms() const;
    Player *GetDesignatedTeamPossessionPlayer() { DO_VALIDATION;
      return designatedTeamPossessionPlayer;
    }
    void UpdateDesignatedTeamPossessionPlayer();
    Player *GetBestPossessionPlayer();
    float GetTeamPossessionAmount() const;
    float GetFadingTeamPossessionAmount() const;
    void SetFadingTeamPossessionAmount(float value);

    void SetLastTouchPlayer(
        Player *player, e_TouchType touchType = e_TouchType_Intentional_Kicked);
    Player *GetLastTouchPlayer() const { return lastTouchPlayer; }
    float GetLastTouchBias(int decay_ms, unsigned long time_ms = 0) { DO_VALIDATION;
      return lastTouchPlayer
                 ? lastTouchPlayer->GetLastTouchBias(decay_ms, time_ms)
                 : 0;
    }

    void ResetSituation(const Vector3 &focusPos);

    void HumanGamersSelectAnyone();
    void SetOpponent(Team* opponent) { DO_VALIDATION; this->opponent = opponent; }
    Team* Opponent() { DO_VALIDATION; return opponent; }
    void SelectPlayer(Player *player);
    void DeselectPlayer(Player *player);

    void RelaxFatigue(float howMuch);

    void Process();
    void PreparePutBuffers();
    void FetchPutBuffers();
    void Put(bool mirror);
    void Put2D(bool mirror);
    void Hide2D();

    void UpdatePossessionStats();
    void UpdateSwitch();
    void ProcessState(EnvState* state);

    Player *GetGoalie();

  protected:
    const int id;
    Match *match;
    Team *opponent = 0;
    TeamData *teamData;
    const float aiDifficulty;

    bool hasPossession = false;
    int timeNeededToGetToBall_ms = 0;
    Player *designatedTeamPossessionPlayer = 0;

    float teamPossessionAmount = 0.0f;
    float fadingTeamPossessionAmount = 0.0f;

    TeamAIController *teamController;

    std::vector<Player*> players;

    boost::intrusive_ptr<Node> teamNode;
    boost::intrusive_ptr<Node> playerNode;

    std::vector<std::unique_ptr<HumanGamer>> humanGamers;

    // humanGamers index whose turn it is
    // begin() == due next
    std::list<int> switchPriority;
    Player *lastTouchPlayer = nullptr;
    Player *mainSelectedPlayer = nullptr;

    boost::intrusive_ptr < Resource<Surface> > kit;
    int side = -1;
    bool mirrored = false;
};

#endif
