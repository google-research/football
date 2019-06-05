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

#ifndef _HPP_GUI2_VIEW_MENU
#define _HPP_GUI2_VIEW_MENU

#include "frame.hpp"
#include "../../../base/properties.hpp"

namespace blunted {

  class Gui2Menu : public Gui2Frame {

    public:
      Gui2Menu(Gui2WindowManager *windowManager, Gui2Menu *parentMenu, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Menu();

      virtual void Process();

      void DecoupleParent(); // for unlinking parent menu
      void _AddSubmenu(Gui2Menu *view);
      void _RemoveSubmenu(Gui2Menu *view);

      virtual void Deactivate();
      virtual void Reactivate();

      virtual void ProcessWindowingEvent(WindowingEvent *event);

    protected:
      Gui2Menu *parentMenu;
      Gui2View *lastFocus;

      std::vector<Gui2Menu*> subMenus;

  };

}

#endif
