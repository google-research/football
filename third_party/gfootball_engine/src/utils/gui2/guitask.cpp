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

#include "guitask.hpp"
#include "../../main.hpp"

namespace blunted {

Gui2Task::Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio,
                   float margin) {
  DO_VALIDATION;
  windowManager = new Gui2WindowManager(scene2D, aspectRatio, margin);
}

Gui2Task::~Gui2Task() {
  DO_VALIDATION;
  windowManager->Exit();
  delete windowManager;
}
}
