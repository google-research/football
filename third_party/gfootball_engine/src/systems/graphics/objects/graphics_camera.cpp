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

#include "graphics_camera.hpp"

#include "../../../systems/graphics/rendering/r3d_messages.hpp"

#include "../graphics_scene.hpp"
#include "../graphics_system.hpp"

#include "graphics_geometry.hpp"
#include "graphics_light.hpp"

namespace blunted {

  GraphicsCamera::GraphicsCamera(GraphicsScene *graphicsScene) : GraphicsObject(graphicsScene) {
    fov = 45;
  }

  GraphicsCamera::~GraphicsCamera() {
  }

  boost::intrusive_ptr<Interpreter> GraphicsCamera::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Camera) {
      boost::intrusive_ptr<GraphicsCamera_CameraInterpreter> CameraInterpreter(new GraphicsCamera_CameraInterpreter(this));
      return CameraInterpreter;
    }
    Log(e_FatalError, "GraphicsCamera", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<GraphicsCamera_CameraInterpreter>();
  }

  void GraphicsCamera::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 GraphicsCamera::GetPosition() const {
    return position;
  }

  void GraphicsCamera::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion GraphicsCamera::GetRotation() const {
    return rotation;
  }

  void GraphicsCamera::SetSize(float x_percent, float y_percent, float width_percent, float height_percent) {
    this->x_percent = x_percent;
    this->y_percent = y_percent;
    this->width_percent = width_percent;
    this->height_percent = height_percent;
  }


  GraphicsCamera_CameraInterpreter::GraphicsCamera_CameraInterpreter(GraphicsCamera *caller) : caller(caller) {
  }

  void GraphicsCamera_CameraInterpreter::OnLoad(const Properties &properties) {
    caller->SetSize(properties.GetReal("x_percent", 0), properties.GetReal("y_percent", 0), properties.GetReal("width_percent", 100), properties.GetReal("height_percent", 100));

    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    Renderer3DMessage_CreateView op(caller->x_percent, caller->y_percent, caller->width_percent, caller->height_percent);
    op.Handle(renderer3D);
    caller->viewID = op.viewID;
  }

  void GraphicsCamera_CameraInterpreter::OnUnload() {
    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    Renderer3DMessage_DeleteView(caller->viewID).Handle(renderer3D);
    delete caller;
    caller = 0;
  }

  void GraphicsCamera_CameraInterpreter::SetFOV(float fov) {
    caller->fov = fov;
  }

  void GraphicsCamera_CameraInterpreter::SetCapping(float nearCap, float farCap) {
    caller->nearCap = nearCap;
    caller->farCap = farCap;
  }

  void GraphicsCamera_CameraInterpreter::OnSpatialChange(const Vector3 &position, const Quaternion &rotation) {
    caller->SetPosition(position);
    caller->SetRotation(rotation);
  }

  void GraphicsCamera_CameraInterpreter::EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes) {

    ViewBuffer *buffer = &caller->viewBuffer;


    // geometry

    std::deque < boost::intrusive_ptr<Geometry> >::iterator visibleGeometryIter = visibleGeometry.begin();
    while (visibleGeometryIter != visibleGeometry.end()) {
      boost::intrusive_ptr<GraphicsGeometry_GeometryInterpreter> interpreter = static_pointer_cast<GraphicsGeometry_GeometryInterpreter>((*visibleGeometryIter)->GetInterpreter(e_SystemType_Graphics));

      // add buffers to visible geometry queue
      interpreter->GetVertexBufferQueue(buffer->visibleGeometry);
      std::deque<VertexBufferQueueEntry>::iterator visibleGeometryBufferIter = buffer->visibleGeometry.end();
      visibleGeometryBufferIter--;
      (*visibleGeometryBufferIter).aabb = (*visibleGeometryIter)->GetAABB();

      visibleGeometryIter++;
    }


    // lights

    std::deque < boost::intrusive_ptr<Light> >::iterator visibleLightIter = visibleLights.begin();
    while (visibleLightIter != visibleLights.end()) {
      LightQueueEntry entry;

      boost::intrusive_ptr<GraphicsLight_LightInterpreter> interpreter = static_pointer_cast<GraphicsLight_LightInterpreter>((*visibleLightIter)->GetInterpreter(e_SystemType_Graphics));

      if (interpreter->GetShadow()) {
        ShadowMap shadowMap = interpreter->GetShadowMap(camName);
        if (shadowMap.cameraName != "") {
          entry.shadowMapTexture = shadowMap.texture;
          entry.lightProjectionMatrix = shadowMap.lightProjectionMatrix;
          entry.lightViewMatrix = shadowMap.lightViewMatrix;
          entry.hasShadow = true;
        } else {
          entry.hasShadow = false;
        }
      } else {
        entry.hasShadow = false;
      }


      entry.position = (*visibleLightIter)->GetDerivedPosition();
      entry.type = (*visibleLightIter)->GetType() == e_LightType_Directional ? 0 : 1;
      entry.shadow = (*visibleLightIter)->GetShadow();
      entry.color = (*visibleLightIter)->GetColor();
      entry.radius = (*visibleLightIter)->GetRadius();
      entry.aabb = (*visibleLightIter)->GetAABB();
      buffer->visibleLights.push_back(entry);

      visibleLightIter++;
    }


    // skyboxes

    std::deque < boost::intrusive_ptr<Skybox> >::iterator skyboxIter = skyboxes.begin();
    while (skyboxIter != skyboxes.end()) {
      boost::intrusive_ptr<GraphicsGeometry_SkyboxInterpreter> interpreter = static_pointer_cast<GraphicsGeometry_SkyboxInterpreter>((*skyboxIter)->GetInterpreter(e_SystemType_Graphics));
      // add buffers to skybox queue
      interpreter->GetVertexBufferQueue(buffer->skyboxes);
      skyboxIter++;
    }


    // camera matrix

    buffer->cameraMatrix.ConstructInverse(caller->GetPosition(), Vector3(1, 1, 1), caller->GetRotation());
    //buffer->projectionMatrix.ConstructProjection(caller->fov, aspect, caller->nearCap, caller->farCap);
    buffer->cameraFOV = caller->fov;
    buffer->cameraNearCap = caller->nearCap;
    buffer->cameraFarCap = caller->farCap;
  }

  void GraphicsCamera_CameraInterpreter::OnPoke() {

    Renderer3DMessage_RenderView op(caller->viewID, caller->viewBuffer);
    op.Handle(caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D());
  }

}
