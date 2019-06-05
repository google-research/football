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

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_PLANMAP
#define _HPP_GUI2_VIEW_PLANMAP

#include "../../utils/gui2/view.hpp"
#include "../../utils/gui2/widgets/caption.hpp"
#include "../../scene/objects/image2d.hpp"

class TeamData;
class Match;

namespace blunted {

  class Gui2PlanMap : public Gui2View {

    public:
      Gui2PlanMap(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, TeamData *teamData);
      virtual ~Gui2PlanMap();

      virtual void Process();

    protected:
      boost::intrusive_ptr<Image2D> image;
  };

  class Gui2PlanMapEntry : public Gui2View {

    public:
      Gui2PlanMapEntry(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &roleName, const std::string &playerName);
      virtual ~Gui2PlanMapEntry();

     protected:
      Gui2Caption *roleNameCaption;  // formerly: captionView
      Gui2Caption *playerNameCaption;

  };

}

#endif
