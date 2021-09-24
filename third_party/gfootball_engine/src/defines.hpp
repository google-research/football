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

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_DEFINES
#define _HPP_DEFINES

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <fstream>
#include <cmath>

#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/slot.hpp>
#include <boost/bind.hpp>
#include "backtrace.h"
#include "base/log.hpp"

#define CHECK(a) assert(a);
#define CHECK_EQ(a, b) assert((a) == (b));

constexpr float EPSILON = 0.000001;

#define X_FIELD_SCALE 54.4
#define Y_FIELD_SCALE -83.6
#define Z_FIELD_SCALE 1
#define MAX_PLAYERS 11

typedef std::string screenshoot;

namespace blunted {
  class Animation;
  //using namespace boost;
}

class Player;
class Team;
class HumanGamer;
class AIControlledKeyboard;
class ScenarioConfig;
class GameContext;
class GameEnv;


#include "base/math/vector3.hpp"

class EnvState {
 public:
  EnvState(GameEnv* game_env, const std::string& state, const std::string reference = "");
  const ScenarioConfig* getConfig() { return scenario_config; }
  const GameContext* getContext() { return context; }
  void process(std::string &value);
  void process(blunted::Animation* &value);
  template<typename T> void process(std::vector<T>& collection) {
    int size = collection.size();
    process(size);
    collection.resize(size);
    for (auto& el : collection) {
      process(el);
    }
  }
  template<typename T> void process(std::list<T>& collection) {
    int size = collection.size();
    process(size);
    collection.resize(size);
    for (auto& el : collection) {
      process(el);
    }
  }
  void process(Player*& value);
  void process(HumanGamer*& value);
  void process(AIControlledKeyboard*& value);
  void process(Team*& value);
  bool isFailure() {
    return failure;
  }
  bool enabled() {
    return this->disable_cnt == 0;
  }
  void setValidate(bool validate) {
    this->disable_cnt += validate ? -1 : 1;
  }
  void setCrash(bool crash) {
    this->crash = crash;
  }
  bool Load() { return load; }
  int getpos() {
    return pos;
  }
  bool eos();
  template<typename T> void process(T& obj) {
    if (load) {
      if (pos + sizeof(T) > state.size()) {
        Log(blunted::e_FatalError, "EnvState", "state", "state is invalid");
      }
      memcpy(&obj, &state[pos], sizeof(T));
      pos += sizeof(T);
    } else {
      state.resize(pos + sizeof(T));
      memcpy(&state[pos], &obj, sizeof(T));
      if (!failure && disable_cnt == 0 && !reference.empty() && (*(T*) &state[pos]) != (*(T*) &reference[pos])) {
        failure = true;
        std::cout << "Position:  " << pos << std::endl;
        std::cout << "Type:      " << typeid(obj).name() << std::endl;
        std::cout << "Value:     " << obj << std::endl;
        std::cout << "Reference: " << (*(T*) &reference[pos]) << std::endl;
        if (crash) {
          Log(blunted::e_FatalError, "EnvState", "state", "Reference mismatch");
        } else {
          print_stacktrace();
        }
      }
      pos += sizeof(T);
      if (pos > 10000000) {
        Log(blunted::e_FatalError, "EnvState", "state", "state is too big");
      }
    }
  }
  void SetPlayers(const std::vector<Player*>& players);
  void SetHumanControllers(const std::vector<HumanGamer*>& controllers);
  void SetControllers(const std::vector<AIControlledKeyboard*>& controllers);
  void SetAnimations(const std::vector<blunted::Animation*>& animations);
  void SetTeams(Team* team0, Team* team1);
  const std::string& GetState();
 protected:
  bool failure = false;
  bool stack = true;
  bool load = false;
  char disable_cnt = 0;
  bool crash = false;
  std::vector<Player*> players;
  std::vector<blunted::Animation*> animations;
  std::vector<Team*> teams;
  std::vector<HumanGamer*> human_controllers;
  std::vector<AIControlledKeyboard*> controllers;
  std::string state;
  std::string reference;
  int pos = 0;
  ScenarioConfig* scenario_config;
  GameContext* context;
 private:
  void process(void** collection, int size, void*& element);
};

