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

#include "types/thread.hpp"
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

boost::intrusive_ptr<Geometry> greenPilon;
boost::intrusive_ptr<Geometry> bluePilon;
boost::intrusive_ptr<Geometry> yellowPilon;
boost::intrusive_ptr<Geometry> redPilon;

boost::intrusive_ptr<Geometry> smallDebugCircle1;
boost::intrusive_ptr<Geometry> smallDebugCircle2;
boost::intrusive_ptr<Geometry> largeDebugCircle;

void SetGreenDebugPilon(const Vector3 &pos) { greenPilon->SetPosition(pos, false); }
void SetBlueDebugPilon(const Vector3 &pos) { bluePilon->SetPosition(pos, false); }
void SetYellowDebugPilon(const Vector3 &pos) { yellowPilon->SetPosition(pos, false); }
void SetRedDebugPilon(const Vector3 &pos) { redPilon->SetPosition(pos, false); }

boost::intrusive_ptr<Geometry> GetGreenDebugPilon() { return greenPilon; }
boost::intrusive_ptr<Geometry> GetBlueDebugPilon() { return bluePilon; }
boost::intrusive_ptr<Geometry> GetYellowDebugPilon() { return yellowPilon; }
boost::intrusive_ptr<Geometry> GetRedDebugPilon() { return redPilon; }

boost::intrusive_ptr<Geometry> GetSmallDebugCircle1() { return smallDebugCircle1; }
boost::intrusive_ptr<Geometry> GetSmallDebugCircle2() { return smallDebugCircle2; }
boost::intrusive_ptr<Geometry> GetLargeDebugCircle() { return largeDebugCircle; }

Database *db;

Properties *config;
ScenarioConfig scenario_config;
GameConfig game_config;
TTF_Font *defaultFont;
TTF_Font *defaultOutlineFont;

boost::intrusive_ptr<Image2D> debugImage;
boost::intrusive_ptr<Image2D> debugOverlay;

std::vector<IHIDevice*> controllers;

bool superDebug = false;
e_DebugMode debugMode = e_DebugMode_Off;

std::string activeSaveDirectory;

std::string configFile = "football.config";
std::string GetConfigFilename() {
  return configFile;
}

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

bool IsReleaseVersion() {
  if (GetConfiguration()->GetBool("debug", false)) return false; else return true;
}

bool Verbose() {
  return !IsReleaseVersion();
}

bool UpdateNonImportableDB() {
  if (IsReleaseVersion()) return false;
  else return true;
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

std::string GetActiveSaveDirectory() {
  return activeSaveDirectory;
}

void SetActiveSaveDirectory(const std::string &dir) {
  activeSaveDirectory = dir;
}

bool SuperDebug() {
  return superDebug;
}

e_DebugMode GetDebugMode() {
  return debugMode;
}

boost::intrusive_ptr<Image2D> GetDebugOverlay() {
  return debugOverlay;
}

void GetDebugOverlayCoord(Match *match, const Vector3 &worldPos, int &x, int &y) {
  Vector3 proj = GetProjectedCoord(worldPos, match->GetCamera());
  int dud1, dud2;
  GetMenuTask()->GetWindowManager()->GetCoordinates(proj.coords[0], proj.coords[1], 1, 1, x, y, dud1, dud2);

  int contextW, contextH, bpp;
  GetScene2D()->GetContextSize(contextW, contextH, bpp);
  x = clamp(x, 0, contextW - 1);
  y = clamp(y, 0, contextH - 1);
}

void InitDebugImage() {
  SDL_Surface *sdlSurface = CreateSDLSurface(200, 150);

  boost::intrusive_ptr < Resource <Surface> > resource = ResourceManagerPool::getSurfaceManager()->Fetch("debugimage", false, true);
  Surface *surface = resource->GetResource();

  surface->SetData(sdlSurface);

  debugImage = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("debugimage", e_ObjectType_Image2D));
  scene2D->CreateSystemObjects(debugImage);
  debugImage->SetImage(resource);

  int contextW, contextH, bpp; // context
  scene2D->GetContextSize(contextW, contextH, bpp);
  debugImage->SetPosition(contextW - 210, contextH - 160);

  scene2D->AddObject(debugImage);

  debugImage->DrawRectangle(0, 0, 200, 150, Vector3(40, 20, 20), 100);
  debugImage->OnChange();
}

