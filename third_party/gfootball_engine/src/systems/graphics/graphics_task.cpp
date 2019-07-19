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

#include "../../managers/scenemanager.hpp"
#include "../../managers/environmentmanager.hpp"

#include "graphics_system.hpp"

#include "../../scene/objects/geometry.hpp"
#include "../../scene/objects/skybox.hpp"
#include "../../scene/objects/light.hpp"

#include "../../scene/scene3d/scene3d.hpp"

namespace blunted {

  GraphicsTask::GraphicsTask(GraphicsSystem *system) : ISystemTask(), graphicsSystem(system) {
  }

  GraphicsTask::~GraphicsTask() {
    graphicsSystem = NULL;
  }

  void GraphicsTask::GetPhase() {
    if (!enabled) return;

    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();


    // poke all image2D objects

    bool success = false;
    boost::shared_ptr<IScene> scene2d = SceneManager::GetInstance().GetScene("scene2D", success);
    if (success) {
      scene2d->PokeObjects(e_ObjectType_Image2D, e_SystemType_Graphics);
    }

    // collect visibles
    boost::shared_ptr<IScene> scene3d = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) {
      std::list < boost::intrusive_ptr<Camera> > cameras;
      boost::static_pointer_cast<Scene3D>(scene3d)->GetObjects<Camera>(e_ObjectType_Camera, cameras);
      std::list < boost::intrusive_ptr<Camera> >::iterator cameraIter = cameras.begin();
      // enqueue all camera views
      while (cameraIter != cameras.end()) {

        if ((*cameraIter)->IsEnabled()) {
          GraphicsTaskCommand_EnqueueView((*cameraIter)).Handle();
        }

        cameraIter++;
      }

    }
  }

  void GraphicsTask::ProcessPhase() {
    if (!enabled) return;

    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();

    // poke lights
    {
      bool success = false;
      boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
      if (success) {
        scene->PokeObjects(e_ObjectType_Light, e_SystemType_Graphics);
      }
    }

    // poke camera
    bool success = false;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) scene->PokeObjects(e_ObjectType_Camera, e_SystemType_Graphics);
    // render the Overlay2D queue
    auto &overlay2DQueue = graphicsSystem->GetOverlay2DQueue();
    bool isMessage = true;
    std::vector<Overlay2DQueueEntry> queue;
    while (isMessage) {
      Overlay2DQueueEntry queueEntry = overlay2DQueue.GetMessage(isMessage);
      if (isMessage) {
        queue.push_back(queueEntry);
      }
    }
    renderer3D->RenderOverlay2D(queue);
  }

  void GraphicsTask::PutPhase() {
    if (!enabled) return;

    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();

    // swap the buffers and stare in awe
    renderer3D->SwapBuffers();
  }

  bool GraphicsTaskCommand_EnqueueView::Execute(void *caller) {
    bool success = false;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);

    if (success) {
      // test camera->SetPosition(Vector3(sin((float)EnvironmentManager::GetInstance().GetTime_ms() * 0.001f) * 60, 0, 0), true);
      Vector3 cameraPos = camera->GetDerivedPosition();
      Quaternion cameraRot = camera->GetDerivedRotation();
      float nearCap, farCap;
      camera->GetCapping(nearCap, farCap);

      float fov = camera->GetFOV() * 2.0f;
      float wideScreenMultiplier = 2.5f;
      vector_Planes bounding;
      Plane plane(cameraPos + cameraRot * Vector3(0, 0, -nearCap), cameraRot * Vector3(0, 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(0, (3.6f * wideScreenMultiplier) / (fov / 24.0f), -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(0, (-3.6f * wideScreenMultiplier) / (fov / 24.0f), -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(2.4f / (fov / 24.0f), 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(-2.4f / (fov / 24.0f), 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos + cameraRot * Vector3(0, 0, -farCap), cameraRot * Vector3(0, 0, 1).GetNormalized());
      bounding.push_back(plane);

      // altogether function (may be slow in scenes with lots of objects)

      std::deque < boost::intrusive_ptr<Object> > visibleObjects;
      //std::list < boost::intrusive_ptr<Object> > visibleObjects;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects(visibleObjects, bounding);

      std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry;
      std::deque < boost::intrusive_ptr<Light> > visibleLights;
      std::deque < boost::intrusive_ptr<Skybox> > skyboxes;

      std::deque < boost::intrusive_ptr<Object> >::iterator objectIter = visibleObjects.begin();
      while (objectIter != visibleObjects.end()) {
        if ((*objectIter)->IsEnabled()) {
          e_ObjectType objectType = (*objectIter)->GetObjectType();
          if      (objectType == e_ObjectType_Geometry) visibleGeometry.push_back(boost::static_pointer_cast<Geometry>(*objectIter));
          else if (objectType == e_ObjectType_Light)    visibleLights.push_back(boost::static_pointer_cast<Light>(*objectIter));
          else if (objectType == e_ObjectType_Skybox)   skyboxes.push_back(boost::static_pointer_cast<Skybox>(*objectIter));
        }
        objectIter++;
      }


      // prepare shadow maps
      for (auto& light : visibleLights) {
        if (light->IsEnabled() && light->GetShadow()) {
          EnqueueShadowMap(light);
        }
      }
      // enqueue camera view

      camera->EnqueueView(visibleGeometry, visibleLights, skyboxes);
    }

    return true;
  }

  void GraphicsTaskCommand_EnqueueView::EnqueueShadowMap(boost::intrusive_ptr<Light> light) {

    bool success = false;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) {
      std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry;
      vector_Planes bounding;
      Plane plane(Vector3(0, -40, 0), Vector3(0, 1, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(0, 40, 0), Vector3(0, -1, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(-60, 0, 0), Vector3(1, 0, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(60, 0, 0), Vector3(-1, 0, 0.3).GetNormalized());
      bounding.push_back(plane);
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry, visibleGeometry, bounding);
      //boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry, visibleGeometry);
      light->EnqueueShadowMap(camera, visibleGeometry);
    }
  }

}
