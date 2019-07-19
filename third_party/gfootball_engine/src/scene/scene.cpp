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

#include "scene.hpp"

namespace blunted {

  Scene::Scene(std::string name, e_SceneType sceneType) : name(name), sceneType(sceneType) {
  }

  Scene::~Scene() {
  }

  void Scene::Exit() {
    DetachAll();
  }

  void Scene::CreateSystemObjects(boost::intrusive_ptr<Object> object) {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISceneInterpreter *sceneInterpreter = static_cast<ISceneInterpreter*>(observers[i].get());
      sceneInterpreter->CreateSystemObject(object);
    }
  }

  const std::string Scene::GetName() const {
    return name;
  }

  e_SceneType Scene::GetSceneType() const {
    return sceneType;
  }

  bool Scene::SupportedObjectType(e_ObjectType objectType) const {
    for (int i = 0; i < (signed int)supportedObjectTypes.size(); i++) {
      if (objectType == supportedObjectTypes[i]) return true;
    }
    return false;
  }

}
