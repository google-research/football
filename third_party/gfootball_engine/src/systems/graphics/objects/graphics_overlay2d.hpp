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

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_OVERLAY2D
#define _HPP_GRAPHICSSYSTEM_OBJECT_OVERLAY2D

#include "../../../scene/objects/image2d.hpp"

#include "../graphics_object.hpp"

#include "../resources/texture.hpp"

namespace blunted {

  class GraphicsOverlay2D_Image2DInterpreter;

  class GraphicsOverlay2D : public GraphicsObject {

    public:
      GraphicsOverlay2D(GraphicsScene *graphicsScene);
      virtual ~GraphicsOverlay2D();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      boost::intrusive_ptr < Resource<Texture> > texture;

      int position[2] = {0, 0};
      int size[2] = {0, 0};

    protected:

  };

  class GraphicsOverlay2D_Image2DInterpreter : public IImage2DInterpreter {

    public:
      GraphicsOverlay2D_Image2DInterpreter(GraphicsOverlay2D *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad(boost::intrusive_ptr < Resource<Surface> > surface);
      virtual void OnUnload();
      virtual void OnChange(boost::intrusive_ptr < Resource<Surface> > surface);
      virtual void OnMove(int x, int y);
      virtual void OnPoke();

    protected:
      GraphicsOverlay2D *caller;

  };
}

#endif
