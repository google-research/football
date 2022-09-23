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

class GameEnv;
class Tracker;
Tracker* GetTracker();
GameEnv* GetGame();

void DoValidation(int line, const char* file);

// Uncomment line below to enable validation.
// #define FULL_VALIDATION 1
#ifdef FULL_VALIDATION
  #define DO_VALIDATION DoValidation(__LINE__, __FILE__);
#else
#define DO_VALIDATION ;
#endif

#include "ai/ai_keyboard.hpp"
#include "blunted.hpp"

#include "gametask.hpp"
#include "menu/menutask.hpp"
#include "hid/ihidevice.hpp"

#include "systems/graphics/graphics_system.hpp"
#include "scene/objectfactory.hpp"
#include "loaders/aseloader.hpp"
#include "loaders/imageloader.hpp"
#include "base/properties.hpp"
#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <condition_variable>
#include <mutex>

#define SHARED_PTR boost::shared_ptr
#define WEAK_PTR boost::weak_ptr

enum e_RenderingMode {
  e_Disabled,
  e_Onscreen,
  e_Offscreen
};

class GameConfig {
 public:
  static SHARED_PTR<GameConfig> make() {
    return SHARED_PTR<GameConfig>(new GameConfig());
  }
  // Is rendering enabled.
  bool render = false;
  // Directory with textures and other resources.
  std::string data_dir;
  // How many physics animation steps are done per single environment step.
  int physics_steps_per_frame = 10;
  int render_resolution_x = 1280;
  int render_resolution_y = 720;
  std::string updatePath(const std::string& path) {
#ifdef WIN32
    boost::filesystem::path boost_path(path);
    if (boost_path.is_absolute()) {
      return path;
    }
    boost::filesystem::path data_dir_boost(data_dir);
    data_dir_boost /= boost_path;
    return data_dir_boost.string();
#else
    if (path[0] == '/') {
      return path;
    }
    return data_dir + '/' + path;
#endif
  }
  void ProcessState(EnvState* state) {
    state->process(data_dir);
    state->process(physics_steps_per_frame);
    state->process(render_resolution_x);
    state->process(render_resolution_y);
  }
 private:
  GameConfig() { }
  friend GameEnv;
};

struct ScenarioConfig {
 public:
  static SHARED_PTR<ScenarioConfig> make() {
    return SHARED_PTR<ScenarioConfig>(new ScenarioConfig());
  }
  bool DynamicPlayerSelection() {
    ComputeCache();
    return cached_dynamic_player_selection;
  }
  int ControllableLeftPlayers() {
    ComputeCache();
    return cached_controllable_left_players;
  }
  int ControllableRightPlayers() {
    ComputeCache();
    return cached_controllable_right_players;
  }
  bool LeftTeamOwnsBall() { DO_VALIDATION;
    float leftDistance = 1000000;
    float rightDistance = 1000000;
    for (auto& player : left_team) { DO_VALIDATION;
      leftDistance = std::min(leftDistance,
          (player.start_position - ball_position).GetLength());
    }
    for (auto& player : right_team) { DO_VALIDATION;
      rightDistance = std::min(rightDistance,
          (player.start_position - ball_position).GetLength());
    }
    return leftDistance < rightDistance;
  }
  void ProcessStateConstant(EnvState* state) {
    cache_computed = false;
    state->process(ball_position);
    int size = left_team.size();
    state->process(size);
    left_team.resize(size);
    size = right_team.size();
    state->process(size);
    right_team.resize(size);
    state->process(left_agents);
    state->process(right_agents);
    state->process(use_magnet);
    state->process(offsides);
    state->process(left_team_difficulty);
    state->process(right_team_difficulty);
    state->process(deterministic);
    state->process(end_episode_on_score);
    state->process(end_episode_on_possession_change);
    state->process(end_episode_on_out_of_play);
    state->process(game_duration);
    state->process(second_half);
    state->process(control_all_players);
  }

  void ProcessState(EnvState* state) {
    cache_computed = false;
    state->process(real_time);
    state->process(game_engine_random_seed);
    state->process(reverse_team_processing);
    for (auto& p : left_team) {
      p.ProcessState(state);
    }
    for (auto& p : right_team) {
      p.ProcessState(state);
    }
  }
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
  // Reverse order of teams' processing, used for symmetry testing.
  bool reverse_team_processing = false;
  // Left team AI difficulty level, from 0.0 to 1.0.
  float left_team_difficulty = 1.0;
  // Right team AI difficulty level, from 0.0 to 1.0.
  float right_team_difficulty = 0.6;
  bool deterministic = false;
  bool end_episode_on_score = false;
  bool end_episode_on_possession_change = false;
  bool end_episode_on_out_of_play = false;
  int game_duration = 3000;
  bool control_all_players = false;
  int second_half = 999999999;

