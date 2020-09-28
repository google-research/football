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

#include "../../scene/objectfactory.hpp"

namespace blunted {

Scene2D::Scene2D(int width, int height) : width(width) , height(height), bpp(32) {
  DO_VALIDATION;
  supportedObjectTypes.push_back(e_ObjectType_Image2D);
}

Scene2D::~Scene2D() { DO_VALIDATION; }

void Scene2D::Init() { DO_VALIDATION; }

void Scene2D::Exit() {
  DO_VALIDATION;  // ATOMIC
  for (int i = 0; i < (signed int)objects.size(); i++) {
    DO_VALIDATION;
    objects[i]->Exit();  // now in spatial intrusiveptr
    objects[i].reset();
  }
  objects.clear();

  int observersSize = observers.size();
  for (int i = 0; i < observersSize; i++) {
    DO_VALIDATION;
    IScene2DInterpreter *scene2DInterpreter =
        static_cast<IScene2DInterpreter *>(observers[i].get());
    scene2DInterpreter->OnUnload();
  }

  Scene::Exit();
}

void Scene2D::AddObject(boost::intrusive_ptr<Object> object) {
  DO_VALIDATION;
  if (!SupportedObjectType(object->GetObjectType())) {
    DO_VALIDATION;
    Log(e_FatalError, "Scene2D", "AddObject", "Object type not supported");
  }
  objects.push_back(object);
}

void Scene2D::DeleteObject(boost::intrusive_ptr<Object> object) {
  DO_VALIDATION;
  std::vector<boost::intrusive_ptr<Object> >::iterator objIter =
      objects.begin();
  while (objIter != objects.end()) {
    DO_VALIDATION;
    if ((*objIter) == object) {
      DO_VALIDATION;
      (*objIter)->Exit();
      objIter = objects.erase(objIter);
    } else {
      objIter++;
    }
  }
}

bool SortObjects(const boost::intrusive_ptr<Object> &a,
                 const boost::intrusive_ptr<Object> &b) {
  DO_VALIDATION;
  return a->GetPokePriority() < b->GetPokePriority();
}

void Scene2D::PokeObjects(e_ObjectType targetObjectType,
                          e_SystemType targetSystemType) {
  DO_VALIDATION;
  if (!SupportedObjectType(targetObjectType)) return;
  std::stable_sort(objects.begin(), objects.end(), SortObjects);
  int objectsSize = objects.size();
  for (int i = 0; i < objectsSize; i++) {
    DO_VALIDATION;
    if (objects[i]->IsEnabled())
      if (objects[i]->GetObjectType() == targetObjectType)
        objects[i]->Poke(targetSystemType);
  }
}

void Scene2D::GetContextSize(int &width, int &height, int &bpp) {
  DO_VALIDATION;
  width = this->width;
  height = this->height;
  bpp = this->bpp;
}
}
