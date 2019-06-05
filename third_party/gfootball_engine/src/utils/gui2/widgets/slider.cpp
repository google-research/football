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

#include "slider.hpp"

#include <cmath>

#include "../windowmanager.hpp"

//#include "SDL2/SDL_gfxBlitFunc.h"

namespace blunted {

  Gui2Slider::Gui2Slider(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), quantizationSteps(51), caption(caption) {

    isSelectable = true;

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image = windowManager->CreateImage2D(name, w, h, true);

    fadeOutTime_ms = 200;
    fadeOut_ms = fadeOutTime_ms;
    switchHelperDescriptionTime_ms = -1;
    switchHelperDescription_ms = 0;
    activeDescription = -1;

    value = 0.5f;
    quantizedValue = value;

    titleCaption = new Gui2Caption(windowManager, name + "caption", 1.0, 0.2, width_percent, 2.4, caption);
    this->AddView(titleCaption);
    titleCaption->Show();

    Redraw();
  }

  Gui2Slider::~Gui2Slider() {
  }

  void Gui2Slider::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    target.push_back(image);
    Gui2View::GetImages(target);
  }

  void Gui2Slider::Process() {
    //printf("gui2slider %s :: processing\n", name.c_str());
    if (fadeOut_ms <= fadeOutTime_ms) {
      fadeOut_ms += windowManager->GetTimeStep_ms();
      if (!IsFocussed() && fadeOut_ms <= fadeOutTime_ms) { // cool fadeout effect!
        Redraw();
      }
    }

    if (IsFocussed()) switchHelperDescription_ms += windowManager->GetTimeStep_ms();
    if (switchHelperDescriptionTime_ms != -1 && switchHelperDescription_ms > switchHelperDescriptionTime_ms) {
      switchHelperDescription_ms = 0;
      activeDescription++;
      if (activeDescription >= (signed int)helperValues.size()) activeDescription = -1; // reset loop
      if (!IsFocussed()) activeDescription = -1;
      for (int i = -1; i < (signed int)helperValues.size(); i++) {
        if (i != activeDescription) {

          if (i == -1) {
            titleCaption->Hide();
          } else {
            helperValues.at((unsigned int)i).descriptionCaption->Hide();
          }

        } else {

          if (i == -1) {
            titleCaption->Show();
          } else {
            helperValues.at((unsigned int)i).descriptionCaption->Show();
          }

        }
      }
    }


    Gui2View::Process();
  }

  void Gui2Slider::Redraw() {
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
    } else {
      alpha = int(std::floor(200 - (fadeOut_ms / (float)fadeOutTime_ms * 100)));
      float bias = fadeOut_ms / (float)fadeOutTime_ms;
      color1 = windowManager->GetStyle()->GetColor(e_DecorationType_Bright2) * (1 - bias) + windowManager->GetStyle()->GetColor(e_DecorationType_Dark1) * bias;
    }
    Vector3 color2 = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);

    // sides
    image->DrawRectangle(0, 0, x_margin, h, color2, alpha);
    image->DrawRectangle(w - x_margin, 0, x_margin, h, color2, alpha);

    // main
    image->DrawRectangle(x_margin, 0, w - x_margin * 2, h, color1, alpha);

    // helper sliders
    for (unsigned int i = 0; i < helperValues.size(); i++) {
      Vector3 helperColor = helperValues.at(i).color * 0.5f + color1 * 0.5f;
      float helperValue = helperValues.at(i).value;
      image->DrawRectangle(x_margin * 2 + helperValue * (w - x_margin * 6), h * 0.5, x_ratio, h * 0.5, helperColor, alpha);
    }

    // slider groove
    image->DrawRectangle(x_margin * 2, h * 0.7, w - x_margin * 4, h * 0.1, color2, 255);

    // the slider
    image->DrawRectangle(x_margin * 2 + quantizedValue * (w - x_margin * 6), h * 0.5, x_ratio, h * 0.5, color2, 160);

    image->OnChange();
  }

  void Gui2Slider::ProcessWindowingEvent(WindowingEvent *event) {

    Vector3 direction = event->GetDirection();

    float xoffset = 0;
    if (direction.coords[0] < -0.3f)
      xoffset = -std::pow(-direction.coords[0], 2.0f);
    if (direction.coords[0] > 0.3f)
      xoffset = std::pow(direction.coords[0], 2.0f);

    bool fullStep = false;
    if (direction.coords[0] > 0.99f || direction.coords[0] < 0.01f) { // full effect, maybe digital input, so make this 1 quantized step
      fullStep = true;
    }

    if (xoffset != 0) {
      if (!fullStep) {
        value += xoffset * 0.01f;
      } else {
        value += xoffset * (1.0f / (quantizationSteps - 1.0f));
      }
      if (value > 1.0f) value = 1.0f;
      if (value < 0.0f) value = 0.0f;
      quantizedValue = std::round(value * (quantizationSteps - 1)) /
                       (quantizationSteps - 1.0f);
      sig_OnChange(this);
      Redraw();
    } else {
      event->Ignore();
    }

  }

  void Gui2Slider::OnGainFocus() {
    Redraw();
  }

  void Gui2Slider::OnLoseFocus() {
    fadeOut_ms = 0;
    switchHelperDescription_ms = 0;
    activeDescription = -1;
    for (unsigned int i = 0; i < helperValues.size(); i++) {
      helperValues.at(i).descriptionCaption->Hide();
    }
    titleCaption->Show();
  }

  void Gui2Slider::SetValue(float newValue) {
    value = clamp(newValue, 0.0f, 1.0f);
    quantizedValue = std::round(value * (quantizationSteps - 1)) /
                     (quantizationSteps - 1.0f);
    Redraw();
  }

  int Gui2Slider::AddHelperValue(const Vector3 &color, const std::string &description, float initialValue) {
    Gui2Slider_HelperValue helper;
    helper.index = helperValues.empty() ? 1 : helperValues.at(helperValues.size() - 1).index + 1;
    helper.color = color;
    helper.descriptionCaption = new Gui2Caption(windowManager, "gui2sliderhelper_" + int_to_str(helper.index) + "_description_caption", 1.0, 0.2, width_percent, 2.4, description);
    helper.descriptionCaption->SetColor(helper.color);
    helper.descriptionCaption->SetTransparency(0.3f);
    this->AddView(helper.descriptionCaption);
    helper.descriptionCaption->Hide();
    helper.value = clamp(initialValue, 0.0f, 1.0f);
    helperValues.push_back(helper);
    return helper.index;
    Redraw();
  }
}
