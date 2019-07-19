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

#include "texture.hpp"

#include "../rendering/r3d_messages.hpp"

namespace blunted {

  Texture::Texture() : textureID(-1) {
    //printf("CREATING TEXTUREID\n");
    this->renderer3D = 0;
    textureID = -1;
    width = 0;
    height = 0;
  }

  Texture::~Texture() {
    //printf("ERASING TEXTURE ID #%i\n", textureID);

    DeleteTexture();
    //printf(" [ok]\n");
  }

  void Texture::SetRenderer3D(Renderer3D *renderer3D) {
    this->renderer3D = renderer3D;
  }

  void Texture::DeleteTexture() {
    if (textureID != -1) {
      assert(renderer3D);
      renderer3D->DeleteTexture(textureID);
      textureID = -1;
    }
  }

  int Texture::CreateTexture(e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, int width, int height, bool alpha, bool repeat, bool mipmaps, bool filter, bool compareDepth) {
    assert(renderer3D);

    textureID = renderer3D->CreateTexture(internalPixelFormat, pixelFormat, width, height, alpha, repeat, mipmaps, filter, false, compareDepth);

    this->width = width;
    this->height = height;

    return textureID;
  }

  void Texture::ResizeTexture(SDL_Surface *image, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha, bool mipmaps) {
    assert(renderer3D);
    assert(textureID != -1);

    bool _alpha = SDL_ISPIXELFORMAT_ALPHA(image->format->format);
    renderer3D->ResizeTexture(textureID, image, internalPixelFormat, pixelFormat, _alpha, mipmaps);
  }

  void Texture::UpdateTexture(SDL_Surface *image, bool alpha, bool mipmaps) {
    assert(renderer3D);
    assert(textureID != -1);

    bool _alpha = SDL_ISPIXELFORMAT_ALPHA(image->format->format);
    renderer3D->UpdateTexture(textureID, image, _alpha, mipmaps);
  }

  int Texture::GetID() {
    return textureID;
  }

}
