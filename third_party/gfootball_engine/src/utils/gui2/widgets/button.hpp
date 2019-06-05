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

#ifndef _HPP_GUI2_VIEW_BUTTON
#define _HPP_GUI2_VIEW_BUTTON

#include "wrap_SDL_ttf.h"

#include "../view.hpp"

#include "../../../scene/objects/image2d.hpp"

#include "caption.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  class Gui2Button : public Gui2View {

    public:
      Gui2Button(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Button();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);
      virtual Gui2Caption *GetCaptionWidget() { return captionView; }
      virtual std::string GetCaption() { return captionView->GetCaption(); }
      virtual void SetCaption(const std::string &caption) { captionView->SetCaption(caption); }

      virtual void Process();

      void SetColor(const Vector3 &color);
      virtual void Redraw();

      virtual void SetToggleable(bool toggleable) { this->toggleable = toggleable; }
      virtual bool IsToggled() { return this->toggled; }
      virtual void SetToggled(bool onOff) { this->toggled = onOff; Redraw(); }
      virtual void SetActive(bool onOff) { this->active = onOff; if (onOff) SetColor(windowManager->GetStyle()->GetColor(e_DecorationType_Bright1)); else SetColor(windowManager->GetStyle()->GetColor(e_DecorationType_Dark2)); Redraw(); }

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      virtual void OnGainFocus();
      virtual void OnLoseFocus();

      boost::signal<void(Gui2Button*)> sig_OnClick;
      boost::signal<void(Gui2Button*)> sig_OnGainFocus;
      boost::signal<void(Gui2Button*)> sig_OnLoseFocus;

    protected:
      Gui2Caption *captionView;

      boost::intrusive_ptr<Image2D> image;

      int fadeOut_ms = 0;
      int fadeOutTime_ms = 0;

      bool toggleable = false;
      bool toggled = false;
      bool active = false;

      Vector3 color;

  };

}

#endif
