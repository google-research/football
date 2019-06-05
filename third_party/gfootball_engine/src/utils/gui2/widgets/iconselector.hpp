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

#ifndef _HPP_GUI2_VIEW_ICONSELECTOR
#define _HPP_GUI2_VIEW_ICONSELECTOR

#include "wrap_SDL_ttf.h"

#include "../view.hpp"

#include "../../../scene/objects/image2d.hpp"

#include "image.hpp"
#include "caption.hpp"

namespace blunted {

  struct Gui2IconSelectorEntry {
    std::string caption;
    std::string id;
    Gui2Image *icon;
  };

  class Gui2IconSelector : public Gui2View {

    public:
      Gui2IconSelector(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2IconSelector();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Process();
      virtual void Redraw();

      std::string GetSelectedEntryID() { if (entries.size() > 0) return entries.at(selectedEntry).id; else return ""; }
      void ClearEntries();
      void AddEntry(const std::string &id, const std::string &caption, const std::string &imageFile);

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      virtual void OnGainFocus();
      virtual void OnLoseFocus();

      boost::signal<void()> sig_OnClick;
      boost::signal<void()> sig_OnChange;

    protected:
      boost::intrusive_ptr<Image2D> image;

      std::string caption;

      std::vector<Gui2IconSelectorEntry> entries;
      Gui2Caption *selectedCaption;

      int selectedEntry = 0;
      float visibleSelectedEntry = 0.0f;

      int fadeOut_ms = 0;
      int fadeOutTime_ms = 0;

  };

}

#endif