void InitDebugOverlay() {
  int contextW, contextH, bpp; // context
  scene2D->GetContextSize(contextW, contextH, bpp);

  SDL_Surface *sdlSurface = CreateSDLSurface(contextW, contextH);

  boost::intrusive_ptr < Resource <Surface> > resource = ResourceManagerPool::getSurfaceManager()->Fetch("debugoverlay", false, true);
  Surface *surface = resource->GetResource();

  surface->SetData(sdlSurface);

  debugOverlay = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("debugoverlay", e_ObjectType_Image2D));
  scene2D->CreateSystemObjects(debugOverlay);
  debugOverlay->SetImage(resource);

  debugOverlay->SetPosition(0, 0);

  scene2D->AddObject(debugOverlay);

  debugOverlay->DrawRectangle(0, 0, contextW, contextH, Vector3(0, 0, 0), 0);
  debugOverlay->OnChange();
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
  randomize(false);

  int timeStep_ms = config->GetInt("physics_frametime_ms", 10);


  // database

  db = new Database();
  bool dbSuccess = db->Load("databases/default/database.sqlite");
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

  if (SuperDebug()) InitDebugImage();
  if (GetDebugMode() == e_DebugMode_AI) InitDebugOverlay();

  // debug pilons

  boost::intrusive_ptr < Resource<GeometryData> > geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/green.ase", true);
  greenPilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("greenPilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(greenPilon);
  greenPilon->SetGeometryData(geometry);
  greenPilon->SetLocalMode(e_LocalMode_Absolute);
  greenPilon->SetPosition(Vector3(0, 0, -10));
  //greenPilon->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/blue.ase", true);
  bluePilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("bluePilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(bluePilon);
  bluePilon->SetGeometryData(geometry);
  bluePilon->SetLocalMode(e_LocalMode_Absolute);
  bluePilon->SetPosition(Vector3(0, 0, -10));
  //bluePilon->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/yellow.ase", true);
  yellowPilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("yellowPilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(yellowPilon);
  yellowPilon->SetGeometryData(geometry);
  yellowPilon->SetLocalMode(e_LocalMode_Absolute);
  yellowPilon->SetPosition(Vector3(0, 0, -10));
  //yellowPilon->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/red.ase", true);
  redPilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("redPilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(redPilon);
  redPilon->SetGeometryData(geometry);
  redPilon->SetLocalMode(e_LocalMode_Absolute);
  redPilon->SetPosition(Vector3(0, 0, -10));
  //redPilon->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/smalldebugcircle.ase", true);
  smallDebugCircle1 = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("smallDebugCircle1", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(smallDebugCircle1);
  smallDebugCircle1->SetGeometryData(geometry);
  smallDebugCircle1->SetLocalMode(e_LocalMode_Absolute);
  smallDebugCircle1->SetPosition(Vector3(0, 0, -10));
//  smallDebugCircle1->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/smalldebugcircle.ase", true);
  smallDebugCircle2 = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("smallDebugCircle2", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(smallDebugCircle2);
  smallDebugCircle2->SetGeometryData(geometry);
  smallDebugCircle2->SetLocalMode(e_LocalMode_Absolute);
  smallDebugCircle2->SetPosition(Vector3(0, 0, -10));
//  smallDebugCircle2->Disable();

  geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/helpers/largedebugcircle.ase", true);
  largeDebugCircle = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("largeDebugCircle", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(largeDebugCircle);
  largeDebugCircle->SetGeometryData(geometry);
  largeDebugCircle->SetLocalMode(e_LocalMode_Absolute);
  largeDebugCircle->SetPosition(Vector3(0, 0, -10));
//  largeDebugCircle->Disable();

  geometry.reset();


  // controllers

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
  if (SuperDebug()) scene2D->DeleteObject(debugImage);
  if (GetDebugMode() == e_DebugMode_AI) scene2D->DeleteObject(debugOverlay);

  gameTask.reset();
  menuTask.reset();

  gameSequence.reset();
  graphicsSequence.reset();

  greenPilon->Exit();
  greenPilon.reset();
  bluePilon->Exit();
  bluePilon.reset();
  yellowPilon->Exit();
  yellowPilon.reset();
  redPilon->Exit();
  redPilon.reset();
  smallDebugCircle1->Exit();
  smallDebugCircle1.reset();
  smallDebugCircle2->Exit();
  smallDebugCircle2.reset();
  largeDebugCircle->Exit();
  largeDebugCircle.reset();

  scene2D.reset();
  scene3D.reset();

  for (unsigned int i = 0; i < controllers.size(); i++) {
    delete controllers.at(i);
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
