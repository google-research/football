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

#undef NDEBUG

#include <sys/prctl.h>
#include <ctime>
#include <iostream>
#include <fenv.h>
#include <ratio>
#include <chrono>
#include <ctime>
#include <cerrno>
#include "managers/resourcemanagerpool.hpp"

#include <sys/wait.h>
#include "ai.hpp"
#include "ai/ai_keyboard.hpp"
#include "gametask.hpp"
#include "helpers.h"


using namespace boost::python;
using namespace boost::interprocess;

#define FRAME_SIZE (1280*720*3)

boost::shared_ptr<GameTask> game_;

using std::string;

void GameEnv::do_step(int count) {
  GetSynchronizationTask()->Step(count);
  while (GetSynchronizationTask()->Steps() > 0) {
    DoStep();
  }
}

float Position::env_coord(int index) const {
  switch (index) {
    case 0:
      return value[0] / X_FIELD_SCALE;
    case 1:
      return value[1] / Y_FIELD_SCALE;
    case 2:
      return value[2] / Z_FIELD_SCALE;
    default:
      PyErr_SetString(PyExc_IndexError, "index out of range");
      throw boost::python::error_already_set();
  }
}

std::string Position::debug() {
  return std::to_string(value[0]) + "," + std::to_string(value[1]) + "," +
         std::to_string(value[2]);
}

GameEnv::~GameEnv() {
}

void setConfig(ScenarioConfig scenario_config) {
  scenario_config.ball_position.coords[0] =
      scenario_config.ball_position.coords[0] * X_FIELD_SCALE;
  scenario_config.ball_position.coords[1] =
      scenario_config.ball_position.coords[1] * Y_FIELD_SCALE;
  GetScenarioConfig() = scenario_config;
  std::vector<SideSelection> setup = GetMenuTask()->GetControllerSetup();
  CHECK(setup.size() == 2 * MAX_PLAYERS);
  int controller = 0;
  for (int x = 0; x < scenario_config.left_agents; x++) {
    setup[controller++].side = -1;
  }
  while (controller < MAX_PLAYERS) {
    setup[controller++].side = 0;
  }
  for (int x = 0; x < scenario_config.right_agents; x++) {
    setup[controller++].side = 1;
  }
  while (controller < 2 * MAX_PLAYERS) {
    setup[controller++].side = 0;
  }
  GetMenuTask()->SetControllerSetup(setup);
}

std::string GameEnv::start_game(GameConfig game_config) {
  // feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
  GetGameConfig() = game_config;
  std::cout << std::unitbuf;
  string data_dir = FindDataDir();
  if (!data_dir.empty()) {
    GetGameConfig().data_dir = data_dir;
  }
  string font_file = FindFontFile();
  if (font_file.empty()) {
    font_file = game_config.font_file;
  }
  Properties* config = new Properties();
  config->Set("match_duration", 0.027);
  config->Set("font_filename", font_file);
  config->Set("physics_frametime_ms", 1);
  config->Set("game", 0);
  // Enable AI.
  config->SetBool("ai_keyboard", true);
  // Disable Audio (not needed and sometimes causes trouble).
  config->SetBool("disable_audio", true);
  if (game_config.render_mode == e_Disabled) {
    config->Set("graphics3d_renderer", "mock");
  } else if (game_config.render_mode == e_Offscreen) {
    setenv("DISPLAY", ":63", 1);
    config->Set("graphics3d_renderer", "egl");
  }
  run_game(config);
  game_ = GetGameTask().get();
  setConfig(ScenarioConfig());
  do_step(1);
  return "ok";
}

SharedInfo GameEnv::get_info() {
  Match* match = game_->GetMatch();
  CHECK(match);
  SharedInfo info;
  match->GetState(&info);
  auto graphics = GetGraphicsSystem();
  return info;
}

PyObject* GameEnv::get_frame() {
  const screenshoot& screen = GetGraphicsSystem()->GetScreen();
  PyObject* str = PyBytes_FromStringAndSize(screen.data(), screen.size());
  return str;
}

