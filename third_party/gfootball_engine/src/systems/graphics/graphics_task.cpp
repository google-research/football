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

#include "graphics_task.hpp"

#include "../../base/log.hpp"
#include "../../base/utils.hpp"
#include "../../main.hpp"
#include "../../scene/objects/geometry.hpp"
#include "../../scene/objects/light.hpp"
#include "../../scene/objects/skybox.hpp"
#include "../../scene/scene3d/scene3d.hpp"
#include "graphics_system.hpp"

namespace blunted {

GraphicsTask::GraphicsTask(GraphicsSystem *system) : graphicsSystem(system) {
  DO_VALIDATION;
}

GraphicsTask::~GraphicsTask() {
  DO_VALIDATION;
  graphicsSystem = NULL;
}

void GraphicsTask::Render(bool swap_buffer) {
  DO_VALIDATION;
  Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();

  // poke all image2D objects
  if (GetContext().scene2D) {
    DO_VALIDATION;
    GetContext().scene2D->PokeObjects(e_ObjectType_Image2D,
                                      e_SystemType_Graphics);
  }
  std::list<boost::intrusive_ptr<Camera> > cameras;
  GetContext().scene3D->GetObjects<Camera>(e_ObjectType_Camera, cameras);
  std::list<boost::intrusive_ptr<Camera> >::iterator cameraIter =
      cameras.begin();
  // enqueue all camera views
  while (cameraIter != cameras.end()) {
    DO_VALIDATION;
    if ((*cameraIter)->IsEnabled()) {
      DO_VALIDATION;
      Execute(*cameraIter);
    }
    cameraIter++;
  }
  // poke lights
  GetContext().scene3D->PokeObjects(e_ObjectType_Light, e_SystemType_Graphics);
  // poke camera
  GetContext().scene3D->PokeObjects(e_ObjectType_Camera, e_SystemType_Graphics);
  // render the Overlay2D queue
  auto &overlay2DQueue = graphicsSystem->GetOverlay2DQueue();
  bool isMessage = true;
  std::vector<Overlay2DQueueEntry> queue;
  while (isMessage) {
    DO_VALIDATION;
    Overlay2DQueueEntry queueEntry = overlay2DQueue.GetMessage(isMessage);
    if (isMessage) {
      DO_VALIDATION;
      queue.push_back(queueEntry);
    }
  }
  renderer3D->RenderOverlay2D(queue);
  if (swap_buffer) {
    renderer3D->SwapBuffers();
  }
}

bool GraphicsTask::Execute(boost::intrusive_ptr<Camera> camera) {
  DO_VALIDATION;
  // test
  // camera->SetPosition(Vector3(sin((float)GetContext().environment_manager.GetTime_ms()
  // * 0.001f) * 60, 0, 0), true);
  Vector3 cameraPos = camera->GetDerivedPosition();
  Quaternion cameraRot = camera->GetDerivedRotation();
  float nearCap, farCap;
  camera->GetCapping(nearCap, farCap);

  float fov = camera->GetFOV() * 2.0f;
  float wideScreenMultiplier = 2.5f;
  vector_Planes bounding;
  Plane plane(cameraPos + cameraRot * Vector3(0, 0, -nearCap),
              cameraRot * Vector3(0, 0, -1).GetNormalized());
  bounding.push_back(plane);
  plane.Set(
      cameraPos,
      cameraRot * Vector3(0, (3.6f * wideScreenMultiplier) / (fov / 24.0f), -1)
                      .GetNormalized());
  bounding.push_back(plane);
  plane.Set(
      cameraPos,
      cameraRot * Vector3(0, (-3.6f * wideScreenMultiplier) / (fov / 24.0f), -1)
                      .GetNormalized());
  bounding.push_back(plane);
  plane.Set(cameraPos,
            cameraRot * Vector3(2.4f / (fov / 24.0f), 0, -1).GetNormalized());
  bounding.push_back(plane);
  plane.Set(cameraPos,
            cameraRot * Vector3(-2.4f / (fov / 24.0f), 0, -1).GetNormalized());
  bounding.push_back(plane);
  plane.Set(cameraPos + cameraRot * Vector3(0, 0, -farCap),
            cameraRot * Vector3(0, 0, 1).GetNormalized());
  bounding.push_back(plane);

  // altogether function (may be slow in scenes with lots of objects)

  std::deque<boost::intrusive_ptr<Object> > visibleObjects;
  GetContext().scene3D->GetObjects(visibleObjects, bounding);

  std::deque<boost::intrusive_ptr<Geometry> > visibleGeometry;
  std::deque<boost::intrusive_ptr<Light> > visibleLights;
  std::deque<boost::intrusive_ptr<Skybox> > skyboxes;

  std::deque<boost::intrusive_ptr<Object> >::iterator objectIter =
      visibleObjects.begin();
  while (objectIter != visibleObjects.end()) {
    DO_VALIDATION;
    if ((*objectIter)->IsEnabled()) {
      DO_VALIDATION;
      e_ObjectType objectType = (*objectIter)->GetObjectType();
      if (objectType == e_ObjectType_Geometry)
        visibleGeometry.push_back(
            boost::static_pointer_cast<Geometry>(*objectIter));
      else if (objectType == e_ObjectType_Light)
        visibleLights.push_back(boost::static_pointer_cast<Light>(*objectIter));
      else if (objectType == e_ObjectType_Skybox)
        skyboxes.push_back(boost::static_pointer_cast<Skybox>(*objectIter));
    }
    objectIter++;
  }

  // prepare shadow maps
  for (auto &light : visibleLights) {
    DO_VALIDATION;
    if (light->IsEnabled() && light->GetShadow()) {
      DO_VALIDATION;
      EnqueueShadowMap(camera, light);
    }
  }
  // enqueue camera view
  camera->EnqueueView(visibleGeometry, visibleLights, skyboxes);
  return true;
}

void GraphicsTask::EnqueueShadowMap(boost::intrusive_ptr<Camera> camera,
                                    boost::intrusive_ptr<Light> light) {
  DO_VALIDATION;

  std::deque<boost::intrusive_ptr<Geometry> > visibleGeometry;
  vector_Planes bounding;
  Plane plane(Vector3(0, -40, 0), Vector3(0, 1, 0.3).GetNormalized());
  bounding.push_back(plane);
  plane.Set(Vector3(0, 40, 0), Vector3(0, -1, 0.3).GetNormalized());
  bounding.push_back(plane);
  plane.Set(Vector3(-60, 0, 0), Vector3(1, 0, 0.3).GetNormalized());
  bounding.push_back(plane);
  plane.Set(Vector3(60, 0, 0), Vector3(-1, 0, 0.3).GetNormalized());
  bounding.push_back(plane);
  GetContext().scene3D->GetObjects<Geometry>(e_ObjectType_Geometry,
                                             visibleGeometry, bounding);
  // boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry,
  // visibleGeometry);
  light->EnqueueShadowMap(camera, visibleGeometry);
}
}
