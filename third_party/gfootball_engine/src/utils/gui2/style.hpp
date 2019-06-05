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

#ifndef _HPP_GUI2_STYLE
#define _HPP_GUI2_STYLE

#include "wrap_SDL_ttf.h"
#include "../../base/math/vector3.hpp"

namespace blunted {

  enum e_TextType {
    e_TextType_Default,
    e_TextType_DefaultOutline,
    e_TextType_Caption,
    e_TextType_Title,
    e_TextType_ToolTip
  };

  enum e_DecorationType {
    e_DecorationType_Dark1,
    e_DecorationType_Dark2,
    e_DecorationType_Bright1,
    e_DecorationType_Bright2,
    e_DecorationType_Toggled
  };

  class Gui2Style {

    public:
      Gui2Style();
      virtual ~Gui2Style();

      void SetFont(e_TextType textType, TTF_Font *font);
      void SetColor(e_DecorationType decorationType, const Vector3 &color);

      TTF_Font *GetFont(e_TextType textType) const;
      Vector3 GetColor(e_DecorationType decorationType) const;

     protected:
      std::map <e_TextType, TTF_Font*> fonts;
      std::map <e_DecorationType, Vector3> colors;

  };

}

#endif
