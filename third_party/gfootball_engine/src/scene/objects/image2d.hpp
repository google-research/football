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

#ifndef _HPP_OBJECT_IMAGE2D
#define _HPP_OBJECT_IMAGE2D

#include "../../defines.hpp"
#include "../../scene/object.hpp"
#include "../../scene/resources/surface.hpp"
#include "../../types/interpreter.hpp"
#include "../../base/math/vector3.hpp"
#include "../../base/geometry/line.hpp"

#include "wrap_SDL_ttf.h"

namespace blunted {

  class Image2D : public Object {

    public:
      Image2D(std::string name);
      virtual ~Image2D();

      virtual void Exit();

      void SetImage(boost::intrusive_ptr < Resource<Surface> > image);
      boost::intrusive_ptr < Resource<Surface> > GetImage();

      void SetPosition(int x, int y);
      virtual void SetPosition(const Vector3 &newPosition, bool updateSpatialData = true);
      virtual Vector3 GetPosition() const;
      Vector3 GetSize() const;
      void DrawRectangle(int x, int y, int w, int h, const Vector3 &color,
                         int alpha = 255);
      void Resize(int w, int h);

      virtual void Poke(e_SystemType targetSystemType);

      void OnChange();

    protected:
      int position[2];
      int size[2];
      boost::intrusive_ptr < Resource<Surface> > image;

  };

  class IImage2DInterpreter : public Interpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr < Resource<Surface> > surface) = 0;
      virtual void OnUnload() = 0;
      virtual void OnChange(boost::intrusive_ptr < Resource<Surface> > surface) = 0;
      virtual void OnMove(int x, int y) = 0;
      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
