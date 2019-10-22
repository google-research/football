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

#include <string>

#include "ai/ai_keyboard.hpp"
#include "base/log.hpp"
#include "base/math/bluntmath.hpp"
#include "base/utils.hpp"
#include "file.h"
#include "main.hpp"
#include "scene/objectfactory.hpp"
#include "scene/scene2d/scene2d.hpp"
#include "scene/scene3d/scene3d.hpp"
#include "utils/objectloader.hpp"
#include "utils/orbitcamera.hpp"
#include "wrap_SDL_ttf.h"

using std::string;

#if defined(WIN32) && defined(__MINGW32__)
#undef main
#endif

using namespace blunted;

thread_local GameContext* context;

GameContext& GetContext() { return *context; }

void SetContext(GameContext* c) { context = c; }

boost::shared_ptr<Scene2D> GetScene2D() { return context->scene2D; }

boost::shared_ptr<Scene3D> GetScene3D() { return context->scene3D; }

GraphicsSystem* GetGraphicsSystem() { return context->graphicsSystem.get(); }

boost::shared_ptr<GameTask> GetGameTask() { return context->gameTask; }

boost::shared_ptr<MenuTask> GetMenuTask() { return context->menuTask; }

Properties* GetConfiguration() { return context->config; }

ScenarioConfig& GetScenarioConfig() { return context->scenario_config; }

GameConfig& GetGameConfig() { return context->game_config; }

const std::vector<IHIDevice*>& GetControllers() { return context->controllers; }

void randomize(unsigned int seed) {
  srand(seed);
  rand(); // mingw32? buggy compiler? first value seems bogus
  randomseed(seed); // for the boost random
}

void run_game(Properties* input_config) {
  context->config = input_config;
  auto& game_config = GetGameConfig();

  Initialize(*context->config);
  randomize(0);

  // initialize systems
  context->graphicsSystem.reset(new GraphicsSystem());
  context->graphicsSystem->Initialize(*context->config);

  // init scenes

  context->scene2D.reset(new Scene2D(*context->config));
  context->graphicsSystem->Create2DScene(context->scene2D);
  context->scene2D->Init();
  context->scene3D.reset(new Scene3D());
  context->graphicsSystem->Create3DScene(context->scene3D);
  context->scene3D->Init();

  for (int x = 0; x < 2 * MAX_PLAYERS; x++) {
    context->controllers.push_back(new AIControlledKeyboard());
  }
  // sequences

  context->gameTask = boost::shared_ptr<GameTask>(new GameTask());
  std::string fontfilename = context->config->Get(
      "font_filename", "media/fonts/alegreya/AlegreyaSansSC-ExtraBold.ttf");
  context->font = GetFile(fontfilename);
  context->defaultFont = TTF_OpenFontIndexRW(
      SDL_RWFromConstMem(context->font.data(), context->font.size()), 0, 32, 0);
  if (!context->defaultFont)
    Log(e_FatalError, "football", "main",
        "Could not load font " + fontfilename);
  context->defaultOutlineFont = TTF_OpenFontIndexRW(
      SDL_RWFromConstMem(context->font.data(), context->font.size()), 0, 32, 0);
  TTF_SetFontOutline(context->defaultOutlineFont, 2);
  context->menuTask = boost::shared_ptr<MenuTask>(
      new MenuTask(5.0f / 4.0f, 0, context->defaultFont,
                   context->defaultOutlineFont, context->config));
}
  // fire!

void quit_game() {
  context->gameTask.reset();
  context->menuTask.reset();

  context->scene2D.reset();
  context->scene3D.reset();

  for (unsigned int i = 0; i < context->controllers.size(); i++) {
    delete context->controllers[i];
  }
  context->controllers.clear();

  TTF_CloseFont(context->defaultFont);
  TTF_CloseFont(context->defaultOutlineFont);

  delete context->config;

  Exit();
}

