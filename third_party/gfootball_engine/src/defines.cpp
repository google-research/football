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
#include "main.hpp"

EnvState::EnvState(const std::string& state, const std::string reference)
    : load(!state.empty()), state(state), reference(reference) {}

void EnvState::process(unsigned long &value) {
  process(&value, sizeof(value));
}

void EnvState::process(unsigned int &value) {
  process(&value, sizeof(value));
}

void EnvState::process(bool &value) {
  process(&value, sizeof(value));
}

void EnvState::process(blunted::Vector3 &value) {
  process(&value, sizeof(value));
}

void EnvState::process(blunted::radian &value) {
  process(&value, sizeof(value));
}

void EnvState::process(blunted::Quaternion &value) {
  process(&value, sizeof(value));
}

void EnvState::process(int &value) {
  process(&value, sizeof(value));
}

void EnvState::process(std::string &value) {
  int s = value.size();
  process(s);
  value.resize(s);
  process(&value[0], s);
}

void EnvState::process(float &value) {
  process(&value, sizeof(value));
}

void EnvState::process(void** collection, int size, void*& element) {
  if (load) {
    int index;
    process(&index, sizeof(int));
    if (index == -1) {
      element = 0;
    } else {
      if (index >= size) {
        Log(blunted::e_FatalError, "EnvState", "element", "element index out of bound");
      }
      element = collection[index];
    }
  } else {
    if (element == 0) {
      int index = -1;
      process(&index, sizeof(int));
    } else {
      for (int x = 0; x < size; x++) {
        if (collection[x] == element) {
          process(&x, sizeof(int));
          return;
        }
      }
      Log(blunted::e_FatalError, "EnvState", "element", "element not found");
    }
  }
}

void EnvState::process(Team*& value) {
  void* v = value;
  process(reinterpret_cast<void**>(&teams[0]), 2, v);
  value = static_cast<Team*>(v);
}

void EnvState::process(Player*& value) {
  void* v = value;
  process(reinterpret_cast<void**>(&players[0]), players.size(), v);
  value = static_cast<Player*>(v);
}

void EnvState::process(HumanController*& value) {
  void* v = value;
  process(reinterpret_cast<void**>(&controllers[0]), controllers.size(), v);
  value = static_cast<HumanController*>(v);
}

void EnvState::process(blunted::Animation*& value) {
  void* v = value;
  process(reinterpret_cast<void**>(&animations[0]), animations.size(), v);
  value = static_cast<blunted::Animation*>(v);
}

void EnvState::process(void* ptr, int size) {
  if (load) {
    if (pos + size > state.size()) {
      Log(blunted::e_FatalError, "EnvState", "state", "state is invalid");
    }
    memcpy(ptr, &state[pos], size);
    pos += size;
  } else {
    state.resize(pos + size);
    memcpy(&state[pos], ptr, size);
    if (validate && !reference.empty() && memcmp(&state[pos], &reference[pos],
                                                 size)) {
      Log(blunted::e_FatalError, "EnvState", "state", "Reference mismatch");
    }
    pos += size;
    if (pos > 10000000) {
      Log(blunted::e_FatalError, "EnvState", "state", "state is too big");
    }
  }
}

void EnvState::SetPlayers(const std::vector<Player*>& players) {
  this->players = players;
}

void EnvState::SetHumanControllers(const std::vector<HumanController*>& controllers) {
  this->controllers = controllers;
}

void EnvState::SetAnimations(const std::vector<blunted::Animation*>& animations) {
  this->animations = animations;
}

void EnvState::SetTeams(Team* team0, Team* team1) {
  this->teams.push_back(team0);
  this->teams.push_back(team1);
}

const std::string& EnvState::GetState() {
  return state;
}
