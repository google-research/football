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

#include "graphics_scene.hpp"
#include "graphics_system.hpp"
#include "graphics_object.hpp"

#include "objects/graphics_camera.hpp"
#include "objects/graphics_overlay2d.hpp"
#include "objects/graphics_geometry.hpp"
#include "objects/graphics_light.hpp"

#include "rendering/r3d_messages.hpp"

namespace blunted {

GraphicsScene::GraphicsScene(GraphicsSystem *graphicsSystem)
    : graphicsSystem(graphicsSystem) {
  DO_VALIDATION;
  // printf("CREATING GFX SCENE\n");
}

GraphicsScene::~GraphicsScene() {
  DO_VALIDATION;
  // printf("DELETING GFX SCENE\n");
}

GraphicsSystem *GraphicsScene::GetGraphicsSystem() {
  DO_VALIDATION;
  return graphicsSystem;
}

// void GraphicsScene::Exit() { DO_VALIDATION;
// printf("EXITING GFX SCENE\n");
//}

boost::intrusive_ptr<ISceneInterpreter> GraphicsScene::GetInterpreter(
    e_SceneType sceneType) {
  DO_VALIDATION;
  if (sceneType == e_SceneType_Scene2D) {
    DO_VALIDATION;
    boost::intrusive_ptr<GraphicsScene_Scene2DInterpreter> scene2DInterpreter(
        new GraphicsScene_Scene2DInterpreter(this));
    return scene2DInterpreter;
  }
  if (sceneType == e_SceneType_Scene3D) {
    DO_VALIDATION;
    boost::intrusive_ptr<GraphicsScene_Scene3DInterpreter> scene3DInterpreter(
        new GraphicsScene_Scene3DInterpreter(this));
    return scene3DInterpreter;
  }
  Log(e_FatalError, "GraphicsScene", "GetInterpreter",
      "No appropriate interpreter found for this SceneType");
  return boost::intrusive_ptr<GraphicsScene_Scene3DInterpreter>();
}

ISystemObject *GraphicsScene::CreateSystemObject(Object *object) {
  DO_VALIDATION;
  assert(object);


  if (object->GetObjectType() == e_ObjectType_Camera) {
    DO_VALIDATION;
    GraphicsCamera *graphicsObject = new GraphicsCamera(this);
    object->Attach(graphicsObject->GetInterpreter(e_ObjectType_Camera));
    return graphicsObject;
  }
  if (object->GetObjectType() == e_ObjectType_Image2D) {
    DO_VALIDATION;
    GraphicsOverlay2D *graphicsObject = new GraphicsOverlay2D(this);
    object->Attach(graphicsObject->GetInterpreter(e_ObjectType_Image2D));
    return graphicsObject;
  }
  if (object->GetObjectType() == e_ObjectType_Geometry) {
    DO_VALIDATION;
    GraphicsGeometry *graphicsObject = new GraphicsGeometry(this);
    object->Attach(graphicsObject->GetInterpreter(e_ObjectType_Geometry),
                   object);
    return graphicsObject;
  }
  if (object->GetObjectType() == e_ObjectType_Skybox) {
    DO_VALIDATION;
    GraphicsGeometry *graphicsObject = new GraphicsGeometry(this);
    object->Attach(graphicsObject->GetInterpreter(e_ObjectType_Skybox));
    return graphicsObject;
  }
  if (object->GetObjectType() == e_ObjectType_Light) {
    DO_VALIDATION;
    GraphicsLight *graphicsLight = new GraphicsLight(this);
    object->Attach(graphicsLight->GetInterpreter(e_ObjectType_Light));
    return graphicsLight;
  }

  return NULL;
}

  // Scene3D interpreter

GraphicsScene_Scene3DInterpreter::GraphicsScene_Scene3DInterpreter(
    GraphicsScene *caller)
    : caller(caller) {
  DO_VALIDATION;
}

void GraphicsScene_Scene3DInterpreter::OnLoad() { DO_VALIDATION; }

void GraphicsScene_Scene3DInterpreter::OnUnload() {
  DO_VALIDATION;
  delete caller;
  caller = 0;
}

ISystemObject *GraphicsScene_Scene3DInterpreter::CreateSystemObject(
    Object *object) {
  DO_VALIDATION;
  return caller->CreateSystemObject(object);
}

  // Scene2D interpreter

GraphicsScene_Scene2DInterpreter::GraphicsScene_Scene2DInterpreter(
    GraphicsScene *caller)
    : caller(caller) {
  DO_VALIDATION;
}

void GraphicsScene_Scene2DInterpreter::OnLoad() { DO_VALIDATION; }

void GraphicsScene_Scene2DInterpreter::OnUnload() {
  DO_VALIDATION;
  delete caller;
  caller = 0;
}

ISystemObject *GraphicsScene_Scene2DInterpreter::CreateSystemObject(
    Object *object) {
  DO_VALIDATION;
  return caller->CreateSystemObject(object);
}
}
