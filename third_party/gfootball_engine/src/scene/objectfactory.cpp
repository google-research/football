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

#include "objectfactory.hpp"

#include "../base/log.hpp"
#include "../base/utils.hpp"

#include "../scene/objects/camera.hpp"
#include "../scene/objects/image2d.hpp"
#include "../scene/objects/geometry.hpp"
#include "../scene/objects/skybox.hpp"
#include "../scene/objects/light.hpp"
#include "../scene/objects/joint.hpp"

namespace blunted {

  template<> ObjectFactory* Singleton<ObjectFactory>::singleton = 0;

  ObjectFactory::ObjectFactory() {
  }

  ObjectFactory::~ObjectFactory() {
  }

  void ObjectFactory::Exit() {
    std::map <e_ObjectType, boost::intrusive_ptr<Object> >::iterator prototype_iter = prototypes.begin();
    while (prototype_iter != prototypes.end()) {
      prototype_iter->second->Exit();
      prototype_iter++;
    }
    prototypes.clear();
  }

  boost::intrusive_ptr<Object> ObjectFactory::CreateObject(const std::string &name, e_ObjectType objectType, std::string postfix) {
    std::map <e_ObjectType, boost::intrusive_ptr<Object> >::iterator prototype_iter = prototypes.find(objectType);
    if (prototype_iter != prototypes.end()) {
      boost::intrusive_ptr<Object> bla = CopyObject((*prototype_iter).second, postfix);
      bla->SetName(name);
      return bla;
    } else {
      Log(e_FatalError, "ObjectFactory", "CreateObject", "Object type " + int_to_str(objectType) + " not found in prototype registry");
      return 0;
    }
  }

  boost::intrusive_ptr<Object> ObjectFactory::CopyObject(boost::intrusive_ptr<Object> source, std::string postfix) {
    boost::intrusive_ptr<Object> bla;

    switch (source->GetObjectType()) {
      case e_ObjectType_Camera:
        bla = new Camera(*static_cast<Camera*>(source.get()));
        break;
      case e_ObjectType_Image2D:
        bla = new Image2D(*static_cast<Image2D*>(source.get()));
        break;
      case e_ObjectType_Geometry:
        bla = new Geometry(*static_cast<Geometry*>(source.get()), postfix);
        break;
      case e_ObjectType_Skybox:
        bla = new Skybox(*static_cast<Skybox*>(source.get()));
        break;
      case e_ObjectType_Light:
        bla = new Light(*static_cast<Light*>(source.get()));
        break;
      case e_ObjectType_Joint:
        bla = new Joint(*static_cast<Joint*>(source.get()));
        break;
      default:
        Log(e_FatalError, "ObjectFactory", "CopyObject", "Object type " + int_to_str(source->GetObjectType()) + " not found");
        break;
    }
    assert(bla != boost::intrusive_ptr<Object>());
    return bla;
  }

  void ObjectFactory::RegisterPrototype(e_ObjectType objectType, boost::intrusive_ptr<Object> prototype) {
    prototypes.insert(std::pair<e_ObjectType, boost::intrusive_ptr<Object> >(objectType, prototype));
  }

}
