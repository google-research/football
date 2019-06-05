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

#ifndef _HPP_SYSTEMS_GRAPHICS_SCENE
#define _HPP_SYSTEMS_GRAPHICS_SCENE

#include "../../defines.hpp"

#include "../../systems/isystemscene.hpp"

#include "../../scene/scene3d/scene3d.hpp"
#include "../../scene/scene2d/scene2d.hpp"

namespace blunted {

  class GraphicsSystem;

  class GraphicsScene : public ISystemScene {

    public:
      GraphicsScene(GraphicsSystem *graphicsSystem);
      virtual ~GraphicsScene();

      virtual GraphicsSystem *GetGraphicsSystem();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

      virtual boost::intrusive_ptr<ISceneInterpreter> GetInterpreter(e_SceneType sceneType);

    protected:
      GraphicsSystem *graphicsSystem;

  };


  class GraphicsScene_Scene3DInterpreter : public IScene3DInterpreter {

    public:
      GraphicsScene_Scene3DInterpreter(GraphicsScene *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad();
      virtual void OnUnload();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

      virtual void SetGravity(const Vector3 &gravity) {}
      virtual void SetErrorCorrection(float value) {}
      virtual void SetConstraintForceMixing(float value) {}

    protected:
      GraphicsScene *caller;

  };

  class GraphicsScene_Scene2DInterpreter : public IScene2DInterpreter {

    public:
      GraphicsScene_Scene2DInterpreter(GraphicsScene *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad();
      virtual void OnUnload();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

    protected:
      GraphicsScene *caller;

  };

}

#endif
