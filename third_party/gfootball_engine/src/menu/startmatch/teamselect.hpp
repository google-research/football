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

#ifndef _HPP_MENU_TEAMSELECT
#define _HPP_MENU_TEAMSELECT

#include "../../utils/gui2/windowmanager.hpp"

#include "../../utils/gui2/widgets/menu.hpp"
#include "../../utils/gui2/widgets/root.hpp"
#include "../../utils/gui2/widgets/grid.hpp"
#include "../../utils/gui2/widgets/button.hpp"
#include "../../utils/gui2/widgets/image.hpp"
#include "../../utils/gui2/widgets/slider.hpp"
#include "../../utils/gui2/widgets/iconselector.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

class TeamSelectPage : public Gui2Page {

  public:
    TeamSelectPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~TeamSelectPage();

    void FocusTeamSelect1();
    void FocusStart1();
    void FocusCompetitionSelect2();
    void FocusTeamSelect2();
    void FocusStart2();
    void SetupTeamSelect1();
    void SetupTeamSelect2();
    void GoOptionsMenu();

    virtual void ProcessWindowingEvent(WindowingEvent *event);

    Gui2Button *buttonStart1;
    Gui2Button *buttonStart2;
    Gui2Caption *p2;
    Gui2Grid *grid2;
    Gui2Image *bg2;

  protected:
    Gui2IconSelector *teamSelect1;
    Gui2IconSelector *teamSelect2;
    Gui2IconSelector *competitionSelect1;
    Gui2IconSelector *competitionSelect2;

};

#endif
