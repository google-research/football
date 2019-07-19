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

#include "scene2d.hpp"


#include "../../scene/objects/image2d.hpp"
#include "../../scene/objects/geometry.hpp"

#include "../../systems/isystemscene.hpp"

#include "../../managers/environmentmanager.hpp"
#include "../../managers/systemmanager.hpp"

#include "../../scene/objectfactory.hpp"

namespace blunted {

  Scene2D::Scene2D(const std::string &name, const Properties &config) : Scene(name, e_SceneType_Scene2D) {
    //printf("CREATING SCENE2D\n");

    supportedObjectTypes.push_back(e_ObjectType_Image2D);

    width = config.GetInt("context_x", 1280);
    height = config.GetInt("context_y", 720);
    bpp = config.GetInt("context_bpp", 32);
  }

  Scene2D::~Scene2D() {
    //printf("DELETING SCENE2D\n");
  }

  void Scene2D::Init() {
  }

  void Scene2D::Exit() { // ATOMIC
    for (int i = 0; i < (signed int)objects.size(); i++) {
      objects[i]->Exit(); //now in spatial intrusiveptr
      objects[i].reset();
    }
    objects.clear();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IScene2DInterpreter *scene2DInterpreter = static_cast<IScene2DInterpreter*>(observers[i].get());
      scene2DInterpreter->OnUnload();
    }

    Scene::Exit();
  }

  void Scene2D::AddObject(boost::intrusive_ptr<Object> object) {
    if (!SupportedObjectType(object->GetObjectType())) {
      Log(e_FatalError, "Scene2D", "AddObject", "Object type not supported");
    }
    objects.push_back(object);
  }

  void Scene2D::DeleteObject(boost::intrusive_ptr<Object> object) {
    std::vector <boost::intrusive_ptr<Object> >::iterator objIter = objects.begin();
    while (objIter != objects.end()) {
      if ((*objIter) == object) {
        (*objIter)->Exit();
        objIter = objects.erase(objIter);
      } else {
        objIter++;
      }
    }
  }

  bool SortObjects(const boost::intrusive_ptr<Object> &a, const boost::intrusive_ptr<Object> &b) {
    return a->GetPokePriority() < b->GetPokePriority();
  }

  void Scene2D::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    if (!SupportedObjectType(targetObjectType)) return;
    std::stable_sort(objects.begin(), objects.end(), SortObjects);
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects[i]->IsEnabled()) if (objects[i]->GetObjectType() == targetObjectType) objects[i]->Poke(targetSystemType);
    }
  }

  void Scene2D::GetContextSize(int &width, int &height, int &bpp) {
    width = this->width;
    height = this->height;
    bpp = this->bpp;
  }
}
