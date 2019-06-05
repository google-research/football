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

#include "scrollbar.hpp"

//#include "SDL2/SDL_gfxBlitFunc.h"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2Scrollbar::Gui2Scrollbar(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {

    mode = e_Gui2ScrollbarMode_Vertical;

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image = windowManager->CreateImage2D(name, w, h, true);

    Redraw();
  }

  Gui2Scrollbar::~Gui2Scrollbar() {
  }

  void Gui2Scrollbar::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    target.push_back(image);
    Gui2View::GetImages(target);
  }

  void Gui2Scrollbar::Process() {
    Gui2View::Process();
  }

  void Gui2Scrollbar::Redraw() {
    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

    Vector3 color1 = windowManager->GetStyle()->GetColor(e_DecorationType_Dark1);
    Vector3 color2 = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);

    // background
    image->DrawRectangle(0, 0, w, h, color1, 255);

    // bar
    if (mode == e_Gui2ScrollbarMode_Vertical) {
      int barSize = size_percent * 0.01 * h;
      int barStart = progress_percent * 0.01 * h;
      barStart -= barSize * progress_percent * 0.01;
      image->DrawRectangle(0, barStart, w, barSize, color2, 255);
    } else {
      int barSize = size_percent * 0.01 * w;
      int barStart = progress_percent * 0.01 * w;
      barStart -= barSize * progress_percent * 0.01;
      image->DrawRectangle(barStart, 0, barSize, h, color2, 255);
    }

    image->OnChange();
  }

}
