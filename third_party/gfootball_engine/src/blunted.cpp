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

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "blunted.hpp"

#include "base/log.hpp"
#include "base/properties.hpp"
#include "base/utils.hpp"
#include "loaders/aseloader.hpp"
#include "loaders/imageloader.hpp"
#include "main.hpp"
#include "managers/resourcemanager.hpp"
#include "scene/objectfactory.hpp"
#include "scene/objects/camera.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objects/image2d.hpp"
#include "scene/objects/light.hpp"
#include "scene/objects/skybox.hpp"
#include "scene/resources/geometrydata.hpp"
#include "scene/resources/surface.hpp"
#include "systems/isystem.hpp"
#include "wrap_SDL_ttf.h"

namespace blunted {

void Initialize(Properties &config) {
  DO_VALIDATION;
  // initialize resource managers
  GetContext().geometry_manager.RegisterLoader("ase", &GetContext().aseLoader);
  GetContext().surface_manager.RegisterLoader("jpg", &GetContext().imageLoader);
  GetContext().surface_manager.RegisterLoader("png", &GetContext().imageLoader);
  GetContext().surface_manager.RegisterLoader("bmp", &GetContext().imageLoader);
  TTF_Init();
}

void Exit() {
  DO_VALIDATION;
  GetContext().scene2D->Exit();
  GetContext().scene3D->Exit();
  GetContext().graphicsSystem.Exit();
  TTF_Quit();
  SDL_Quit();
}
}
