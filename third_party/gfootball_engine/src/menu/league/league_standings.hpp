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

#ifndef _HPP_MENU_LEAGUE_STANDINGS
#define _HPP_MENU_LEAGUE_STANDINGS

#include "../../utils/gui2/windowmanager.hpp"

#include "../../utils/gui2/page.hpp"
#include "../../utils/gui2/widgets/root.hpp"
#include "../../utils/gui2/widgets/image.hpp"
#include "../../utils/gui2/widgets/button.hpp"
#include "../../utils/gui2/widgets/slider.hpp"
#include "../../utils/gui2/widgets/grid.hpp"
#include "../../utils/gui2/widgets/caption.hpp"
#include "../../utils/gui2/widgets/capturekey.hpp"

using namespace blunted;

class LeagueStandingsPage : public Gui2Page {

  public:
    LeagueStandingsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsPage();

  protected:

};

class LeagueStandingsLeaguePage : public Gui2Page {

  public:
    LeagueStandingsLeaguePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsLeaguePage();

  protected:

};

class LeagueStandingsLeagueTablePage : public Gui2Page {

  public:
    LeagueStandingsLeagueTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsLeagueTablePage();

  protected:

};

class LeagueStandingsLeagueStatsPage : public Gui2Page {

  public:
    LeagueStandingsLeagueStatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsLeagueStatsPage();

  protected:

};

class LeagueStandingsNCupPage : public Gui2Page {

  public:
    LeagueStandingsNCupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsNCupPage();

  protected:

};

class LeagueStandingsNCupTreePage : public Gui2Page {

  public:
    LeagueStandingsNCupTreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsNCupTreePage();

  protected:

};

class LeagueStandingsNCupStatsPage : public Gui2Page {

  public:
    LeagueStandingsNCupStatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsNCupStatsPage();

  protected:

};

class LeagueStandingsICup1Page : public Gui2Page {

  public:
    LeagueStandingsICup1Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup1Page();

  protected:

};

class LeagueStandingsICup1GroupTablePage : public Gui2Page {

  public:
    LeagueStandingsICup1GroupTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup1GroupTablePage();

  protected:

};

class LeagueStandingsICup1TreePage : public Gui2Page {

  public:
    LeagueStandingsICup1TreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup1TreePage();

  protected:

};

class LeagueStandingsICup1StatsPage : public Gui2Page {

  public:
    LeagueStandingsICup1StatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup1StatsPage();

  protected:

};

class LeagueStandingsICup2Page : public Gui2Page {

  public:
    LeagueStandingsICup2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup2Page();

  protected:

};

class LeagueStandingsICup2GroupTablePage : public Gui2Page {

  public:
    LeagueStandingsICup2GroupTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup2GroupTablePage();

  protected:

};

class LeagueStandingsICup2TreePage : public Gui2Page {

  public:
    LeagueStandingsICup2TreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup2TreePage();

  protected:

};

class LeagueStandingsICup2StatsPage : public Gui2Page {

  public:
    LeagueStandingsICup2StatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStandingsICup2StatsPage();

  protected:

};

#endif
