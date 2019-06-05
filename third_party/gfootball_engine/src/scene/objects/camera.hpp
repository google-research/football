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

#ifndef _HPP_OBJECT_CAMERA
#define _HPP_OBJECT_CAMERA

#include "../../defines.hpp"

#include "../../scene/object.hpp"

#include "../../types/interpreter.hpp"

#include "../../base/math/quaternion.hpp"
#include "../../base/math/vector3.hpp"

namespace blunted {

  class Light;
  class Geometry;
  class Skybox;

  class Camera : public Object {

    public:
      Camera(std::string name);
      virtual ~Camera();

      virtual void Init();
      virtual void Exit();

      virtual void SetFOV(float fov);
      virtual float GetFOV() const { return fov; }
      virtual void SetCapping(float nearCap, float farCap);
      virtual void GetCapping(float &nearCap, float &farCap) const { nearCap = this->nearCap; farCap = this->farCap; }


      virtual void EnqueueView(std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes);
      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:
      float fov = 0.0f;
      float nearCap = 0.0f;
      float farCap = 0.0f;

  };

  class ICameraInterpreter : public Interpreter {

    public:
      virtual void OnLoad(const Properties &properties) = 0;
      virtual void OnUnload() = 0;
      virtual void SetFOV(float fov) = 0;
      virtual void SetCapping(float nearCap, float farCap) = 0;
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation) = 0;

      virtual void EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes) = 0;
      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