 private:
  ScenarioConfig() { }
  void ComputeCache() {
    if (cache_computed) {
      return;
    }
    cached_controllable_left_players = 0;
    cached_controllable_right_players = 0;
    for (auto& p : left_team) {
      if (p.controllable) {
        cached_controllable_left_players++;
      }
    }
    for (auto& p : right_team) {
      if (p.controllable) {
        cached_controllable_right_players++;
      }
    }
    cached_dynamic_player_selection =
        !((cached_controllable_left_players == left_agents || left_agents == 0) &&
        (cached_controllable_right_players == right_agents || right_agents == 0));
    cache_computed = true;
  }
  int cached_controllable_left_players = -1;
  int cached_controllable_right_players = -1;
  bool cached_dynamic_player_selection = false;
  bool cache_computed = false;
  friend GameEnv;
};

enum GameState {
  game_created,
  game_initiated,
  game_running,
  game_done
};

class GameContext {
 public:
  GameContext() : rng(BaseGenerator(), Distribution()), rng_non_deterministic(BaseGenerator(), Distribution()) { }
  GraphicsSystem graphicsSystem;
  boost::shared_ptr<GameTask> gameTask;
  boost::shared_ptr<MenuTask> menuTask;
  boost::shared_ptr<Scene2D> scene2D;
  boost::shared_ptr<Scene3D> scene3D;
  boost::intrusive_ptr<Node> fullbodyNode;
  boost::intrusive_ptr<Node> goalsNode;
  boost::intrusive_ptr<Node> stadiumRender;
  boost::intrusive_ptr<Node> stadiumNoRender;
  Properties *config = nullptr;
  std::string font;
  TTF_Font *defaultFont = nullptr;
  TTF_Font *defaultOutlineFont = nullptr;

  std::vector<AIControlledKeyboard*> controllers;
  ObjectFactory object_factory;
  ResourceManager<GeometryData> geometry_manager;
  ResourceManager<Surface> surface_manager;
  ResourceManager<Texture> texture_manager;
  ResourceManager<VertexBuffer> vertices_manager;
  ASELoader aseLoader;
  ImageLoader imageLoader;

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
  int step = 0;
  int tracker_disabled = 1;
  long tracker_pos = 0;
  void ProcessState(EnvState* state);
};

class Match;

void SetGame(GameEnv* c);
GameContext& GetContext();
boost::shared_ptr<Scene2D> GetScene2D();
boost::shared_ptr<Scene3D> GetScene3D();
GraphicsSystem *GetGraphicsSystem();
boost::shared_ptr<GameTask> GetGameTask();
boost::shared_ptr<MenuTask> GetMenuTask();

Properties *GetConfiguration();
ScenarioConfig& GetScenarioConfig();
GameConfig& GetGameConfig();

const std::vector<AIControlledKeyboard*> &GetControllers();

void run_game(Properties* input_config, bool render);
void randomize(unsigned int seed);
void quit_game();
#ifndef WIN32
int main(int argc, char** argv);
#endif

class Tracker {
 public:
  void setup(long start, long end) {
    this->start = start;
    this->end = end;
    GetContext().tracker_disabled = 0;
    GetContext().tracker_pos = 0;
  }
  void setDisabled(bool disabled) {
    GetContext().tracker_disabled += disabled ? 1 : -1;
  }
  bool enabled() {
    return GetContext().tracker_disabled == 0;
  }
  inline void verify(int line, const char* file) {
    if (GetContext().tracker_disabled) return;
    GetContext().tracker_pos++;
    if (GetContext().tracker_pos < start || GetContext().tracker_pos > end) return;
    std::unique_lock<std::mutex> lock(mtx);
    std::string trace;
    if (waiting_game == nullptr) {
      if (GetContext().tracker_pos % 10000 == 0) {
        std::cout << "Validating: " << GetContext().tracker_pos << std::endl;
      }
      waiting_stack_trace = trace;
      waiting_game = GetGame();
      waiting_line = line;
      waiting_file = file;
      cv.wait(lock);
      return;
    }
    GetContext().tracker_disabled++;
    verify_snapshot(GetContext().tracker_pos, line, file, trace);
    GetContext().tracker_disabled--;
    waiting_game = nullptr;
    cv.notify_one();
  }
 private:
  void verify_snapshot(long pos, int line, const char* file, const std::string& trace);
  // Tweak start and end to verify that line numbers match for
  // each call in the verification range (2 bytes / call).
  long start = 0LL;
  long end = 1000000000LL;
  bool verify_stack_trace = true;
  std::mutex mtx;
  std::condition_variable cv;
  GameEnv* waiting_game = nullptr;
  int waiting_line;
  const char* waiting_file;
  std::string waiting_stack_trace;
};

#endif
