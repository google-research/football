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

#include "directoryparser.hpp"

#include "../base/log.hpp"
#include "../main.hpp"

namespace fs = boost::filesystem;

namespace blunted {

  DirectoryParser::DirectoryParser() {
  }

  DirectoryParser::~DirectoryParser() {
  }

  void DirectoryParser::Parse(boost::filesystem::path path, const std::string &extension, std::vector<std::string> &files, bool recurse) {
    path = GetGameConfig().updatePath(path.string());
    if (!fs::exists(path) || !fs::is_directory(path)) Log(e_Error, "DirectoryParser", "Parse", "Could not open directory " + path.string() + " for reading");

    fs::directory_iterator dirIter(path);
    fs::directory_iterator endIter;
    while (dirIter != endIter) {
      if (is_directory(dirIter->status())) {

        if (recurse) {
          boost::filesystem::path thePath(path);
          thePath /= dirIter->path().filename();
          Parse(thePath, extension, files);
        }

      } else {
        boost::filesystem::path thePath(path);
        thePath /= dirIter->path().filename();

        if (thePath.extension() == "." + extension) {

          // add to results
          //printf("adding %s\n", dirIter->path().filename().c_str());
          files.push_back(path.string() + "/" + thePath.filename().string());

        }
      }

      dirIter++;
    }
  }
}
