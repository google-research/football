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

#ifndef _HPP_GUI2_VIEW_EDITLINE
#define _HPP_GUI2_VIEW_EDITLINE

#include "../view.hpp"
#include "caption.hpp"
#include "image.hpp"

namespace blunted {

  class Gui2EditLine : public Gui2View {

    public:
      Gui2EditLine(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &defaultText);
      virtual ~Gui2EditLine();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      void SetText(const std::string &newText);
      std::string GetText() { return currentText; }
      void SetMaxLength(int length) { maxLength = length; }
      void SetAllowedChars(const std::string &chars) { allowedChars = chars; }

      virtual void Show() { caption->Show(); Gui2View::Show(); } // ignore cursor

      virtual void OnGainFocus();
      virtual void OnLoseFocus();
      virtual void ProcessKeyboardEvent(KeyboardEvent *event);

      boost::signal<void(Gui2EditLine*)> sig_OnEnter;
      boost::signal<void(Gui2EditLine*)> sig_OnGainFocus;
      boost::signal<void(Gui2EditLine*)> sig_OnLoseFocus;
      boost::signal<void(Gui2EditLine*)> sig_OnChange;

    protected:
      Gui2Caption *caption;
      Gui2Image *cursor;

      std::string currentText;
      std::string allowedChars;
      unsigned int maxLength = 0;
      Vector3 color;
      Vector3 outlineColor;

      int cursorPos = 0;

  };

}

#endif
