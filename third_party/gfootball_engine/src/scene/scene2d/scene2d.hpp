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

#ifndef _HPP_SCENE2D
#define _HPP_SCENE2D

#include "../../defines.hpp"

#include "../scene.hpp"
#include "../object.hpp"
#include "../../base/properties.hpp"

namespace blunted {

  class Scene2D : public Scene {

    public:
      Scene2D(const std::string &name, const Properties &config);
      virtual ~Scene2D();

      virtual void Init();
      virtual void Exit(); // ATOMIC

      void AddObject(boost::intrusive_ptr<Object> object);
      void DeleteObject(boost::intrusive_ptr<Object> object);

      void PokeObjects(e_ObjectType targetObjectType,
                       e_SystemType targetSystemType);

      void GetContextSize(int &width, int &height, int &bpp);

     protected:
      vector_Objects objects;

      int width, height, bpp;

  };

  class IScene2DInterpreter : public ISceneInterpreter {

    public:
      virtual void OnLoad() = 0;
      virtual void OnUnload() = 0;

    protected:

  };

}

#endif
