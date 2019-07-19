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

#ifndef _HPP_FOOTBALL_GAMETASK
#define _HPP_FOOTBALL_GAMETASK

#include "types/iusertask.hpp"

#include "onthepitch/match.hpp"
#include "menu/menuscene.hpp"

#include "menu/menutask.hpp"

using namespace blunted;

enum e_GameTaskMessage {
  e_GameTaskMessage_StartMatch,
  e_GameTaskMessage_StopMatch,
  e_GameTaskMessage_StartMenuScene,
  e_GameTaskMessage_StopMenuScene,
  e_GameTaskMessage_None
};

class GameTask : public IUserTask {

  public:
    GameTask();
    virtual ~GameTask();

    void Exit();

    void Action(e_GameTaskMessage message);

    virtual void GetPhase();
    virtual void ProcessPhase();
    virtual void PutPhase();

    Match *GetMatch() { return match; }
    MenuScene *GetMenuScene() { return menuScene; }

    MessageQueue<e_GameTaskMessage> messageQueue;

    virtual std::string GetName() const { return "game"; }

  protected:
    Match *match;
    MenuScene *menuScene;
    boost::shared_ptr<Scene3D> scene3D;
};

#endif
