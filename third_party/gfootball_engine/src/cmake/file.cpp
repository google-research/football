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

#include "file.h"

#include "../base/log.hpp"
#include "../main.hpp"

namespace fs = boost::filesystem;

std::string GetFile(const std::string &fileName) {
  DO_VALIDATION;
  std::ifstream file;
  file.open(fileName.c_str(), std::ios::in);
  std::string str((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());
  file.close();
  return str;
}

void GetFilesRec(boost::filesystem::path path, const std::string &extension,
                 std::vector<std::string> &files) {
  DO_VALIDATION;
  if (!fs::exists(path) || !fs::is_directory(path)) {
    DO_VALIDATION;
    Log(e_Error, "DirectoryParser", "Parse",
        "Could not open directory " + path.string() + " for reading");
  }
  fs::directory_iterator dirIter(path);
  fs::directory_iterator endIter;
  while (dirIter != endIter) {
    DO_VALIDATION;
    if (is_directory(dirIter->status())) {
      DO_VALIDATION;
      boost::filesystem::path thePath(path);
      thePath /= dirIter->path().filename();
      GetFilesRec(thePath, extension, files);
    } else {
      boost::filesystem::path thePath(path);
      thePath /= dirIter->path().filename();

      if (thePath.extension() == "." + extension) {
        DO_VALIDATION;
        files.push_back(path.string() + "/" + thePath.filename().string());
      }
    }

    dirIter++;
  }
}

void GetFiles(std::string path, const std::string &extension,
              std::vector<std::string> &files) {
  DO_VALIDATION;
  GetFilesRec(GetGameConfig().updatePath(path), extension, files);
}
