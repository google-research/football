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

#include "sdl_surface.hpp"
#include "geometry/triangle.hpp"

namespace blunted {

  SDL_Surface *CreateSDLSurface(int width, int height) {
    return SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL, width, height, 32, r_mask, g_mask, b_mask, a_mask);
  }

  Uint32 sdl_getpixel(const SDL_Surface *surface, int x, int y) {

    int bpp = surface->format->BytesPerPixel;
    // Here p is the address to the pixel we want to retrieve
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0; // shouldn't happen, but avoids warnings
    }
  }

  void sdl_rectangle_filled(SDL_Surface *surface, int x, int y, int width, int height, Uint32 color) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    SDL_FillRect(surface, &rect, color);
  }

  void sdl_setsurfacealpha(SDL_Surface *surface, int alpha) {
    // original from: Warren Schwader
    // sdl at libsdl.org

    // the surface width and height
    int width = surface->w;
    int height = surface->h;

    // the pitch of the surface. Used to determine where the next vertical line in pixels starts
    int pitch = surface->pitch;
    int bpp = surface->format->BytesPerPixel;

    // NOTE: since we are only modifying the alpha bytes we could simply read only that byte and then add 4 to get
    // to the next alpha byte. That approach depends on the endianess of the alpha byte and also the 32 bit pixel depth
    // so if those things change then this code will need changing too. but we already rely on 32 bpp

    SDL_LockSurface(surface);
    for (int h = 0; h < height; h++) {

      for (int w = 0; w < width; w++) {
        Uint8 *p = (Uint8 *)surface->pixels + h * pitch + w * bpp;

        p[3] = clamp((p[3] / 256.0f) * (alpha / 256.0f) * 256, 0, 255);
      }

    }
    SDL_UnlockSurface(surface);

  }

}
