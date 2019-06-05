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

#ifndef _HPP_GUI2_VIEW_SLIDER
#define _HPP_GUI2_VIEW_SLIDER

#include "wrap_SDL_ttf.h"

#include "../view.hpp"

#include "../../../scene/objects/image2d.hpp"

#include "caption.hpp"

namespace blunted {

  struct Gui2Slider_HelperValue {
    int index = 0;
    Vector3 color;
    float value = 0.0f;
    Gui2Caption *descriptionCaption;
  };

  class Gui2Slider : public Gui2View {

    public:
      Gui2Slider(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Slider();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Process();
      virtual void Redraw();

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      virtual void Show() { titleCaption->Show(); Gui2View::Show(); } // ignore helper descriptions

      virtual void OnGainFocus();
      virtual void OnLoseFocus();

      void SetValue(float newValue);
      float GetValue() { return quantizedValue; }

      int AddHelperValue(const Vector3 &color, const std::string &description,
                         float initialValue = 0.0f);

      boost::signal<void(Gui2Slider*)> sig_OnChange;

    protected:
      boost::intrusive_ptr<Image2D> image;

      int fadeOut_ms = 0;
      int fadeOutTime_ms = 0;
      int switchHelperDescription_ms = 0;
      int switchHelperDescriptionTime_ms = 0;
      int activeDescription = 0;
      int quantizationSteps = 0;

      std::string caption;

      Gui2Caption *titleCaption;

      std::vector<Gui2Slider_HelperValue> helperValues;

      float value = 0.0f;
      float quantizedValue = 0.0f;

  };

}

#endif
