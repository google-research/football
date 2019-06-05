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

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_GEOMETRY
#define _HPP_GRAPHICSSYSTEM_OBJECT_GEOMETRY

#include "../../../base/math/vector3.hpp"
#include "../../../scene/objects/geometry.hpp"

#include "../graphics_object.hpp"

#include "../resources/vertexbuffer.hpp"

#include "../rendering/interface_renderer3d.hpp"

namespace blunted {

  class GraphicsGeometry_GeometryInterpreter;

  struct VertexBufferIndex;

  class GraphicsGeometry : public GraphicsObject {

    public:
      GraphicsGeometry(GraphicsScene *graphicsScene);
      virtual ~GraphicsGeometry();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      Vector3 GetPosition() const;
      virtual void SetRotation(const Quaternion &newRotation);
      Quaternion GetRotation() const;

      std::list<VertexBufferIndex> vertexBufferIndices;
      boost::intrusive_ptr < Resource<VertexBuffer> > vertexBuffer;

    protected:
      Vector3 position;
      Quaternion rotation;

  };

  class GraphicsGeometry_GeometryInterpreter : public IGeometryInterpreter {

    public:
      GraphicsGeometry_GeometryInterpreter(GraphicsGeometry *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad(boost::intrusive_ptr<Geometry> geometry);
      virtual void OnUpdateGeometry(boost::intrusive_ptr<Geometry> geometry, bool updateMaterials);
      virtual void OnUnload();
      inline virtual void OnMove(const Vector3 &position);
      inline virtual void OnRotate(const Quaternion &rotation);
      virtual void OnSynchronize();

      virtual void GetVertexBufferQueue(std::deque<VertexBufferQueueEntry> &queue);

      inline virtual void OnPoke();

    protected:
      GraphicsGeometry *caller;

      bool usesIndices = false;

  };

  class GraphicsGeometry_SkyboxInterpreter : public GraphicsGeometry_GeometryInterpreter {

    public:
      GraphicsGeometry_SkyboxInterpreter(GraphicsGeometry *caller);

      inline virtual void OnPoke();

    protected:

  };
}

#endif
