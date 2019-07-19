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

#ifndef _HPP_MANAGERS_SCENE
#define _HPP_MANAGERS_SCENE

#include "../defines.hpp"

#include "../types/singleton.hpp"
#include "../scene/scene.hpp"

namespace blunted {

  typedef std::vector < boost::shared_ptr<IScene> > vector_Scenes;

  class SceneManager : public Singleton<SceneManager> {

    friend class Singleton<SceneManager>;

    public:
      SceneManager();
      virtual ~SceneManager();
      
      virtual void Exit();

      void RegisterScene(boost::shared_ptr<IScene> scene);
      boost::shared_ptr<IScene> GetScene(const std::string &name,
                                         bool &success);  // ATOMIC

     protected:
      vector_Scenes scenes;

    private:

  };

}

#endif
