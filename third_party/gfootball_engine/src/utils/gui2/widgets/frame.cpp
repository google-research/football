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

#include "frame.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2Frame::Gui2Frame(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, bool background) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {
    if (background) {
      Gui2Image *bg = new Gui2Image(windowManager, name + "_frame", 0, 0, width_percent, height_percent);
      this->AddView(bg);
      bg->LoadImage("media/menu/backgrounds/black.png");
      bg->Show();
    }
  }

  Gui2Frame::~Gui2Frame() {
  }

}
