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

#include "style.hpp"

namespace blunted {

  Gui2Style::Gui2Style() {
  }

  Gui2Style::~Gui2Style() {
  }

  void Gui2Style::SetFont(e_TextType textType, TTF_Font *font) {
    fonts.insert(std::pair<e_TextType, TTF_Font*>(textType, font));
  }

  void Gui2Style::SetColor(e_DecorationType decorationType, const Vector3 &color) {
    colors.insert(std::pair<e_DecorationType, Vector3>(decorationType, color));
  }

  TTF_Font *Gui2Style::GetFont(e_TextType textType) const {
    std::map<e_TextType, TTF_Font*>::const_iterator iter = fonts.find(textType);
    assert(iter != fonts.end());
    return iter->second;
  }

  Vector3 Gui2Style::GetColor(e_DecorationType decorationType) const {
    std::map<e_DecorationType, Vector3>::const_iterator iter = colors.find(decorationType);
    if (iter == colors.end()) {
      return Vector3(0);
    } else {
      return iter->second;
    }
  }

}
