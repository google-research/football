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

#include "surface.hpp"

#include <SDL2_rotozoom.h>

#include <cmath>

#include "../../base/log.hpp"

namespace blunted {

#define SMOOTHING_OFF    0
#define SMOOTHING_ON   1

  Surface::Surface() : surface(0) {
    //printf("CREATING SURFACE\n");
  }

  Surface::~Surface() {
    //printf("ANNIHILATING SURFACE.. ");
    if (surface) {
      SDL_FreeSurface(surface);
      surface = 0;
    }
  }

  Surface::Surface(const Surface &src) {
    this->surface = SDL_ConvertSurface(src.surface, src.surface->format, 0);
    assert(this->surface);
  }

  SDL_Surface *Surface::GetData() {
    return surface;
  }

  void Surface::SetData(SDL_Surface *surface) {
    if (this->surface) SDL_FreeSurface(this->surface);
    this->surface = surface;
  }

  void Surface::Resize(int x, int y) {

    // zoomSurface doesn't seem to create a completely new surface; got some weird segfaults.
    // not 100% sure if it's their fault though, or if i'm doing something wrong. either way,
    // it works with this fix, though it's a bit of a performance hit, an extra surface copy.
    bool buggyZoomSurface = true;

    assert(this->surface);
    int xcur = this->surface->w;
    int ycur = this->surface->h;
    double xfac, yfac;
    xfac = x / (xcur * 1.0);
    yfac = y / (ycur * 1.0);
    if (yfac == 0) yfac = xfac;
    if (xfac == 0) xfac = yfac;
    if (xfac == 0 || yfac == 0) return;
    SDL_Surface *newSurf = zoomSurface(this->surface, xfac, yfac, SMOOTHING_ON);
    //printf("resize factors: %f %f\n", xfac, yfac);
    //printf("surface size: %i %i\n", this->surface->w, this->surface->h);
    //printf("new surface size: %i %i\n", newSurf->w, newSurf->h);
    SDL_FreeSurface(this->surface);
    if (buggyZoomSurface) {
      this->surface = SDL_ConvertSurface(newSurf, newSurf->format, 0);
      SDL_FreeSurface(newSurf);
    } else {
      this->surface = newSurf;
    }
  }
}
