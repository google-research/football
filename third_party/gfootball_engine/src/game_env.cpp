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

#include "game_env.hpp"

#include <fenv.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>

#include "ai/ai_keyboard.hpp"
#include "file.h"
#include "gametask.hpp"

#define FRAME_SIZE (1280*720*3)

using std::string;

void GameEnv::do_step(int count, bool render) {
  while (!context->gameTask->GetMatch() || count--) {
    context->menuTask->ProcessPhase();
    context->gameTask->ProcessPhase();
    if (render) {
      context->graphicsSystem->GetTask()->GetPhase();
      context->graphicsSystem->GetTask()->ProcessPhase();
    }
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
      Log(e_FatalError, "football", "main", "index out of range");
      return 0;
  }
}

std::string Position::debug() {
  return std::to_string(value[0]) + "," + std::to_string(value[1]) + "," +
         std::to_string(value[2]);
}

GameEnv::~GameEnv() { }

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
  std::cout.precision(17);
  context = new GameContext();
  SetContext(context);
  // feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
  GetGameConfig() = game_config;
  std::cout << std::unitbuf;

  char* data_dir = getenv("GFOOTBALL_DATA_DIR");
  if (data_dir) {
    GetGameConfig().data_dir = data_dir;
  }
  Properties* config = new Properties();
  config->Set("match_duration", 0.027);
  char* font_file = getenv("GFOOTBALL_FONT");
  if (font_file) {
    config->Set("font_filename", font_file);
  }
  config->Set("game", 0);
  // Enable AI.
  config->SetBool("ai_keyboard", true);
  if (game_config.render_mode == e_Disabled) {
    config->Set("graphics3d_renderer", "mock");
  } else if (game_config.render_mode == e_Offscreen) {
    setenv("DISPLAY", ":63", 1);
    config->Set("graphics3d_renderer", "egl");
  }
  run_game(config);
  setConfig(ScenarioConfig());
  do_step(1, GetScenarioConfig().render);
  return "ok";
}

SharedInfo GameEnv::get_info() {
  SharedInfo info;
  GetGameTask()->GetMatch()->GetState(&info);
  info.step = context->step;
  auto graphics = GetGraphicsSystem();
  return info;
}

screenshoot GameEnv::get_frame() {
  SetContext(context);
  return GetGraphicsSystem()->GetScreen();
}

void GameEnv::action(int action, bool left_team, int player) {
  SetContext(context);
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

string GameEnv::get_state() {
  SetContext(context);
  EnvState reader("");
  reader.process(context->step);
  ProcessState(&reader);
  return reader.GetState();
}

void GameEnv::set_state(const std::string& state) {
  SetContext(context);
  EnvState writer(state);
  writer.process(context->step);
  ProcessState(&writer);
}


void GameEnv::step() {
  SetContext(context);
  // We do 10 environment steps per second, while game does 100 frames of
  // physics animation.
  int steps_to_do = GetGameConfig().physics_steps_per_frame;
  if (GetScenarioConfig().real_time) {
    auto start = std::chrono::system_clock::now();
    for (int x = 1; x <= steps_to_do; x++) {
      bool render_current_step =
          x * last_step_rendered_frames_ / steps_to_do !=
          (x - 1) * last_step_rendered_frames_ / steps_to_do;
      do_step(1, render_current_step);
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
    do_step(steps_to_do - 1, false);
    do_step(1, GetScenarioConfig().render);
  }
  if (context->gameTask->GetMatch()->IsInPlay()) {
    context->step++;
  }
}

void GameEnv::reset(const ScenarioConfig& game_config) {
  SetContext(context);
  context->step = -1;
  SetContext(context);
  setConfig(game_config);
  for (auto controller : GetControllers()) {
    controller->Reset();
  }
  context->geometry_manager.RemoveUnused();
  context->surface_manager.RemoveUnused();
  context->texture_manager.RemoveUnused();
  context->vertices_manager.RemoveUnused();
  GetMenuTask()->SetMenuAction(e_MenuAction_Menu);
  for (int x = 0; x < 2; x++) {
    context->menuTask->ProcessPhase();
    context->gameTask->ProcessPhase();
    if (GetScenarioConfig().render) {
      context->graphicsSystem->GetTask()->GetPhase();
      context->graphicsSystem->GetTask()->ProcessPhase();
    }
  }
}
