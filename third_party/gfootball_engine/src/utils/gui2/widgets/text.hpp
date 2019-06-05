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

#ifndef _HPP_GUI2_VIEW_TEXT
#define _HPP_GUI2_VIEW_TEXT

#include "../view.hpp"

#include "caption.hpp"

#include "../../../scene/objects/image2d.hpp"

namespace blunted {

  class Gui2Text : public Gui2View {

    public:
      Gui2Text(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, float fontsize_percent, unsigned int maxHorizChars, const std::string &text);
      virtual ~Gui2Text();

      void AddEmptyLine();
      void AddText(const std::string &newText);

     protected:
      boost::intrusive_ptr<Image2D> image;
      float fontsize_percent = 0.0f;
      unsigned int maxHorizChars = 0;
      std::string text;
      std::vector<std::string> resultText;
      std::vector<Gui2Caption*> resultCaptions;
      Vector3 color;
      Vector3 outlineColor;

  };

}

#endif
