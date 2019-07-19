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

#ifndef _HPP_FOOTBALL_SYNCHRONIZATIONTASK
#define _HPP_FOOTBALL_SYNCHRONIZATIONTASK

#include "types/iusertask.hpp"

#include "onthepitch/match.hpp"
#include "menu/menuscene.hpp"

#include "menu/menutask.hpp"


class SynchronizationTask : public IUserTask {

  public:
    SynchronizationTask();
    virtual ~SynchronizationTask();

    virtual void GetPhase();
    virtual void ProcessPhase();
    virtual void PutPhase();

    virtual std::string GetName() const { return "synchronization"; }

    bool inPlay();
    void Step(int steps = 1);
    int Steps();

  protected:
    int steps_ = 0;
};

#endif
