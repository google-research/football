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

#include "xmlloader.hpp"

#include "../base/utils.hpp"
#include "../base/log.hpp"

namespace blunted {

  XMLLoader::XMLLoader() {
  }

  XMLLoader::~XMLLoader() {
  }


  XMLTree XMLLoader::LoadFile(const std::string &filename) {
    std::string source;
    source = file_to_string(filename);

    XMLTree tree;
    BuildTree(tree, source);

    return tree;
  }

  XMLTree XMLLoader::Load(const std::string &file) {
    XMLTree tree;
    BuildTree(tree, file);

    return tree;
  }

  void XMLLoader::BuildTree(XMLTree &tree, const std::string &source) {

    size_t index_end = 0;
    size_t index = source.find("<", 0);

    if (index == std::string::npos) {
      // no tags, must be a value
      tree.value = source;
      tree.value.erase(remove_if(tree.value.begin(), tree.value.end(), isspace), tree.value.end());
      //printf("value: '%s'\n", source.c_str());
      return;
    }


    // a tag (or multiple), so must contain children

    while (index != std::string::npos) {
      index_end = source.find(">", index);
      std::string tag = source.substr(index + 1, index_end - index - 1);
      //printf("tag: '%s'\n", tag.c_str());
      index = index_end;
      // index is now directly behind opening tag

      // find closing tag
      int recurse_counter = 1;
      size_t index_nexttag_open = 0;
      size_t index_nexttag_close = 0;
      while (recurse_counter != 0) {
        index_nexttag_open = source.find("<" + tag + ">", index_end + 1);
        index_nexttag_close = source.find("</" + tag + ">", index_end + 1);
        //printf("%i %i\n", index_nexttag_open, index_nexttag_close);
        if (index_nexttag_open > index_nexttag_close || index_nexttag_open == std::string::npos) {
          recurse_counter--;
          index_end = index_nexttag_close;
        } else {
          recurse_counter++;
          index_end = index_nexttag_open;
        }
        //printf("%i\n", recurse_counter);
        if (index_end == std::string::npos) {
          Log(e_FatalError, "XMLLoader", "BuildTree", "No closing tag found for <" + tag + ">");
        }
      }

      std::string data = source.substr(index + 1, index_end - index - 1);
      //printf("data: '%s'\n", data.c_str());

      XMLTree child;
      BuildTree(child, data);
      tree.children.insert(std::make_pair(tag, child));

      // close
      index = source.find(">", index_end);

      // find next tag
      index = source.find("<", index);
    }
  }

}
