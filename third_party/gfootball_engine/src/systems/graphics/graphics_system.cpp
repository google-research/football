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

#include "graphics_system.hpp"

#include "graphics_scene.hpp"

#include "../../base/log.hpp"
#include "../../base/utils.hpp"

#include "../../managers/resourcemanagerpool.hpp"

#include "rendering/r3d_messages.hpp"

namespace blunted {

  GraphicsSystem::GraphicsSystem() {
  }

  GraphicsSystem::~GraphicsSystem() {
  }

  void GraphicsSystem::Initialize(const Properties &config) {

    // start thread for renderer
    std::string renderer = config.Get("graphics3d_renderer", "opengl");

    if (renderer == "opengl" || renderer == "egl" || renderer == "osmesa")
      renderer3DTask = new OpenGLRenderer3D();
    if (renderer == "mock") renderer3DTask = new MockRenderer3D();
    width = config.GetInt("context_x", 1280);
    height = config.GetInt("context_y", 720);
    bpp = config.GetInt("context_bpp", 32);
    bool fullscreen = config.GetBool("context_fullscreen", false);

    if (!static_cast<Renderer3D*>(renderer3DTask)->CreateContext(width, height, bpp, fullscreen)) {
      Log(e_FatalError, "GraphicsSystem", "Initialize", "Could not create context");
    }

    task = new GraphicsTask(this);
  }


  const screenshoot& GraphicsSystem::GetScreen() {
    return renderer3DTask->GetScreen();
  }

  void GraphicsSystem::Exit() {
    delete task;
    task = NULL;

    // shutdown renderer thread
    delete renderer3DTask;
    renderer3DTask = NULL;
  }

  e_SystemType GraphicsSystem::GetSystemType() const {
    return systemType;
  }

  ISystemScene *GraphicsSystem::CreateSystemScene(boost::shared_ptr<IScene> scene) {
    if (scene->GetSceneType() == e_SceneType_Scene2D) {
      GraphicsScene *graphicsScene = new GraphicsScene(this);
      scene->Attach(graphicsScene->GetInterpreter(e_SceneType_Scene2D));
      return graphicsScene;
    }
    if (scene->GetSceneType() == e_SceneType_Scene3D) {
      GraphicsScene *graphicsScene = new GraphicsScene(this);
      scene->Attach(graphicsScene->GetInterpreter(e_SceneType_Scene3D));
      return graphicsScene;
    }
    return NULL;
  }

  ISystemTask *GraphicsSystem::GetTask() {
    return task;
  }

  Renderer3D *GraphicsSystem::GetRenderer3D() {
    return renderer3DTask;
  }

  MessageQueue<Overlay2DQueueEntry> &GraphicsSystem::GetOverlay2DQueue() {
    return overlay2DQueue;
  }

}
