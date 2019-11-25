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

#include "src/game_env.hpp"
#include <Python.h>

#include <boost/python.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace bp = boost::python;
using namespace boost::python;
using namespace boost::interprocess;
using std::string;

class GameEnv_Python : public GameEnv {
 public:
  PyObject* get_frame_python() {
    ContextHolder c(this);
    screenshoot screen = get_frame();
    PyObject* str = PyBytes_FromStringAndSize(screen.data(), screen.size());
    return str;
  }

  PyObject* get_state_python(const std::string& to_pickle) {
    std::string state = get_state(to_pickle);
    PyObject* str = PyBytes_FromStringAndSize(state.data(), state.size());
    return str;
  }

  PyObject* set_state_python(const std::string& state) {
    std::string from_pickle = set_state(state);
    PyObject* str = PyBytes_FromStringAndSize(from_pickle.data(), from_pickle.size());
    return str;
  }

  void step_python() {
    ContextHolder c(this);
    PyThreadState* _save = NULL;
    Py_UNBLOCK_THREADS;
    step();
    Py_BLOCK_THREADS;
  }

  void render_python(bool swap_buffer) {
    ContextHolder c(this);
    PyThreadState* _save = NULL;
    Py_UNBLOCK_THREADS;
    render(swap_buffer);
    Py_BLOCK_THREADS;
  }

  void reset_python(ScenarioConfig& game_config, bool init_animation) {
    ContextHolder c(this);
    context->step = -1;
    PyThreadState* _save = NULL;
    Py_UNBLOCK_THREADS;
    GetTracker()->setDisabled(true);
    reset(game_config, init_animation);
    GetTracker()->setDisabled(false);
    Py_BLOCK_THREADS;
  }
};

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
      .def_readonly("ball_owned_team", &SharedInfo::ball_owned_team)
      .def_readonly("ball_owned_player", &SharedInfo::ball_owned_player)
      .add_property("left_controllers", &SharedInfo::left_controllers)
      .add_property("right_controllers", &SharedInfo::right_controllers)
      .def_readonly("game_mode", &SharedInfo::game_mode)
      .def_readonly("step", &SharedInfo::step);

  enum_<GameState>("GameState")
      .value("game_created", GameState::game_created)
      .value("game_initiated", GameState::game_initiated)
      .value("game_running", GameState::game_running)
      .value("game_done", GameState::game_done);

  class_<GameEnv_Python>("GameEnv")
      .def("start_game", &GameEnv_Python::start_game)
      .def("get_info", &GameEnv_Python::get_info)
      .def("get_frame", &GameEnv_Python::get_frame_python)
      .def("perform_action", &GameEnv_Python::action)
      .def("sticky_action_state", &GameEnv_Python::sticky_action_state)
      .def("step", &GameEnv_Python::step_python)
      .def("get_state", &GameEnv_Python::get_state_python)
      .def("set_state", &GameEnv_Python::set_state_python)
      .def("reset", &GameEnv_Python::reset_python)
      .def("render", &GameEnv_Python::render_python)
      .def_readwrite("config", &GameEnv_Python::scenario_config)
      .def_readwrite("game_config", &GameEnv_Python::game_config)
      .def_readwrite("state", &GameEnv_Python::state)
      .def_readwrite("waiting_for_game_count",
                     &GameEnv_Python::waiting_for_game_count)
      .def("tracker_setup", &GameEnv::tracker_setup);

  class_<Vector3>("Vector3", init<float, float, float>())
     .def("__getitem__", &Vector3::GetEnvCoord)
     .def("__setitem__", &Vector3::SetEnvCoord)
  ;

  class_<GameConfig>("GameConfig")
      .def_readwrite("render", &GameConfig::render)
      .def_readwrite("physics_steps_per_frame",
                     &GameConfig::physics_steps_per_frame);

  class_<ScenarioConfig, SHARED_PTR<ScenarioConfig>, boost::noncopyable>(
      "ScenarioConfig", no_init)
      .def("make", &ScenarioConfig::make)
      .staticmethod("make")
      .def_readwrite("ball_position", &ScenarioConfig::ball_position)
      .def_readwrite("left_team", &ScenarioConfig::left_team)
      .def_readwrite("right_team", &ScenarioConfig::right_team)
      .def_readwrite("left_agents", &ScenarioConfig::left_agents)
      .def_readwrite("right_agents", &ScenarioConfig::right_agents)
      .def_readwrite("use_magnet", &ScenarioConfig::use_magnet)
      .def_readwrite("game_engine_random_seed",
                     &ScenarioConfig::game_engine_random_seed)
      .def_readwrite("reverse_team_processing",
                     &ScenarioConfig::reverse_team_processing)
      .def_readwrite("offsides", &ScenarioConfig::offsides)
      .def_readwrite("real_time", &ScenarioConfig::real_time)
      .def_readwrite("left_team_difficulty",
                     &ScenarioConfig::left_team_difficulty)
      .def_readwrite("right_team_difficulty",
                     &ScenarioConfig::right_team_difficulty)
      .def_readwrite("deterministic", &ScenarioConfig::deterministic)
      .def_readwrite("end_episode_on_score",
                     &ScenarioConfig::end_episode_on_score)
      .def_readwrite("end_episode_on_possession_change",
                     &ScenarioConfig::end_episode_on_possession_change)
      .def_readwrite("end_episode_on_out_of_play",
                     &ScenarioConfig::end_episode_on_out_of_play)
      .def_readwrite("game_duration", &ScenarioConfig::game_duration);

  class_<std::vector<FormationEntry> >("FormationEntryVec").def(
      vector_indexing_suite<std::vector<FormationEntry> >());

  class_<FormationEntry>("FormationEntry",
                         init<float, float, e_PlayerRole, bool, bool>())
      .def_readonly("role", &FormationEntry::role)
      .add_property("position", &FormationEntry::position_env)
      .def_readwrite("lazy", &FormationEntry::lazy)
      .def_readwrite("controllable", &FormationEntry::controllable);

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
