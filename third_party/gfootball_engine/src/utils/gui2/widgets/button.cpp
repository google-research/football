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

#include "button.hpp"

#include <cmath>

//#include "SDL2/SDL_gfxBlitFunc.h"

namespace blunted {

  Gui2Button::Gui2Button(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {

    isSelectable = true;

    color = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image = windowManager->CreateImage2D(name, w, h, true);

    captionView = new Gui2Caption(windowManager, name + "caption", 1.2, 0.3, width_percent, height_percent - 0.6, caption);
    this->AddView(captionView);
    captionView->Show();

    fadeOutTime_ms = 200;
    fadeOut_ms = fadeOutTime_ms;

    toggleable = false;
    toggled = false;
    active = true;

    Redraw();
  }

  Gui2Button::~Gui2Button() {
  }

  void Gui2Button::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    target.push_back(image);
    Gui2View::GetImages(target);
  }

  void Gui2Button::Process() {
    //printf("gui2button %s :: processing\n", name.c_str());
    if (fadeOut_ms <= fadeOutTime_ms) {
      fadeOut_ms += windowManager->GetTimeStep_ms();
      if (!IsFocussed() && fadeOut_ms <= fadeOutTime_ms) { // cool fadeout effect!
        Redraw();
      }
    }

    Gui2View::Process();
  }

  void Gui2Button::SetColor(const Vector3 &color) {
    this->color = color;
    captionView->SetColor(color);
    Redraw();
  }

  void Gui2Button::Redraw() {
    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    float x_ratio = w / width_percent; // 1% width
    float y_ratio = h / height_percent; // 1% width
    int x_margin = int(round(x_ratio * 0.5));
    int y_margin = int(round(y_ratio * 0.5));

    int alpha = 0;
    Vector3 color1;
    if (IsFocussed()) {
      alpha = 200;

      color1 = windowManager->GetStyle()->GetColor(e_DecorationType_Bright2);
      if (toggleable && toggled) color1 = windowManager->GetStyle()->GetColor(e_DecorationType_Toggled);

    } else {
      alpha = int(std::floor(200 - (fadeOut_ms / (float)fadeOutTime_ms * 100)));
      float bias = fadeOut_ms / (float)fadeOutTime_ms;

      Vector3 dark1 = windowManager->GetStyle()->GetColor(e_DecorationType_Dark1);
      if (toggleable && toggled) dark1 = windowManager->GetStyle()->GetColor(e_DecorationType_Toggled);

      Vector3 selectedColor = windowManager->GetStyle()->GetColor(e_DecorationType_Bright2);
      if (toggleable && toggled) selectedColor = windowManager->GetStyle()->GetColor(e_DecorationType_Toggled);

      color1 = selectedColor * (1 - bias) + dark1 * bias;
    }
    Vector3 color2 = color;
    if (!active) {
      color2 = windowManager->GetStyle()->GetColor(e_DecorationType_Dark2);
    }
    image->DrawRectangle(0, 0, w, h, color2, alpha);
    image->DrawRectangle(x_margin, 0, w - x_margin * 2, h, color1, alpha);

    image->OnChange();
  }

  void Gui2Button::ProcessWindowingEvent(WindowingEvent *event) {
    if (event->IsActivate() && active) {
      if (toggleable) {
        if (toggled) { toggled = false; } else { toggled = true; }
        Redraw();
      }
      sig_OnClick(this);
    } else {
      event->Ignore();
    }
  }

  void Gui2Button::OnGainFocus() {
    Redraw();
    sig_OnGainFocus(this);
  }

  void Gui2Button::OnLoseFocus() {
    fadeOut_ms = 0;
    sig_OnLoseFocus(this);
  }

}
