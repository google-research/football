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

#ifndef _AI_AI
#define _AI_AI

#include "../onthepitch/match.hpp"
#include "../gamedefines.hpp"
#include "gfootball_actions.h"
#include <Python.h>

#include <boost/python.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "main.hpp"


namespace bp = boost::python;

class AIControlledKeyboard;
class GameTask;

typedef std::vector<std::string> StringVector;

// Game environment. This is the class that can be used directly from Python.
struct GameEnv {
  ~GameEnv();
  // Start the game (in separate process).
  std::string start_game(GameConfig game_config);

  // Get the current state of the game (observation).
  SharedInfo get_info();

  // Get the current rendered frame.
  PyObject* get_frame();

  // Executes the action inside the game.
  void action(int action, bool left_team, int player);
  void reset(ScenarioConfig game_config);
  void step();

  private:
  void do_step(int count = 1);
  void getObservations();

  AIControlledKeyboard* keyboard_;
  GameTask* game_;
  bool disable_graphics_ = false;
  int last_step_rendered_frames_ = 1;
};

#endif
