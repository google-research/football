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

#include "defines.hpp"

#include "backtrace.h"
#include "base/log.hpp"
#include "game_env.hpp"
#include "main.hpp"

EnvState::EnvState(GameEnv* game, const std::string& state,
                   const std::string reference)
    : load(!state.empty()),
      state(state),
      reference(reference),
      scenario_config(&game->scenario_config),
      context(game->context) {
}

void EnvState::process(std::string& value) {
  int s = value.size();
  process(s);
  value.resize(s);
  for (char& c : value) {
    process(c);
  }
}

void EnvState::process(void** collection, int size, void*& element) {
  DO_VALIDATION;
  if (load) {
    DO_VALIDATION;
    int index;
    process(index);
    if (index == -1) {
      DO_VALIDATION;
      element = 0;
    } else {
      if (index >= size) {
        DO_VALIDATION;
        Log(blunted::e_FatalError, "EnvState", "element", "element index out of bound");
      }
      element = collection[index];
    }
  } else {
    if (element == 0) {
      DO_VALIDATION;
      int index = -1;
      process(index);
    } else {
      for (int x = 0; x < size; x++) {
        DO_VALIDATION;
        if (collection[x] == element) {
          DO_VALIDATION;
          process(x);
          return;
        }
      }
      Log(blunted::e_FatalError, "EnvState", "element", "element not found");
    }
  }
}

void EnvState::process(Team*& value) {
  DO_VALIDATION;
  void* v = value;
  process(reinterpret_cast<void**>(&teams[0]), 2, v);
  value = static_cast<Team*>(v);
}

void EnvState::process(Player*& value) {
  DO_VALIDATION;
  void* v = value;
  process(reinterpret_cast<void**>(&players[0]), players.size(), v);
  value = static_cast<Player*>(v);
}

void EnvState::process(HumanGamer*& value) {
  DO_VALIDATION;
  void* v = value;
  process(reinterpret_cast<void**>(&human_controllers[0]), human_controllers.size(), v);
  value = static_cast<HumanGamer*>(v);
}

void EnvState::process(AIControlledKeyboard*& value) {
  DO_VALIDATION;
  void* v = value;
  process(reinterpret_cast<void**>(&controllers[0]), controllers.size(), v);
  value = static_cast<AIControlledKeyboard*>(v);
}

void EnvState::process(blunted::Animation*& value) {
  DO_VALIDATION;
  void* v = value;
  process(reinterpret_cast<void**>(&animations[0]), animations.size(), v);
  value = static_cast<blunted::Animation*>(v);
}

bool EnvState::eos() {
  DO_VALIDATION;
  return pos == state.size();
}

void EnvState::SetPlayers(const std::vector<Player*>& players) {
  DO_VALIDATION;
  this->players = players;
}

void EnvState::SetHumanControllers(
    const std::vector<HumanGamer*>& controllers) {
  DO_VALIDATION;
  this->human_controllers = controllers;
}

void EnvState::SetControllers(const std::vector<AIControlledKeyboard*>& controllers) {
  DO_VALIDATION;
  this->controllers = controllers;
}

void EnvState::SetAnimations(
    const std::vector<blunted::Animation*>& animations) {
  DO_VALIDATION;
  this->animations = animations;
}

void EnvState::SetTeams(Team* team0, Team* team1) {
  DO_VALIDATION;
  this->teams.push_back(team0);
  this->teams.push_back(team1);
}

const std::string& EnvState::GetState() {
  DO_VALIDATION;
  return state;
}
