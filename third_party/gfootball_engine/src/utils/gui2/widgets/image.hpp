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

#ifndef _HPP_GUI2_VIEW_IMAGE
#define _HPP_GUI2_VIEW_IMAGE

#include "../view.hpp"

#include "../../../scene/objects/image2d.hpp"


#include "wrap_SDL_surface.h"

namespace blunted {

  SDL_Surface* IMG_LoadBmp(const std::string&);

  class Gui2Image : public Gui2View {

    public:
      Gui2Image(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Image();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      void LoadImage(const std::string &filename);
      virtual void Redraw();

      virtual void SetSize(float new_width_percent, float new_height_percent);
      virtual void SetZoom(float zoomx, float zoomy);

    protected:
      boost::intrusive_ptr<Image2D> image;
      boost::intrusive_ptr<Image2D> imageSource;

  };

}

#endif
