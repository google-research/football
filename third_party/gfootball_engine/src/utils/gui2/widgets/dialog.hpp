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

#ifndef _HPP_GUI2_VIEW_DIALOG
#define _HPP_GUI2_VIEW_DIALOG

#include "../view.hpp"

#include "frame.hpp"

namespace blunted {

  class Gui2Grid;
  class Gui2Button;

  class Gui2Dialog : public Gui2Frame {

    public:
      Gui2Dialog(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Dialog();

      virtual void AddContent(Gui2View *view);

      virtual Gui2Button *AddPosNegButtons(const std::string &posName, const std::string &negName);
      virtual Gui2Button *AddSingleButton(const std::string &caption);

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      //boost::signal<void(Gui2Dialog*)> sig_OnClose;
      boost::signal<void(Gui2Dialog*)> sig_OnPositive;
      boost::signal<void(Gui2Dialog*)> sig_OnNegative;

    protected:
      Gui2Grid *grid;

  };

}

#endif
