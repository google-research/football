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

#ifndef _HPP_GUI2_VIEW_SCOREBOARD
#define _HPP_GUI2_VIEW_SCOREBOARD

#include "../../utils/gui2/view.hpp"

#include "../../scene/objects/image2d.hpp"
#include "../../utils/gui2/widgets/caption.hpp"
#include "../../utils/gui2/widgets/image.hpp"

using namespace blunted;

class Match;

class Gui2ScoreBoard : public Gui2View {

  public:
    Gui2ScoreBoard(Gui2WindowManager *windowManager, Match *match);
    virtual ~Gui2ScoreBoard();

    void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

    virtual void Redraw();

    void SetTimeStr(const std::string &timeStr);
    void SetGoalCount(int teamID, int goalCount);

  protected:
    boost::intrusive_ptr<Image2D> image;

    float content_yOffset = 0.0f;

    std::string timeStr;
    int goalCount[2];

    Gui2Caption *timeCaption;
    Gui2Caption *teamNameCaption[2];
    Gui2Caption *goalCountCaption[2];

    Gui2Image *leagueLogo;
    Gui2Image *teamLogo[2];
};

#endif
