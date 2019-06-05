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

#ifndef _HPP_MENU_LEAGUE
#define _HPP_MENU_LEAGUE

#include "../../utils/gui2/windowmanager.hpp"

#include "../../utils/gui2/page.hpp"
#include "../../utils/gui2/widgets/grid.hpp"
#include "../../utils/gui2/widgets/editline.hpp"
#include "../../utils/gui2/widgets/dialog.hpp"
#include "../../utils/gui2/widgets/filebrowser.hpp"
#include "../../utils/gui2/widgets/button.hpp"
#include "../../utils/gui2/widgets/pulldown.hpp"
#include "../../utils/gui2/widgets/slider.hpp"

using namespace blunted;

class LeaguePage : public Gui2Page {

  public:
    LeaguePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeaguePage();

  protected:
    void StepTime();
    void SetTimeCaption();

    Gui2Caption *captionTime;

};

class LeagueStartPage : public Gui2Page {

  public:
    LeagueStartPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartPage();

  protected:
    void GoLoad();
    void GoNew();

};

class LeagueStartLoadPage : public Gui2Page {

  public:
    LeagueStartLoadPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartLoadPage();

  protected:
    void GoLoadSave();

    Gui2FileBrowser *browser;

};

class LeagueStartNewPage : public Gui2Page {

  public:
    LeagueStartNewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartNewPage();

  protected:
    void GoDatabaseSelectDialog();
    void CloseDatabaseSelectDialog();
    void GoProceed();
    void CloseCreateSaveDialog();

    Gui2Button *databaseSelectButton;
    Gui2Pulldown *currencySelectPulldown;
    Gui2Slider *difficultySlider;
    Gui2EditLine *saveNameInput;
    Gui2EditLine *managerNameInput;

    Gui2Dialog *databaseSelectDialog;
    Gui2FileBrowser *databaseSelectBrowser;
    Gui2View *previousFocus;

    std::string data_SelectedDatabase;

    Gui2Dialog *createSaveDialog;

    bool success = false;

};

#endif
