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

#ifndef _HPP_MAIN
#define _HPP_MAIN

#include "ai/ai_keyboard.hpp"
#include "blunted.hpp"

#include "gametask.hpp"
#include "menu/menutask.hpp"
#include "hid/ihidevice.hpp"

#include "systems/graphics/graphics_system.hpp"
#include "synchronizationTask.hpp"
#include "managers/systemmanager.hpp"
#include "managers/scenemanager.hpp"
#include "scene/objectfactory.hpp"
#include "loaders/aseloader.hpp"
#include "loaders/imageloader.hpp"
#include "base/properties.hpp"
#include <boost/random.hpp>


enum e_RenderingMode {
  e_Disabled,
  e_Onscreen,
  e_Offscreen
};

struct GameConfig {
  // Should game render in high quality.
  bool high_quality = false;
  // Is rendering enabled.
  e_RenderingMode render_mode = e_Onscreen;
  // Directory with textures and other resources.
  std::string data_dir;
  // How many physics animation steps are done per single environment step.
  int physics_steps_per_frame = 10;
  std::string updatePath(const std::string& path) {
    if (path[0] == '/') {
      return path;
    }
    return data_dir + '/' + path;
  }

};

struct ScenarioConfig {
  // Start ball position.
  Vector3 ball_position;
  // Initial configuration of left team.
  std::vector<FormationEntry> left_team;
  // Initial configuration of right team.
  std::vector<FormationEntry> right_team;
  // How many left team players are controlled externally.
  int left_agents = 1;
  // How many right team players are controlled externally.
  int right_agents = 0;
  // Whether to use magnet logic (that automatically pushes active player
  // towards the ball).
  bool use_magnet = true;
  // Are offsides enabled.
  bool offsides = true;
  // Should game run in "real time", ie. aiming at 100 physics animations
  // or full speed otherwise.
  bool real_time = false;
  // Seed to use for random generators.
  unsigned int game_engine_random_seed = 42;
  // Should players in both teams be identical.
  bool symmetrical_teams = true;
  // Is rendering enabled.
  bool render = true;
  // Computer AI difficulty level, from 0.0 to 1.0.
  float game_difficulty = 0.8;
  // Should the kickoff start with the team loosing a goal to own the ball.
  bool kickoff_for_goal_loosing_team = false;

  bool LeftTeamOwnsBall() {
    float leftDistance = 1000000;
    float rightDistance = 1000000;
    for (auto& player : left_team) {
      leftDistance =std::min(leftDistance,
          (player.start_position - ball_position).GetLength());
    }
    for (auto& player : right_team) {
      rightDistance = std::min(rightDistance,
          (player.start_position - ball_position).GetLength());
    }
    return leftDistance < rightDistance;
  }
};

struct GameContext {
  GameContext() : rng(BaseGenerator(), Distribution()), rng_non_deterministic(BaseGenerator(), Distribution()) {}
  GraphicsSystem *graphicsSystem = nullptr;
  boost::shared_ptr<GameTask> gameTask;
  boost::shared_ptr<MenuTask> menuTask;
  boost::shared_ptr<TaskSequence> gameSequence;
  boost::shared_ptr<TaskSequence> graphicsSequence;
  boost::shared_ptr<Scene2D> scene2D;
  boost::shared_ptr<Scene3D> scene3D;
  boost::shared_ptr<SynchronizationTask> synchronizationTask;

  Properties *config = nullptr;
  ScenarioConfig scenario_config;
  GameConfig game_config;
  std::string font;
  TTF_Font *defaultFont = nullptr;
  TTF_Font *defaultOutlineFont = nullptr;

  std::vector<IHIDevice*> controllers;
  SystemManager system_manager;
  ObjectFactory object_factory;
  EnvironmentManager environment_manager;
  ResourceManager<GeometryData> geometry_manager;
  ResourceManager<Surface> surface_manager;
  ResourceManager<Texture> texture_manager;
  ResourceManager<VertexBuffer> vertices_manager;
  ASELoader aseLoader;
  ImageLoader imageLoader;
  Scheduler scheduler;
  SceneManager scene_manager;

  typedef boost::mt19937 BaseGenerator;
  typedef boost::uniform_real<float> Distribution;
  typedef boost::variate_generator<BaseGenerator, Distribution> Generator;
  Generator rng;

  // Two random number generators are needed. One (deterministic when running
  // in deterministic mode) to be used in places which generate deterministic
  // game state. Second one is used in places which are optional and don't
  // affect observations (like position of the sun).
  Generator rng_non_deterministic;
  bool already_loaded = false;
  int playerCount = 0;
  int stablePlayerCount = 0;
  BiasedOffsets emptyOffsets;
  boost::shared_ptr<AnimCollection> anims;
  std::map<Animation*, std::vector<Vector3>> animPositionCache;
  std::map<Vector3, Vector3> colorCoords;
};

class Match;

void SetContext(GameContext* c);
GameContext& GetContext();
boost::shared_ptr<Scene2D> GetScene2D();
boost::shared_ptr<Scene3D> GetScene3D();
GraphicsSystem *GetGraphicsSystem();
boost::shared_ptr<GameTask> GetGameTask();
boost::shared_ptr<MenuTask> GetMenuTask();
boost::shared_ptr<SynchronizationTask> GetSynchronizationTask();

Properties *GetConfiguration();
SystemManager* GetSystemManager();
ScenarioConfig& GetScenarioConfig();
GameConfig& GetGameConfig();

const std::vector<IHIDevice*> &GetControllers();

void run_game(Properties* input_config);
void randomize(unsigned int seed);
void quit_game();
int main(int argc, char** argv);
void set_rendering(bool);
#endif