void GameEnv::action(int action, bool left_team, int player) {
  int controller_id = player + (left_team ? 0 : 11);
  auto controller = static_cast<AIControlledKeyboard*>(GetControllers()[controller_id]);
  switch (Action(action)) {
    case game_idle:
      break;
    case game_left:
      controller->SetDirection(Vector3(-1, 0, 0));
      break;
    case game_top_left:
      controller->SetDirection(Vector3(-1, 1, 0));
      break;
    case game_top:
      controller->SetDirection(Vector3(0, 1, 0));
      break;
    case game_top_right:
      controller->SetDirection(Vector3(1, 1, 0));
      break;
    case game_right:
      controller->SetDirection(Vector3(1, 0, 0));
      break;
    case game_bottom_right:
      controller->SetDirection(Vector3(1, -1, 0));
      break;
    case game_bottom:
      controller->SetDirection(Vector3(0, -1, 0));
      break;
    case game_bottom_left:
      controller->SetDirection(Vector3(-1, -1, 0));
      break;

    case game_long_pass:
      controller->SetButton(e_ButtonFunction_LongPass, true);
      break;
    case game_high_pass:
      controller->SetButton(e_ButtonFunction_HighPass, true);
      break;
    case game_short_pass:
      controller->SetButton(e_ButtonFunction_ShortPass, true);
      break;
    case game_shot:
      controller->SetButton(e_ButtonFunction_Shot, true);
      break;
    case game_keeper_rush:
      controller->SetButton(e_ButtonFunction_KeeperRush, true);
      break;
    case game_sliding:
      controller->SetButton(e_ButtonFunction_Sliding, true);
      break;
    case game_pressure:
      controller->SetButton(e_ButtonFunction_Pressure, true);
      break;
    case game_team_pressure:
      controller->SetButton(e_ButtonFunction_TeamPressure, true);
      break;
    case game_switch:
      controller->SetButton(e_ButtonFunction_Switch, true);
      break;
    case game_sprint:
      controller->SetButton(e_ButtonFunction_Sprint, true);
      break;
    case game_dribble:
      controller->SetButton(e_ButtonFunction_Dribble, true);
      break;
    case game_release_direction:
      controller->SetDirection(Vector3(0, 0, 0));
      break;
    case game_release_long_pass:
      controller->SetButton(e_ButtonFunction_LongPass, false);
      break;
    case game_release_high_pass:
      controller->SetButton(e_ButtonFunction_HighPass, false);
      break;
    case game_release_short_pass:
      controller->SetButton(e_ButtonFunction_ShortPass, false);
      break;
    case game_release_shot:
      controller->SetButton(e_ButtonFunction_Shot, false);
      break;
    case game_release_keeper_rush:
      controller->SetButton(e_ButtonFunction_KeeperRush, false);
      break;
    case game_release_sliding:
      controller->SetButton(e_ButtonFunction_Sliding, false);
      break;
    case game_release_pressure:
      controller->SetButton(e_ButtonFunction_Pressure, false);
      break;
    case game_release_team_pressure:
      controller->SetButton(e_ButtonFunction_TeamPressure, false);
      break;
    case game_release_switch:
      controller->SetButton(e_ButtonFunction_Switch, false);
      break;
    case game_release_sprint:
      controller->SetButton(e_ButtonFunction_Sprint, false);
      break;
    case game_release_dribble:
      controller->SetButton(e_ButtonFunction_Dribble, false);
      break;
  }
}

void GameEnv::step() {
  // We do 10 environment steps per second, while game does 100 frames of
  // physics animation.
  int steps_to_do = GetGameConfig().physics_steps_per_frame;
  if (GetScenarioConfig().real_time) {
    auto start = std::chrono::system_clock::now();
    for (int x = 1; x <= steps_to_do; x++) {
      bool render_current_step =
          x * last_step_rendered_frames_ / steps_to_do !=
          (x - 1) * last_step_rendered_frames_ / steps_to_do;
      set_rendering(render_current_step);
      do_step(1);
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start);
    if (elapsed.count() > 9 * (steps_to_do + 1) &&
        last_step_rendered_frames_ > 1) {
      last_step_rendered_frames_--;
    } else if (elapsed.count() < 9 * (steps_to_do - 1) &&
               last_step_rendered_frames_ < steps_to_do) {
      last_step_rendered_frames_++;
    }
  } else {
    set_rendering(false);
    do_step(steps_to_do - 1);
    set_rendering(GetScenarioConfig().render);
    do_step(1);
  }
}

void GameEnv::reset(ScenarioConfig game_config) {
  setConfig(game_config);
  for (auto controller : GetControllers()) {
    controller->Reset();
  }
  ResourceManagerPool::CleanUp();
  GetMenuTask()->SetMenuAction(e_MenuAction_Menu);
  do_step(1);
}

