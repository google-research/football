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

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_CAMERA
#define _HPP_GRAPHICSSYSTEM_OBJECT_CAMERA

#include "../../../base/math/vector3.hpp"
#include "../../../scene/objects/camera.hpp"
#include "../../../scene/objects/geometry.hpp"
#include "../../../scene/objects/skybox.hpp"
#include "../graphics_object.hpp"

#include "../rendering/r3d_messages.hpp"

namespace blunted {

  class GraphicsCamera_CameraInterpreter;

  class GraphicsCamera : public GraphicsObject {

    public:
      GraphicsCamera(GraphicsScene *graphicsScene);
      virtual ~GraphicsCamera();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      Vector3 GetPosition() const;
      virtual void SetRotation(const Quaternion &newRotation);
      Quaternion GetRotation() const;

      void SetSize(float x_percent, float y_percent, float width_percent, float height_percent);

      float x_percent = 0.0f;
      float y_percent = 0.0f;
      float width_percent = 0.0f;
      float height_percent = 0.0f;
      int viewID = 0;
      ViewBuffer viewBuffer;
      float fov = 0.0f;
      float nearCap = 0.0f;
      float farCap = 0.0f;

    protected:
      Vector3 position;
      Quaternion rotation;

  };

  class GraphicsCamera_CameraInterpreter : public ICameraInterpreter {

    public:
      GraphicsCamera_CameraInterpreter(GraphicsCamera *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad(const Properties &properties);
      virtual void OnUnload();
      virtual void SetFOV(float fov);
      virtual void SetCapping(float nearCap, float farCap);
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation);
      virtual void EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes);
      virtual void OnPoke();

    protected:
      GraphicsCamera *caller = nullptr;

  };
}

#endif
