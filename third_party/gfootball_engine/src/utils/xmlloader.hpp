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

#ifndef _HPP_UTILS_XMLLOADER
#define _HPP_UTILS_XMLLOADER

#include "../defines.hpp"

namespace blunted {

  struct XMLTree;

  typedef std::multimap<std::string, XMLTree> map_XMLTree;

  struct XMLTree {
    std::string value;
    map_XMLTree children;
  };

  class XMLLoader {

    public:
      XMLLoader();
      ~XMLLoader();

      XMLTree LoadFile(const std::string &filename);
      XMLTree Load(const std::string &file);

     protected:
      void BuildTree(XMLTree &tree, const std::string &source);

  };

}

#endif
