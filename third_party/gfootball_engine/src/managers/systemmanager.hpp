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

#ifndef _HPP_MANAGERS_SYSTEM
#define _HPP_MANAGERS_SYSTEM

#include "../defines.hpp"

#include "../types/singleton.hpp"

namespace blunted {

  class ISystem;
  class IScene;
  class Object;

  typedef std::map <std::string, ISystem*> map_Systems;

  /// manages the registration of systems and the creation of their scenes and objects

  class SystemManager : public Singleton<SystemManager> {

    public:
      SystemManager();
      virtual ~SystemManager();

      virtual void Exit();
      bool RegisterSystem(const std::string systemName, ISystem *system);
      void CreateSystemScenes(boost::shared_ptr<IScene> scene);

     protected:
      map_Systems systems;

    private:

  };

}

#endif
