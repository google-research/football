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

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_LIGHT
#define _HPP_GRAPHICSSYSTEM_OBJECT_LIGHT

#include "../../../base/math/vector3.hpp"
#include "../../../scene/objects/light.hpp"

#include "../graphics_object.hpp"

#include "../resources/texture.hpp"

#include "../rendering/interface_renderer3d.hpp"

namespace blunted {

  class GraphicsLight_LightInterpreter;

  class GraphicsLight : public GraphicsObject {

    public:
      GraphicsLight(GraphicsScene *graphicsScene);
      virtual ~GraphicsLight();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      virtual Vector3 GetPosition() const;

      virtual void SetColor(const Vector3 &newColor);
      virtual Vector3 GetColor() const;

      virtual void SetRadius(float radius);
      virtual float GetRadius() const;

      virtual void SetType(e_LightType lightType);
      virtual e_LightType GetType() const;

      virtual void SetShadow(bool shadow);
      virtual bool GetShadow() const;

      std::vector<ShadowMap> shadowMaps;

    protected:
      Vector3 position;
      Vector3 color;
      float radius = 0.0f;
      e_LightType lightType;
      bool shadow = false;

  };

  class GraphicsLight_LightInterpreter : public ILightInterpreter {

    public:
      GraphicsLight_LightInterpreter(GraphicsLight *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnUnload();
      virtual void SetValues(const Vector3 &color, float radius);
      virtual void SetType(e_LightType lightType);
      virtual void SetShadow(bool shadow);
      virtual bool GetShadow();
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation);
      virtual void EnqueueShadowMap(boost::intrusive_ptr<Camera> camera, std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry);
      virtual ShadowMap GetShadowMap(const std::string &camName);
      virtual void OnPoke();

    protected:
      GraphicsLight *caller;

  };
}

#endif
