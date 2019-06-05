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

#ifndef _HPP_SYSTEM_GRAPHICS_RESOURCE_TEXTURE
#define _HPP_SYSTEM_GRAPHICS_RESOURCE_TEXTURE

#include "../../../defines.hpp"

#include "../../../base/sdl_surface.hpp"

#include "../../../systems/graphics/rendering/interface_renderer3d.hpp"

namespace blunted {

  class Renderer3D;

  class Texture {

    public:
      Texture();
      virtual ~Texture();

      void SetRenderer3D(Renderer3D *renderer3D);

      void DeleteTexture();
      int CreateTexture(e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, int width, int height, bool alpha, bool repeat, bool mipmaps, bool filter, bool compareDepth = false);
      void ResizeTexture(SDL_Surface *image, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha, bool mipmaps);
      void UpdateTexture(SDL_Surface *image, bool alpha, bool mipmaps);

      int GetID();

      void GetSize(int &width, int &height) const { width = this->width; height = this->height; }

    protected:
      int textureID = 0;
      Renderer3D *renderer3D;
      int width, height;

  };

}

#endif
