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

#include "../../systems/isystemscene.hpp"

#include "../../managers/environmentmanager.hpp"
#include "../../managers/systemmanager.hpp"

#include "../../scene/objectfactory.hpp"

namespace blunted {

  Scene3D::Scene3D(std::string name) : Scene(name, e_SceneType_Scene3D) {
    //printf("CREATING SCENE3D\n");
    boost::intrusive_ptr<Node> root(new Node("Scene3D root node"));
    hierarchyRoot = root;

    supportedObjectTypes.push_back(e_ObjectType_Geometry);
    supportedObjectTypes.push_back(e_ObjectType_Skybox);
    supportedObjectTypes.push_back(e_ObjectType_Camera);
    supportedObjectTypes.push_back(e_ObjectType_Light);
    supportedObjectTypes.push_back(e_ObjectType_Joint);
  }

  Scene3D::~Scene3D() {
    //printf("DELETING SCENE3D\n");
  }

  void Scene3D::Init() {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IScene3DInterpreter *scene3DInterpreter = static_cast<IScene3DInterpreter*>(observers[i].get());
      scene3DInterpreter->OnLoad();
    }

  }

  void Scene3D::Exit() { // ATOMIC
    hierarchyRoot->Exit();
    hierarchyRoot.reset();


    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IScene3DInterpreter *scene3DInterpreter = static_cast<IScene3DInterpreter*>(observers[i].get());
      scene3DInterpreter->OnUnload();
    }

    Scene::Exit();

  }


  void Scene3D::AddNode(boost::intrusive_ptr<Node> node) {
    hierarchyRoot->AddNode(node);
  }

  void Scene3D::DeleteNode(boost::intrusive_ptr<Node> node) {
    hierarchyRoot->DeleteNode(node);
  }

  void Scene3D::AddObject(boost::intrusive_ptr<Object> object) {
    if (!SupportedObjectType(object->GetObjectType())) {
      Log(e_FatalError, "Scene3D", "AddObject", "Object type not supported");
    }
    hierarchyRoot->AddObject(object);
  }

  void Scene3D::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    if (!SupportedObjectType(targetObjectType)) {
      Log(e_Error, "Scene3D", "PokeObjects", "targetObjectType " + int_to_str(targetObjectType) + " is not supported by this scene");
      return;
    }

    hierarchyRoot->PokeObjects(targetObjectType, targetSystemType);
  }

}