// 3-d position of object (available from python).
struct Position {
  Position(float x = 0.0, float y = 0.0, float z = 0.0, bool env_coords = false) {
    if (env_coords) {
      value[0] = X_FIELD_SCALE * x;
      value[1] = Y_FIELD_SCALE * y;
      value[2] = Z_FIELD_SCALE * z;
    } else {
      value[0] = x;
      value[1] = y;
      value[2] = z;
    }
  }
  Position(const Position& other) {
    value[0] = other.value[0];
    value[1] = other.value[1];
    value[2] = other.value[2];
  }
  Position& operator=(float* position) {
    value[0] = position[0];
    value[1] = position[1];
    value[2] = position[2];
    return *this;
  }
  bool operator == (const Position& f) const {
    return value[0] == f.value[0] &&
        value[1] == f.value[1] &&
        value[2] == f.value[2];
  }
  // Returns environment coordinates, ie [-1,1] for x (0),
  // [-0.42,0.42] for y (1).
  float env_coord(int index) const;
  std::string debug();
 private:
  float value[3];
};

enum e_PlayerRole {
  e_PlayerRole_GK,
  e_PlayerRole_CB,
  e_PlayerRole_LB,
  e_PlayerRole_RB,
  e_PlayerRole_DM,
  e_PlayerRole_CM,
  e_PlayerRole_LM,
  e_PlayerRole_RM,
  e_PlayerRole_AM,
  e_PlayerRole_CF,
};

enum e_GameMode {
  e_GameMode_Normal,
  e_GameMode_KickOff,
  e_GameMode_GoalKick,
  e_GameMode_FreeKick,
  e_GameMode_Corner,
  e_GameMode_ThrowIn,
  e_GameMode_Penalty,
};

enum e_PlayerColor {
  e_PlayerColor_Blue,
  e_PlayerColor_Green,
  e_PlayerColor_Red,
  e_PlayerColor_Yellow,
  e_PlayerColor_Purple,
  e_PlayerColor_Default
};

enum e_Team {
  e_Left,
  e_Right,
};

// Information about the player (available from python).
struct PlayerInfo {
  PlayerInfo() { }
  PlayerInfo(const PlayerInfo& f) {
    player_position = f.player_position;
    player_direction = f.player_direction;
    has_card = f.has_card;
    is_active = f.is_active;
    tired_factor = f.tired_factor;
    role = f.role;
    designated_player = f.designated_player;
  }
  bool operator == (const PlayerInfo& f) const {
    return player_position == f.player_position &&
        player_direction == f.player_direction &&
        has_card == f.has_card &&
        is_active == f.is_active &&
        tired_factor == f.tired_factor &&
        role == f.role &&
        designated_player == f.designated_player;
  }
  Position player_position;
  Position player_direction;
  bool has_card = false;
  bool is_active = true;
  bool designated_player = false;
  float tired_factor = 0.0f; // In the [0..1] range.
  e_PlayerRole role = e_PlayerRole_GK;
};

struct ControllerInfo {
  ControllerInfo() { }
  ControllerInfo(int controlled_player) : controlled_player(controlled_player) { }
  bool operator == (const ControllerInfo& f) const {
    return controlled_player == f.controlled_player;
  }
  int controlled_player = -1;
};

// All the information about the current state (available from python).
struct SharedInfo {
  Position ball_position;
  Position ball_direction;
  Position ball_rotation;
  std::vector<PlayerInfo> left_team;
  std::vector<PlayerInfo> right_team;
  std::vector<ControllerInfo> left_controllers;
  std::vector<ControllerInfo> right_controllers;
  int left_goals, right_goals;
  e_GameMode game_mode;
  bool is_in_play = false;
  int ball_owned_team = 0;
  int ball_owned_player = 0;
  int step = 0;
};

#endif
