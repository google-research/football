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

#include "object.hpp"

#include "../systems/isystemobject.hpp"

namespace blunted {

  Object::Object(std::string name, e_ObjectType objectType) : Spatial(name), objectType(objectType) {
    MustUpdateSpatialData clear;
    clear.haveTo = false;
    clear.excludeSystem = e_SystemType_None;
    updateSpatialDataAfterPoke = clear;
    pokePriority = 0;
    enabled = true;
  }

  Object::~Object() {
    if (observers.size() != 0) {
      Log(e_FatalError, "Object", "~Object", "Observer(s) still present at destruction time (spatial named: " + GetName() + ")");
    }
  }

  Object::Object(const Object &src) : Subject<Interpreter>(), Spatial(src) {
    objectType = src.objectType;
    properties = src.properties;
    requestProperties.AddProperties(src.requestProperties);

    MustUpdateSpatialData clear;
    clear.haveTo = false;
    clear.excludeSystem = e_SystemType_None;
    updateSpatialDataAfterPoke = clear;
    pokePriority = src.GetPokePriority();
    enabled = src.enabled;

    assert(observers.size() == 0);
  }

  void Object::Exit() {
    //printf("detaching all observers from object named %s\n", GetName().c_str());
    DetachAll();
  }

  inline e_ObjectType Object::GetObjectType() {
    return objectType;
  }

  const Properties &Object::GetProperties() const {
    return properties;
  }

  bool Object::PropertyExists(const char *property) const {
    bool exists = properties.Exists(property);
    return exists;
  }

  const std::string &Object::GetProperty(const char *property) const {
    return properties.Get(property);
  }

  void Object::SetProperties(Properties newProperties) {
    properties.AddProperties(&newProperties);
  }

  void Object::SetProperty(const char *name, const char *value) {
    properties.Set(name, value);
  }

  bool Object::RequestPropertyExists(const char *property) const {
    bool exists = requestProperties.Exists(property);
    return exists;
  }

  std::string Object::GetRequestProperty(const char *property) const {
    std::string result = requestProperties.Get(property);
    return result;
  }

  void Object::AddRequestProperty(const char *property) {
    requestProperties.Set(property, "");
  }

  void Object::SetRequestProperty(const char *property, const char *value) {
    requestProperties.Set(property, value);
  }

  void Object::Synchronize() {
    boost::shared_ptr<Interpreter> result;
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      boost::intrusive_ptr<Interpreter> interpreter = static_pointer_cast<Interpreter>(observers[i]);
      interpreter->OnSynchronize();
    }
  }

  void Object::Poke(e_SystemType targetSystemType) {
  }

  inline void Object::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
  }

  boost::intrusive_ptr<Interpreter> Object::GetInterpreter(e_SystemType targetSystemType) {
    boost::intrusive_ptr<Interpreter> result;
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      boost::intrusive_ptr<Interpreter> interpreter = static_pointer_cast<Interpreter>(observers[i]);
      if (interpreter->GetSystemType() == targetSystemType) result = interpreter;
    }
    return result;
  }
}
