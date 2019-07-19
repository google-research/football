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

#include "systemmanager.hpp"
#include "scenemanager.hpp"

#include "../systems/isystem.hpp"
#include "../base/log.hpp"
#include "../scene/object.hpp"

namespace blunted {

  template<> SystemManager* Singleton<SystemManager>::singleton = 0;

  SystemManager::SystemManager() {
  }

  SystemManager::~SystemManager() {
  }

  void SystemManager::Exit() {
    map_Systems::iterator s_iter = systems.begin();
    while (s_iter != systems.end()) {
      ISystem *system = s_iter->second;
      system->Exit();
      delete system;
      s_iter++;
    }
    systems.clear();
  }

  bool SystemManager::RegisterSystem(const std::string systemName, ISystem *system) {
    std::pair <map_Systems::iterator, bool> result = systems.insert(map_Systems::value_type(systemName, system));
    if (result.second == false) {
      // property already exists, replace its value?
      return false;
    }
    return true;
  }

  void SystemManager::CreateSystemScenes(boost::shared_ptr<IScene> scene) {
    map_Systems::iterator s_iter = systems.begin();
    while (s_iter != systems.end()) {
      ISystem *system = s_iter->second;
      system->CreateSystemScene(scene);
      s_iter++;
    }
    scene->Init();
  }
}
