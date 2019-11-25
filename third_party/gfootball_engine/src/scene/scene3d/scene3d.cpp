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

#include "scene3d.hpp"
#include "../../scene/objects/geometry.hpp"
#include "../../scene/objectfactory.hpp"

namespace blunted {

Scene3D::Scene3D() {
  DO_VALIDATION;
  boost::intrusive_ptr<Node> root(new Node("Scene3D root node"));
  hierarchyRoot = root;

  supportedObjectTypes.push_back(e_ObjectType_Geometry);
  supportedObjectTypes.push_back(e_ObjectType_Skybox);
  supportedObjectTypes.push_back(e_ObjectType_Camera);
  supportedObjectTypes.push_back(e_ObjectType_Light);
}

Scene3D::~Scene3D() { DO_VALIDATION; }

void Scene3D::Init() {
  DO_VALIDATION;

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    IScene3DInterpreter *scene3DInterpreter =
        static_cast<IScene3DInterpreter *>(observers[i].get());
    scene3DInterpreter->OnLoad();
  }
}

void Scene3D::Exit() {
  DO_VALIDATION;  // ATOMIC
  hierarchyRoot->Exit();
  hierarchyRoot.reset();

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    IScene3DInterpreter *scene3DInterpreter =
        static_cast<IScene3DInterpreter *>(observers[i].get());
    scene3DInterpreter->OnUnload();
  }

  Scene::Exit();
}

void Scene3D::AddNode(boost::intrusive_ptr<Node> node) {
  DO_VALIDATION;
  hierarchyRoot->AddNode(node);
}

void Scene3D::DeleteNode(boost::intrusive_ptr<Node> node) {
  DO_VALIDATION;
  hierarchyRoot->DeleteNode(node);
}

void Scene3D::PokeObjects(e_ObjectType targetObjectType,
                          e_SystemType targetSystemType) {
  DO_VALIDATION;
  if (!SupportedObjectType(targetObjectType)) {
    DO_VALIDATION;
    Log(e_Error, "Scene3D", "PokeObjects",
        "targetObjectType " + int_to_str(targetObjectType) +
            " is not supported by this scene");
    return;
  }

  hierarchyRoot->PokeObjects(targetObjectType, targetSystemType);
}
}
