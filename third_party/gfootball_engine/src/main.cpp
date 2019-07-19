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

#ifdef WIN32
#include <windows.h>
#endif

#include "main.hpp"

#include "base/utils.hpp"
#include "base/math/bluntmath.hpp"

#include "scene/scene2d/scene2d.hpp"
#include "scene/scene3d/scene3d.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "utils/objectloader.hpp"
#include "scene/objectfactory.hpp"

#include "framework/scheduler.hpp"

#include "managers/systemmanager.hpp"
#include "managers/scenemanager.hpp"

#include "base/log.hpp"

#include "utils/orbitcamera.hpp"

#include "wrap_SDL_ttf.h"

#include "ai/ai_keyboard.hpp"

#include <string>

using std::string;

#if defined(WIN32) && defined(__MINGW32__)
#undef main
#endif

using namespace blunted;

GraphicsSystem *graphicsSystem;

boost::shared_ptr<Scene2D> scene2D;
boost::shared_ptr<Scene3D> scene3D;

boost::shared_ptr<TaskSequence> graphicsSequence;
boost::shared_ptr<TaskSequence> gameSequence;

boost::shared_ptr<GameTask> gameTask;
boost::shared_ptr<MenuTask> menuTask;
boost::shared_ptr<SynchronizationTask> synchronizationTask;

Database *db;

Properties *config;
ScenarioConfig scenario_config;
GameConfig game_config;
TTF_Font *defaultFont;
TTF_Font *defaultOutlineFont;

std::vector<IHIDevice*> controllers;

std::string activeSaveDirectory;

std::string configFile = "football.config";

boost::shared_ptr<Scene2D> GetScene2D() {
  return scene2D;
}

boost::shared_ptr<Scene3D> GetScene3D() {
  return scene3D;
}

GraphicsSystem *GetGraphicsSystem() {
  return graphicsSystem;
}

boost::shared_ptr<GameTask> GetGameTask() {
  return gameTask;
}

boost::shared_ptr<MenuTask> GetMenuTask() {
  return menuTask;
}

boost::shared_ptr<SynchronizationTask> GetSynchronizationTask() {
  return synchronizationTask;
}

Database *GetDB() {
  return db;
}

Properties *GetConfiguration() {
  return config;
}

ScenarioConfig& GetScenarioConfig() {
  return scenario_config;
}

GameConfig& GetGameConfig() {
  return game_config;
}

const std::vector<IHIDevice*> &GetControllers() {
  return controllers;
}

void randomize(unsigned int seed) {
  srand(seed);
  rand(); // mingw32? buggy compiler? first value seems bogus
  randomseed(seed); // for the boost random
  fastrandomseed(seed);
}

void run_game(Properties* input_config) {
  config = input_config;
  auto& game_config = GetGameConfig();

  Initialize(*config);
  randomize(0);

  int timeStep_ms = config->GetInt("physics_frametime_ms", 10);


  // database

  db = new Database();
  bool dbSuccess =
      db->Load(game_config.updatePath("databases/default/database.sqlite"));
  if (!dbSuccess) Log(e_FatalError, "main", "()", "Could not open database");


  // initialize systems

  SystemManager *systemManager = SystemManager::GetInstancePtr();

  graphicsSystem = new GraphicsSystem();
  bool returnvalue = systemManager->RegisterSystem("GraphicsSystem", graphicsSystem);
  if (!returnvalue) Log(e_FatalError, "football", "main", "Could not register GraphicsSystem");
  graphicsSystem->Initialize(*config);

  // init scenes

  scene2D = boost::shared_ptr<Scene2D>(new Scene2D("scene2D", *config));
  SceneManager::GetInstance().RegisterScene(scene2D);

  scene3D = boost::shared_ptr<Scene3D>(new Scene3D("scene3D"));
  SceneManager::GetInstance().RegisterScene(scene3D);

  for (int x = 0; x < 2 * MAX_PLAYERS; x++) {
    controllers.push_back(new AIControlledKeyboard());
  }
  // sequences

  gameTask = boost::shared_ptr<GameTask>(new GameTask());
  std::string fontfilename = config->Get("font_filename", "media/fonts/alegreya/AlegreyaSansSC-ExtraBold.ttf");
  defaultFont = TTF_OpenFont(fontfilename.c_str(), 32);
  if (!defaultFont) Log(e_FatalError, "football", "main", "Could not load font " + fontfilename);
  defaultOutlineFont = TTF_OpenFont(fontfilename.c_str(), 32);
  TTF_SetFontOutline(defaultOutlineFont, 2);
  menuTask = boost::shared_ptr<MenuTask>(new MenuTask(5.0f / 4.0f, 0, defaultFont, defaultOutlineFont, config));
  // ME
  //if (controllers.size() > 1) menuTask->SetEventJoyButtons(static_cast<HIDGamepad*>(controllers.at(1))->GetControllerMapping(e_ControllerButton_A), static_cast<HIDGamepad*>(controllers.at(1))->GetControllerMapping(e_ControllerButton_B));


  gameSequence = boost::shared_ptr<TaskSequence>(new TaskSequence("game", timeStep_ms, false));

  // note: the whole locking stuff is now happening from within some of the code, iirc, 't is all very ugly and unclear. sorry

  //gameSequence->AddLockEntry(graphicsGameMutex, e_LockAction_Lock);   // ---------- lock -----

  synchronizationTask = boost::shared_ptr<SynchronizationTask>(new SynchronizationTask());
  gameSequence->AddUserTaskEntry(synchronizationTask, e_TaskPhase_Get);

  gameSequence->AddUserTaskEntry(menuTask, e_TaskPhase_Get);
  gameSequence->AddUserTaskEntry(menuTask, e_TaskPhase_Process);
  gameSequence->AddUserTaskEntry(menuTask, e_TaskPhase_Put);

  //gameSequence->AddLockEntry(graphicsGameMutex, e_LockAction_Unlock); // ---------- unlock ---

  gameSequence->AddUserTaskEntry(gameTask, e_TaskPhase_Get);
  gameSequence->AddUserTaskEntry(gameTask, e_TaskPhase_Process);
  gameSequence->AddUserTaskEntry(gameTask, e_TaskPhase_Put);
  if (graphicsSystem && game_config.render_mode != e_Disabled) {
    gameSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Get);
    gameSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Process);
    gameSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Put);
  }

  gameSequence->AddUserTaskEntry(synchronizationTask, e_TaskPhase_Put);
  GetScheduler()->RegisterTaskSequence(gameSequence);
}
  // fire!

void quit_game() {
  gameTask.reset();
  menuTask.reset();

  gameSequence.reset();
  graphicsSequence.reset();

  scene2D.reset();
  scene3D.reset();

  for (unsigned int i = 0; i < controllers.size(); i++) {
    delete controllers[i];
  }
  controllers.clear();

  TTF_CloseFont(defaultFont);
  TTF_CloseFont(defaultOutlineFont);

  delete db;
  delete config;

  Exit();
}

void set_rendering(bool enabled) {
  GetGraphicsSystem()->GetTask()->SetEnabled(enabled);
}
