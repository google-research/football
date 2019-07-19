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

#ifndef _hpp_sdl_surface
#define _hpp_sdl_surface

#include "wrap_SDL_endian.h"
#include "wrap_SDL_surface.h"

#include <wrap_SDL.h>

namespace blunted {

  class Triangle;

  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  static const Uint32 r_mask = 0xFF000000;
  static const Uint32 g_mask = 0x00FF0000;
  static const Uint32 b_mask = 0x0000FF00;
  static const Uint32 a_mask = 0x000000FF;
  #else
  static const Uint32 r_mask = 0x000000FF;
  static const Uint32 g_mask = 0x0000FF00;
  static const Uint32 b_mask = 0x00FF0000;
  static const Uint32 a_mask = 0xFF000000;
  #endif

  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  static const Uint8 r_mask8 = 0xC0;
  static const Uint8 g_mask8 = 0x80;
  static const Uint8 b_mask8 = 0x40;
  static const Uint8 a_mask8 = 0x00;
#else
  static const Uint8 r_mask8 = 0x00;
  static const Uint8 g_mask8 = 0x40;
  static const Uint8 b_mask8 = 0x80;
  static const Uint8 a_mask8 = 0xC0;
#endif

  SDL_Surface *CreateSDLSurface(int width, int height);
  Uint32 sdl_getpixel(const SDL_Surface *surface, int x, int y);
  void sdl_rectangle_filled(SDL_Surface *surface, int x, int y, int width,
                            int height, Uint32 color);

  // only works on 32-bits surfaces
  // might have endian problems
  void sdl_setsurfacealpha(SDL_Surface *surface, int alpha);

}

#endif
