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
    caller->viewID = renderer3D->CreateView(caller->x_percent, caller->y_percent, caller->width_percent, caller->height_percent);
  }

  void GraphicsCamera_CameraInterpreter::OnUnload() {
    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    renderer3D->DeleteView(caller->viewID);
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
    Renderer3D *renderer = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();
    auto viewID = caller->viewID;
    auto &buffer = caller->viewBuffer;
    renderer->ClearBuffer(Vector3(0, 0, 0), false, true);

    View &view = renderer->GetView(viewID);

    Matrix4 projectionMatrix = renderer->CreatePerspectiveMatrix(view.width / (view.height * 1.0f), buffer.cameraNearCap, buffer.cameraFarCap);
    Matrix4 viewMatrix = buffer.cameraMatrix;


    // not sorting actually seems to be fastest atm. (the sorting being slower than the performance win by reducing state changes)
    //std::sort(buffer.visibleGeometry.begin(), buffer.visibleGeometry.end(), R3DM_SortVertexBufferQueueEntries);
    //printf("%i entries\n", buffer.visibleGeometry.size());
    //buffer.visibleGeometry.sort(SortVertexBufferQueueEntries);

    int width = 0;
    int height = 0;
    int bpp = 0;
    renderer->GetContextSize(width, height, bpp);
    // opengl window starts lower left, so invert y
    renderer->SetViewport(view.x, height - view.y - view.height, view.width, view.height);

    renderer->SetFOV(buffer.cameraFOV);

    float depthParamNear = 0;
    float depthParamFar = 0;
    volatile float division = buffer.cameraNearCap - buffer.cameraFarCap;
    if (fabs(division) > EPSILON) {
      division = 1 / division;
      depthParamFar = (buffer.cameraFarCap * buffer.cameraNearCap) * division;
    }
    if (fabs(division) > EPSILON) {
      depthParamNear = buffer.cameraFarCap / (buffer.cameraFarCap - buffer.cameraNearCap);
    }

    std::vector<e_TargetAttachment> targets;


    // render skybox

    if (buffer.skyboxes.size() > 0) {
      Matrix4 skyboxMatrix = viewMatrix;
      skyboxMatrix.SetTranslation(Vector3(0, 0, 0));
      //XX renderer->SetMatrixMode(e_MatrixMode_ModelView);
      //XX renderer->LoadMatrix(skyboxMatrix);

      renderer->UseShader("");

      //renderer->SetMatrix("viewMatrix", buffer.cameraMatrix);

      targets.push_back(e_TargetAttachment_Back);
      renderer->SetRenderTargets(targets);
      targets.clear();

      //renderer->ClearBuffer(Vector3(0, 0, 0), true, true);
      renderer->SetCullingMode(e_CullingMode_Back);
      renderer->SetBlendingMode(e_BlendingMode_Off);
      renderer->SetDepthFunction(e_DepthFunction_Less);
      renderer->SetDepthTesting(false);
      renderer->SetDepthMask(false);
      renderer->RenderVertexBuffer(buffer.skyboxes, e_RenderMode_Diffuse);
      renderer->SetDepthMask(true);
      renderer->SetDepthTesting(true);
    }


    // render vertexbuffers


    // pre z phase

    // disabled: geometry phase is already doing this, right?
    // enabled again: the geometry phase writes to the gbuffer, this writes to the accumbuffer that the lighting uses
    // disabled again: seems to work without z-phase, even with ambient phase on z equal. i guess the depth buffer is kept indepedently of the bound frame buffer.
    // disabled again: alpha pass (lame transparency) won't work with this

    bool zphase = false;
    if (zphase) {
      //XXrenderer->SetMatrixMode(e_MatrixMode_ModelView);

      renderer->SetMatrix("projectionMatrix", projectionMatrix);
      renderer->SetMatrix("viewMatrix", viewMatrix);

      renderer->UseShader("zphase");

      renderer->BindFrameBuffer(view.gBufferID);

      targets.push_back(e_TargetAttachment_None);
      renderer->SetRenderTargets(targets);
      targets.clear();

      renderer->SetCullingMode(e_CullingMode_Back);
      renderer->SetBlendingMode(e_BlendingMode_Off);
      renderer->SetDepthFunction(e_DepthFunction_Less);
      renderer->SetDepthTesting(true);
      renderer->SetDepthMask(true);
      renderer->ClearBuffer(Vector3(0, 0, 0), true, false);
      renderer->RenderVertexBuffer(buffer.visibleGeometry, e_RenderMode_GeometryOnly);
    }



    // geometry phase

    renderer->UseShader("simple");

    renderer->BindFrameBuffer(view.gBufferID);

    // framebuffer starts at lower left 0,0
    renderer->SetViewport(0, 0, view.width, view.height);

    renderer->SetMatrix("projectionMatrix", projectionMatrix);
    renderer->SetMatrix("viewMatrix", viewMatrix);

    // work-around ati bug: clearing z with color buffers attached could be slow
    // http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=105&Itemid=27
    //targets.push_back(e_TargetAttachment_Depth);

    targets.push_back(e_TargetAttachment_Color0);
    targets.push_back(e_TargetAttachment_Color1);
    targets.push_back(e_TargetAttachment_Color2);
    renderer->SetRenderTargets(targets);
    targets.clear();

    renderer->SetCullingMode(e_CullingMode_Back);
    renderer->SetBlendingMode(e_BlendingMode_Off);
    renderer->SetDepthTesting(true);

    if (zphase) {
      renderer->SetDepthFunction(e_DepthFunction_Equal); // changed from less
      renderer->SetDepthMask(false); // changed
      renderer->ClearBuffer(Vector3(0, 0, 0), false, true); // changed
    } else {
      renderer->SetDepthFunction(e_DepthFunction_Less);
      renderer->SetDepthMask(true);
      renderer->ClearBuffer(Vector3(0, 0, 0), true, true);
    }

    renderer->RenderVertexBuffer(buffer.visibleGeometry, e_RenderMode_Full);



    // lighting phase

    // output goes to accumulation buffer
    renderer->BindFrameBuffer(view.accumBufferID);
    targets.push_back(e_TargetAttachment_Color0);
    targets.push_back(e_TargetAttachment_Color1);
    renderer->SetRenderTargets(targets);
    targets.clear();

    // blend the remaining
    renderer->SetTextureUnit(1);
    renderer->BindTexture(view.gBuffer_NormalTexID);
    renderer->SetTextureUnit(2);
    renderer->BindTexture(view.gBuffer_DepthTexID);
    renderer->SetTextureUnit(3);
    renderer->BindTexture(view.gBuffer_AuxTexID);
    renderer->SetTextureUnit(0);
    renderer->BindTexture(view.gBuffer_AlbedoTexID);

    //renderer->ClearBuffer(Vector3(0, 0, 0), false, true);

    // ambient
    renderer->UseShader("ambient");

    renderer->SetUniformFloat("ambient", "contextWidth", (float)view.width);
    renderer->SetUniformFloat("ambient", "contextHeight", (float)view.height);
    renderer->SetUniformFloat("ambient", "contextX", (float)0);
    renderer->SetUniformFloat("ambient", "contextY", (float)0);
    renderer->SetUniformFloat2("ambient", "cameraClip", depthParamNear, depthParamFar);
    Matrix4 inverseProjectionViewMatrix = (projectionMatrix * viewMatrix).GetInverse();
    renderer->SetUniformMatrix4("ambient", "inverseProjectionViewMatrix", inverseProjectionViewMatrix);
    renderer->SetUniformMatrix4("ambient", "projectionMatrix", projectionMatrix);
    renderer->SetUniformMatrix4("ambient", "viewMatrix", viewMatrix);

    renderer->SetDepthTesting(false);
    renderer->SetDepthMask(false);

    renderer->RenderOverlay2D();

    // lights
    //renderer->SetMatrixMode(e_MatrixMode_ModelView);

    renderer->UseShader("lighting");

    renderer->SetBlendingMode(e_BlendingMode_On);
    renderer->SetBlendingFunction(e_BlendingFunction_One, e_BlendingFunction_One);

    renderer->SetUniformFloat("lighting", "contextWidth", (float)view.width);
    renderer->SetUniformFloat("lighting", "contextHeight", (float)view.height);
    renderer->SetUniformFloat("lighting", "contextX", (float)0);
    renderer->SetUniformFloat("lighting", "contextY", (float)0);
    renderer->SetUniformMatrix4("lighting", "inverseProjectionViewMatrix", inverseProjectionViewMatrix);
    renderer->SetUniformMatrix4("lighting", "projectionMatrix", projectionMatrix);
    renderer->SetUniformMatrix4("lighting", "viewMatrix", viewMatrix);

    //renderer->SetUniformFloat2("lighting", "cameraClip", depthParamNear, depthParamFar);

    renderer->SetDepthTesting(false);
    renderer->SetDepthMask(false);

    renderer->RenderLights(buffer.visibleLights, projectionMatrix, viewMatrix);

    renderer->SetBlendingMode(e_BlendingMode_Off);
    renderer->SetDepthMask(true);
    renderer->SetCullingMode(e_CullingMode_Off);

    renderer->SetTextureUnit(1);
    renderer->BindTexture(0);
    renderer->SetTextureUnit(2);
    renderer->BindTexture(0);
    renderer->SetTextureUnit(3);
    renderer->BindTexture(0);
    renderer->SetTextureUnit(0);
    renderer->BindTexture(0);



    // render accumulation buffer with some nice postprocessing effects

    renderer->BindFrameBuffer(0);

    targets.push_back(e_TargetAttachment_Back);
    renderer->SetRenderTargets(targets);
    targets.clear();

    renderer->UseShader("postprocess");

    //renderer->SetUniformFloat("postprocess", "brightness", (float)renderer->HDRGetOverallBrightness());

    renderer->SetUniformFloat("postprocess", "contextWidth", (float)view.width);
    renderer->SetUniformFloat("postprocess", "contextHeight", (float)view.height);
    renderer->SetUniformFloat("postprocess", "contextX", (float)view.x);
    renderer->SetUniformFloat("postprocess", "contextY", (float)(height - (view.y + view.height)));
    renderer->SetUniformFloat2("postprocess", "cameraClip", depthParamNear, depthParamFar);
    renderer->SetUniformFloat("postprocess", "fogScale", 0.8f - NormalizedClamp(buffer.cameraFOV, 20, 100) * 0.6f);

    renderer->SetViewport(view.x, height - (view.y + view.height), view.width, view.height);

    renderer->SetTextureUnit(2);
    renderer->BindTexture(view.gBuffer_DepthTexID);
    renderer->SetTextureUnit(1);
    renderer->BindTexture(view.accumBuffer_ModifierTexID);
    renderer->SetTextureUnit(0);
    renderer->BindTexture(view.accumBuffer_AccumTexID);

    renderer->SetDepthTesting(false);
    renderer->SetDepthMask(false);

    renderer->SetFramebufferGammaCorrection(true);
    renderer->RenderOverlay2D();
    renderer->SetFramebufferGammaCorrection(false);

    renderer->SetTextureUnit(2);
    renderer->BindTexture(0);
    renderer->SetTextureUnit(1);
    renderer->BindTexture(0);
    renderer->SetTextureUnit(0);
    renderer->BindTexture(0);


    // too slow
    //renderer->HDRCaptureOverallBrightness();

    // back to the context viewport
    renderer->UseShader("");

    targets.push_back(e_TargetAttachment_Back);
    renderer->SetRenderTargets(targets);
    targets.clear();

    renderer->SetViewport(0, 0, width, height);

    buffer.visibleGeometry.clear();
    buffer.visibleLights.clear();
    buffer.skyboxes.clear();
  }

}
