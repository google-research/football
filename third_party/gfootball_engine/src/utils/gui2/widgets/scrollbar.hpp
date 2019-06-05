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

#ifndef _HPP_GUI2_VIEW_SCROLLBAR
#define _HPP_GUI2_VIEW_SCROLLBAR

#include "wrap_SDL_ttf.h"

#include "../view.hpp"

#include "../../../scene/objects/image2d.hpp"

namespace blunted {

  enum e_Gui2ScrollbarMode {
    e_Gui2ScrollbarMode_Horizontal,
    e_Gui2ScrollbarMode_Vertical
  };

  class Gui2Scrollbar : public Gui2View {

    public:
      Gui2Scrollbar(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Scrollbar();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Process();

      virtual void Redraw();

      void SetSizePercent(float newValue) { size_percent = newValue; }
      void SetProgressPercent(float newValue) { progress_percent = newValue; }

    protected:
      boost::intrusive_ptr<Image2D> image;

      e_Gui2ScrollbarMode mode;
      float size_percent = 0.0f;
      float progress_percent = 0.0f;

  };

}

#endif
