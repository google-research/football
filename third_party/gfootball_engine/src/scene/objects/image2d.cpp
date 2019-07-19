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

#include "image2d.hpp"

#include <cmath>

#include "../../base/log.hpp"

#include "../../systems/isystemobject.hpp"

//#include "SDL2/SDL_gfxBlitFunc.h"

namespace blunted {

  Image2D::Image2D(std::string name) : Object(name, e_ObjectType_Image2D) {
    //printf("CREATING IMAGE\n");
  }

  Image2D::~Image2D() {
    //printf("DELETING IMAGE\n");
  }

  void Image2D::Exit() { // ATOMIC
    //printf("EXITING IMAGE\n");


    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers[i].get());
      image2DInterpreter->OnUnload();
    }

    Object::Exit();

    if (image) image.reset();

  }


  void Image2D::SetImage(boost::intrusive_ptr < Resource<Surface> > image) {

    this->image = image;

    position[0] = 0;
    position[1] = 0;
    size[0] = image->GetResource()->GetData()->w;
    size[1] = image->GetResource()->GetData()->h;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers[i].get());
      image2DInterpreter->OnLoad(image);
    }

  }

  boost::intrusive_ptr < Resource<Surface> > Image2D::GetImage() {

    return image;
  }

  void Image2D::SetPosition(int x, int y) {

    position[0] = x;
    position[1] = y;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers[i].get());
      image2DInterpreter->OnMove(x, y);
    }

  }

  void Image2D::SetPosition(const Vector3 &newPosition, bool updateSpatialData) {
    SetPosition(int(std::floor(newPosition.coords[0])),
                int(std::floor(newPosition.coords[1])));
  }

  Vector3 Image2D::GetPosition() const {
    Vector3 tmp(position[0], position[1], 0);

    return tmp;
  }

  Vector3 Image2D::GetSize() const {
    Vector3 tmp(size[0], size[1], 0);

    return tmp;
  }

  void Image2D::DrawRectangle(int x, int y, int w, int h, const Vector3 &color, int alpha) {
    SDL_Surface *surface = image->GetResource()->GetData();
    //SDL_LockSurface(surface);

    Uint32 color32;
    if (SDL_ISPIXELFORMAT_ALPHA(surface->format->format))
      color32 = SDL_MapRGBA(surface->format, int(std::floor(color.coords[0])),
                            int(std::floor(color.coords[1])),
                            int(std::floor(color.coords[2])), alpha);
    else
      color32 = SDL_MapRGB(surface->format, int(std::floor(color.coords[0])),
                           int(std::floor(color.coords[1])),
                           int(std::floor(color.coords[2])));

    sdl_rectangle_filled(surface, x, y, w, h, color32);
  }

  void Image2D::Resize(int w, int h) {
    image->GetResource()->Resize(w, h);
    size[0] = w;
    size[1] = h;

  }

  void Image2D::Poke(e_SystemType targetSystemType) {

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers[i].get());
      if (image2DInterpreter->GetSystemType() == targetSystemType) image2DInterpreter->OnPoke();
    }

  }


  // events

  void Image2D::OnChange() {
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers[i].get());
      image2DInterpreter->OnChange(image);
    }
  }

}
