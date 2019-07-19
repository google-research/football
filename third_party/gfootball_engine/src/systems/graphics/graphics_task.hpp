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

#ifndef _HPP_SYSTEMS_GRAPHICS_TASK
#define _HPP_SYSTEMS_GRAPHICS_TASK

#include "../../defines.hpp"

#include "../../systems/isystemtask.hpp"
#include "rendering/interface_renderer3d.hpp"
#include "rendering/r3d_messages.hpp"

#include "../../scene/objects/camera.hpp"

namespace blunted {

  class GraphicsSystem;

  class GraphicsTask : public ISystemTask {

    public:
      GraphicsTask(GraphicsSystem *system);
      virtual ~GraphicsTask();

      virtual void operator()() {}

    protected:
      void GetPhase();
      void ProcessPhase();
      void PutPhase();

      GraphicsSystem *graphicsSystem;
  };

  class GraphicsTaskCommand_EnqueueView : public Command {

    public:
      GraphicsTaskCommand_EnqueueView(boost::intrusive_ptr<Camera> camera) : Command("EnqueueView"), camera(camera) {};

    protected:
      virtual bool Execute(void *caller = NULL);
      virtual void EnqueueShadowMap(boost::intrusive_ptr<Light> light);

      boost::intrusive_ptr<Camera> camera;
  };
}

#endif
