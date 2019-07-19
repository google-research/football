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

#ifndef _HPP_SYSTEMS_GRAPHICS_SYSTEM
#define _HPP_SYSTEMS_GRAPHICS_SYSTEM

#include "../../defines.hpp"

#include "../../types/messagequeue.hpp"
#include "../../systems/isystem.hpp"
#include "../../systems/isystemscene.hpp"
#include "../../systems/graphics/rendering/opengl_renderer3d.hpp"

#include "../../scene/iscene.hpp"

#include "graphics_task.hpp"

#include "resources/texture.hpp"
#include "resources/vertexbuffer.hpp"

#include "../../managers/resourcemanager.hpp"

namespace blunted {

  class Renderer3D;

  class GraphicsSystem : public ISystem {

    public:
      GraphicsSystem();
      virtual ~GraphicsSystem();

      virtual void Initialize(const Properties &config);
      virtual void Exit();
      const screenshoot& GetScreen();

      e_SystemType GetSystemType() const;

      virtual ISystemScene *CreateSystemScene(boost::shared_ptr<IScene> scene);

      virtual ISystemTask *GetTask();
      virtual Renderer3D *GetRenderer3D();

      MessageQueue<Overlay2DQueueEntry> &GetOverlay2DQueue();

      Vector3 GetContextSize() { return Vector3(width, height, bpp); }

      virtual std::string GetName() const { return "graphics"; }

      //boost::mutex getPhaseMutex;

    protected:
      const e_SystemType systemType = e_SystemType_Graphics;

      Renderer3D *renderer3DTask = 0;

      GraphicsTask *task = 0;

      MessageQueue<Overlay2DQueueEntry> overlay2DQueue;

      int width = 0, height = 0, bpp = 0;

  };

}

#endif
