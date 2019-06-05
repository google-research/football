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

#include "base/properties.hpp"

#include "utils/database.hpp"
#include <sqlite3.h>

enum e_RenderingMode {
  e_Disabled,
  e_Onscreen,
  e_Offscreen
};

struct GameConfig {
  // Should we log notice logs.
  bool log_notice = false;
  // Should game render in high quality.
  bool high_quality = false;
  // Is rendering enabled.
  e_RenderingMode render_mode = e_Onscreen;
  // Directory with textures and other resources.
  std::string data_dir;
  // Font file used for rendering UI.
  std::string font_file;
  // Computer AI difficulty level, from 0.0 to 1.0.
  float game_difficulty;
  // How many physics animation steps are done per single environment step.
  int physics_steps_per_frame = 10;
};

struct ScenarioConfig {
  // Start ball position.
  Vector3 ball_position;
  // Initial configuration of home team.
  std::vector<FormationEntry> home_team;
  // Initial configuration of away team.
  std::vector<FormationEntry> away_team;
  // How many home players are controlled externally.
  int home_agents = 1;
  // How many away players are controlled externally.
  int away_agents = 0;
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
};

enum e_DebugMode {
  e_DebugMode_Off,
  e_DebugMode_Tactical,
  e_DebugMode_AI
};

class Match;

void SetGreenDebugPilon(const Vector3 &pos);
void SetBlueDebugPilon(const Vector3 &pos);
void SetYellowDebugPilon(const Vector3 &pos);
void SetRedDebugPilon(const Vector3 &pos);

boost::intrusive_ptr<Geometry> GetGreenDebugPilon();
boost::intrusive_ptr<Geometry> GetBlueDebugPilon();
boost::intrusive_ptr<Geometry> GetYellowDebugPilon();
boost::intrusive_ptr<Geometry> GetRedDebugPilon();

boost::intrusive_ptr<Geometry> GetSmallDebugCircle1();
boost::intrusive_ptr<Geometry> GetSmallDebugCircle2();
boost::intrusive_ptr<Geometry> GetLargeDebugCircle();

std::string GetConfigFilename();
boost::shared_ptr<Scene2D> GetScene2D();
boost::shared_ptr<Scene3D> GetScene3D();
GraphicsSystem *GetGraphicsSystem();
boost::shared_ptr<GameTask> GetGameTask();
boost::shared_ptr<MenuTask> GetMenuTask();
boost::shared_ptr<SynchronizationTask> GetSynchronizationTask();


bool IsReleaseVersion();
bool Verbose();
bool UpdateNonImportableDB();

Database *GetDB();
Properties *GetConfiguration();
ScenarioConfig& GetScenarioConfig();
GameConfig& GetGameConfig();
std::string GetActiveSaveDirectory();
void SetActiveSaveDirectory(const std::string &dir);
bool SuperDebug();
e_DebugMode GetDebugMode();
boost::intrusive_ptr<Image2D> GetDebugOverlay();
void GetDebugOverlayCoord(Match *match, const Vector3 &worldPos, int &x, int &y);

const std::vector<IHIDevice*> &GetControllers();

void run_game(Properties* input_config);
void randomize(unsigned int seed);
void quit_game();
int main(int argc, char** argv);
void set_rendering(bool);

#endif
