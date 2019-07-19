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

#ifndef _HPP_SCENE
#define _HPP_SCENE

#include "../defines.hpp"

#include "iscene.hpp"

namespace blunted {

  typedef std::vector < boost::intrusive_ptr<Object> > vector_Objects;

  class Scene : public IScene {

    public:
      Scene(std::string name, e_SceneType sceneType);
      virtual ~Scene();

      virtual void Init() = 0; // ATOMIC
      virtual void Exit(); // ATOMIC

      virtual void CreateSystemObjects(boost::intrusive_ptr<Object> object);

      virtual const std::string GetName() const;
      virtual e_SceneType GetSceneType() const;

      virtual bool SupportedObjectType(e_ObjectType objectType) const;

    protected:
      std::string name;

      e_SceneType sceneType;

      std::vector<e_ObjectType> supportedObjectTypes;

  };

}

#endif
