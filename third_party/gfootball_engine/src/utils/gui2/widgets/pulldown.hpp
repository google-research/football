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

#ifndef _HPP_GUI2_VIEW_PULLDOWN
#define _HPP_GUI2_VIEW_PULLDOWN

#include "../view.hpp"

#include "grid.hpp"
#include "button.hpp"
#include "image.hpp"

namespace blunted {

  struct PulldownEntry {
    std::string name;
    Gui2Button *button;
  };

  class Gui2Pulldown : public Gui2View {

    public:
      Gui2Pulldown(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Pulldown();

      void AddEntry(const std::string &caption, const std::string &name);
      void PullDownOrUp();
      void SetSelected(int selectedEntry);
      std::string GetSelected() const;

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      boost::signal<void(Gui2Pulldown*)> sig_OnChange;

    protected:
      void Select(int selectedEntry);

      Gui2Button *pulldownButton;

      std::vector<PulldownEntry> entries;
      int selectedEntry = 0;

      Gui2Image *bg;
      Gui2Grid *grid;

      bool pulledDown = false;

  };

}

#endif