BOOST_PYTHON_MODULE(_gameplayfootball) {
  class_<std::vector<float> >("FloatVec")
      .def(vector_indexing_suite<std::vector<float> >());

  class_<std::vector<int> >("IntVec")
      .def(vector_indexing_suite<std::vector<int> >());

  class_<std::vector<PlayerInfo> >("PlayerInfoVec")
      .def(vector_indexing_suite<std::vector<PlayerInfo> >());

  class_<StringVector>("StringVector")
      .def(boost::python::vector_indexing_suite<StringVector>());

  class_<Position>("Position")
      .def("__getitem__", &Position::env_coord)
      .def("__str__", &Position::debug);

  class_<PlayerInfo>("PlayerInfo")
      .def_readonly("position", &PlayerInfo::player_position)
      .def_readonly("direction", &PlayerInfo::player_direction)
      .def_readonly("tired_factor", &PlayerInfo::tired_factor)
      .def_readonly("has_card", &PlayerInfo::has_card)
      .def_readonly("is_active", &PlayerInfo::is_active)
      .def_readonly("role", &PlayerInfo::role);

  class_<ControllerInfo>("ControllerInfo", init<int>())
      .add_property("controlled_player", &ControllerInfo::controlled_player);

  class_<std::vector<ControllerInfo> >("ControllerInfoVec").def(
      vector_indexing_suite<std::vector<ControllerInfo> >());

  class_<SharedInfo>("SharedInfo")
      .def_readonly("ball_position", &SharedInfo::ball_position)
      .def_readonly("ball_rotation", &SharedInfo::ball_rotation)
      .def_readonly("ball_direction", &SharedInfo::ball_direction)
      .def_readonly("left_team", &SharedInfo::left_team)
      .def_readonly("right_team", &SharedInfo::right_team)
      .def_readonly("left_goals", &SharedInfo::left_goals)
      .def_readonly("right_goals", &SharedInfo::right_goals)
      .def_readonly("is_in_play", &SharedInfo::is_in_play)
      .def_readonly("done", &SharedInfo::done)
      .def_readonly("ball_owned_team", &SharedInfo::ball_owned_team)
      .def_readonly("ball_owned_player", &SharedInfo::ball_owned_player)
      .add_property("left_controllers", &SharedInfo::left_controllers)
      .add_property("right_controllers", &SharedInfo::right_controllers)
      .def_readonly("game_mode", &SharedInfo::game_mode);

  class_<GameEnv>("GameEnv")
      .def("start_game", &GameEnv::start_game)
      .def("get_info", &GameEnv::get_info)
      .def("get_frame", &GameEnv::get_frame)
      .def("perform_action", &GameEnv::action)
      .def("step", &GameEnv::step)
      .def("reset", &GameEnv::reset);
  ;

  class_<Vector3>("Vector3", init<float, float, float>())
     .def("__getitem__", &Vector3::GetEnvCoord)
     .def("__setitem__", &Vector3::SetEnvCoord)
  ;

  class_<GameConfig>("GameConfig")
      .def_readwrite("high_quality", &GameConfig::high_quality)
      .def_readwrite("render_mode", &GameConfig::render_mode)
      .def_readwrite("data_dir", &GameConfig::data_dir)
      .def_readwrite("font_file", &GameConfig::font_file)
      .def_readwrite("physics_steps_per_frame",
                     &GameConfig::physics_steps_per_frame);

  class_<ScenarioConfig>("ScenarioConfig")
      .def_readwrite("ball_position", &ScenarioConfig::ball_position)
      .def_readwrite("left_team", &ScenarioConfig::left_team)
      .def_readwrite("right_team", &ScenarioConfig::right_team)
      .def_readwrite("left_agents", &ScenarioConfig::left_agents)
      .def_readwrite("right_agents", &ScenarioConfig::right_agents)
      .def_readwrite("use_magnet", &ScenarioConfig::use_magnet)
      .def_readwrite("game_engine_random_seed",
                     &ScenarioConfig::game_engine_random_seed)
      .def_readwrite("offsides", &ScenarioConfig::offsides)
      .def_readwrite("real_time", &ScenarioConfig::real_time)
      .def_readwrite("render", &ScenarioConfig::render)
      .def_readwrite("game_difficulty", &ScenarioConfig::game_difficulty)
      .def_readwrite("kickoff_for_goal_loosing_team",
                     &ScenarioConfig::kickoff_for_goal_loosing_team);

  class_<std::vector<FormationEntry> >("FormationEntryVec").def(
      vector_indexing_suite<std::vector<FormationEntry> >());

  class_<FormationEntry>("FormationEntry",
                         init<float, float, e_PlayerRole, bool>())
      .def_readonly("role", &FormationEntry::role)
      .add_property("position", &FormationEntry::position_env)
      .def_readwrite("lazy", &FormationEntry::lazy);

  enum_<e_PlayerRole>("e_PlayerRole")
      .value("e_PlayerRole_GK", e_PlayerRole::e_PlayerRole_GK)
      .value("e_PlayerRole_CB", e_PlayerRole::e_PlayerRole_CB)
      .value("e_PlayerRole_LB", e_PlayerRole::e_PlayerRole_LB)
      .value("e_PlayerRole_RB", e_PlayerRole::e_PlayerRole_RB)
      .value("e_PlayerRole_DM", e_PlayerRole::e_PlayerRole_DM)
      .value("e_PlayerRole_CM", e_PlayerRole::e_PlayerRole_CM)
      .value("e_PlayerRole_LM", e_PlayerRole::e_PlayerRole_LM)
      .value("e_PlayerRole_RM", e_PlayerRole::e_PlayerRole_RM)
      .value("e_PlayerRole_AM", e_PlayerRole::e_PlayerRole_AM)
      .value("e_PlayerRole_CF", e_PlayerRole::e_PlayerRole_CF);

  enum_<e_RenderingMode>("e_RenderingMode")
      .value("e_Disabled", e_RenderingMode::e_Disabled)
      .value("e_Onscreen", e_RenderingMode::e_Onscreen)
      .value("e_Offscreen", e_RenderingMode::e_Offscreen);

  enum_<e_GameMode>("e_GameMode")
      .value("e_GameMode_Normal", e_GameMode::e_GameMode_Normal)
      .value("e_GameMode_KickOff", e_GameMode::e_GameMode_KickOff)
      .value("e_GameMode_GoalKick", e_GameMode::e_GameMode_GoalKick)
      .value("e_GameMode_FreeKick", e_GameMode::e_GameMode_FreeKick)
      .value("e_GameMode_Corner", e_GameMode::e_GameMode_Corner)
      .value("e_GameMode_ThrowIn", e_GameMode::e_GameMode_ThrowIn)
      .value("e_GameMode_Penalty", e_GameMode::e_GameMode_Penalty);

  enum_<Action>("e_BackendAction")
    .value("idle", Action::game_idle)
    .value("left", Action::game_left)
    .value("top_left", Action::game_top_left)
    .value("top", Action::game_top)
    .value("top_right", Action::game_top_right)
    .value("right", Action::game_right)
    .value("bottom_right", Action::game_bottom_right)
    .value("bottom", Action::game_bottom)
    .value("bottom_left", Action::game_bottom_left)
    .value("long_pass", Action::game_long_pass)
    .value("high_pass", Action::game_high_pass)
    .value("short_pass", Action::game_short_pass)
    .value("shot", Action::game_shot)
    .value("keeper_rush", Action::game_keeper_rush)
    .value("sliding", Action::game_sliding)
    .value("pressure", Action::game_pressure)
    .value("team_pressure", Action::game_team_pressure)
    .value("switch", Action::game_switch)
    .value("sprint", Action::game_sprint)
    .value("dribble", Action::game_dribble)
    .value("release_direction", Action::game_release_direction)
    .value("release_long_pass", Action::game_release_long_pass)
    .value("release_high_pass", Action::game_release_high_pass)
    .value("release_short_pass", Action::game_release_short_pass)
    .value("release_shot", Action::game_release_shot)
    .value("release_keeper_rush", Action::game_release_keeper_rush)
    .value("release_sliding", Action::game_release_sliding)
    .value("release_pressure", Action::game_release_pressure)
    .value("release_team_pressure", Action::game_release_team_pressure)
    .value("release_switch", Action::game_release_switch)
    .value("release_sprint", Action::game_release_sprint)
    .value("release_dribble", Action::game_release_dribble);

  enum_<e_Team>("e_Team")
      .value("e_Left", e_Team::e_Left)
      .value("e_Right", e_Team::e_Right);
}
