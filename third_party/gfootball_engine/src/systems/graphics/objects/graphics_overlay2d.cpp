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

#include "graphics_overlay2d.hpp"

#include "../../../managers/resourcemanagerpool.hpp"

#include "../graphics_scene.hpp"
#include "../graphics_system.hpp"

namespace blunted {

  GraphicsOverlay2D::GraphicsOverlay2D(GraphicsScene *graphicsScene) : GraphicsObject(graphicsScene) {
    //printf("CREATING GFX OBJECT\n");
  }

  GraphicsOverlay2D::~GraphicsOverlay2D() {
  }

  boost::intrusive_ptr<Interpreter> GraphicsOverlay2D::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Image2D) {
      boost::intrusive_ptr<GraphicsOverlay2D_Image2DInterpreter> image2DInterpreter(new GraphicsOverlay2D_Image2DInterpreter(this));
      return image2DInterpreter;
    }
    Log(e_FatalError, "GraphicsOverlay2D", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<GraphicsOverlay2D_Image2DInterpreter>();
  }

  GraphicsOverlay2D_Image2DInterpreter::GraphicsOverlay2D_Image2DInterpreter(GraphicsOverlay2D *caller) : caller(caller) {
  }

  void GraphicsOverlay2D_Image2DInterpreter::OnLoad(boost::intrusive_ptr < Resource<Surface> > surface) {

    bool alreadyThere = false;

    caller->texture =
      ResourceManagerPool::getTextureManager()->
        Fetch(surface->GetIdentString(), false, alreadyThere, true); // false == don't try to use loader

    SDL_Surface *image = surface->GetResource()->GetData();

    if (!alreadyThere) {
      Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

      caller->texture->GetResource()->SetRenderer3D(renderer3D);
      bool alpha = SDL_ISPIXELFORMAT_ALPHA((image->format->format));
      caller->texture->GetResource()->CreateTexture(e_InternalPixelFormat_RGBA8, alpha ? e_PixelFormat_RGBA : e_PixelFormat_RGB, image->w, image->h, alpha, false, false, false);
      caller->texture->GetResource()->UpdateTexture(image, alpha, false);
      caller->position[0] = 0;
      caller->position[1] = 0;
      caller->size[0] = image->w;
      caller->size[1] = image->h;
    } else {
      OnChange(surface);
    }

    //printf("texture id: %i\n", caller->textureID);
  }

  void GraphicsOverlay2D_Image2DInterpreter::OnUnload() {
    //printf("resetting link to texture.. ");
    caller->texture.reset();
    delete caller;
    caller = 0;
    //printf("[OK]\n");
  }

  void GraphicsOverlay2D_Image2DInterpreter::OnChange(boost::intrusive_ptr < Resource<Surface> > surface) {
    SDL_Surface *image = surface->GetResource()->GetData();
    assert(image);
    assert(caller->texture);
    bool alpha = SDL_ISPIXELFORMAT_ALPHA((image->format->format));
    if (caller->size[0] != image->w || caller->size[1] != image->h) {
      // surface was resized
      caller->size[0] = image->w;
      caller->size[1] = image->h;
      caller->texture->GetResource()->ResizeTexture(image, e_InternalPixelFormat_RGBA8, alpha ? e_PixelFormat_RGBA : e_PixelFormat_RGB, alpha, false);
    } else {
      // no resize, just update
      caller->texture->GetResource()->UpdateTexture(image, alpha, false);
    }
  }

  void GraphicsOverlay2D_Image2DInterpreter::OnMove(int x, int y) {
    caller->position[0] = x;
    caller->position[1] = y;
  }

  void GraphicsOverlay2D_Image2DInterpreter::OnPoke() {
    Overlay2DQueueEntry queueEntry;
    queueEntry.texture = caller->texture;
    queueEntry.position[0] = caller->position[0];
    queueEntry.position[1] = caller->position[1];
    queueEntry.size[0] = caller->size[0];
    queueEntry.size[1] = caller->size[1];
    caller->GetGraphicsScene()->GetGraphicsSystem()->GetOverlay2DQueue().PushMessage(queueEntry);
  }

}
