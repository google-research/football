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

#ifndef _HPP_OBJECT
#define _HPP_OBJECT

#include "../defines.hpp"

#include "../types/subject.hpp"
#include "../types/spatial.hpp"
#include "../base/properties.hpp"

namespace blunted {

  class ISystemObject;

  enum e_ObjectType {
    e_ObjectType_Camera = 1,
    e_ObjectType_Image2D = 2,
    e_ObjectType_Geometry = 3,
    e_ObjectType_Skybox = 4,
    e_ObjectType_Light = 5,
    e_ObjectType_Joint = 6,
    e_ObjectType_UserStart = 7
  };

  struct MustUpdateSpatialData {
    bool haveTo = false;
    e_SystemType excludeSystem;
  };

  // ATOMICITY: this class is responsible for doing about everything concurrently without crashing.
  // this implicitly accounts for atomicity in observers.
  class Object : public Subject<Interpreter>, public Spatial {

    public:
      Object(std::string name, e_ObjectType objectType);
      virtual ~Object();

      Object(const Object &src);

      virtual void Exit(); // ATOMIC

      virtual e_ObjectType GetObjectType();

      virtual bool IsEnabled() { return enabled; }
      virtual void Enable() { enabled = true; }
      virtual void Disable() { enabled = false; }

      virtual const Properties &GetProperties() const;
      virtual bool PropertyExists(const char *property) const;
      virtual const std::string &GetProperty(const char *property) const;

      virtual void SetProperties(Properties properties);
      virtual void SetProperty(const char *name, const char *value);

      virtual bool RequestPropertyExists(const char *property) const;
      virtual std::string GetRequestProperty(const char *property) const;
      virtual void AddRequestProperty(const char *property);
      virtual void SetRequestProperty(const char *property, const char *value);

      virtual void Synchronize();
      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

      MustUpdateSpatialData updateSpatialDataAfterPoke;

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_SystemType targetSystemType);

      virtual void SetPokePriority(int prio) { pokePriority = prio; }
      virtual int GetPokePriority() const { return pokePriority; }

      // set these before creating system objects

      Properties properties;



    protected:
      e_ObjectType objectType;

      mutable int pokePriority;

      // request these to be set by observing objects
      mutable Properties requestProperties;

      bool enabled = false;

  };

}

#endif
