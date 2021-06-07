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

#include "utils.hpp"

#include "../main.hpp"
#include "file.h"
#include "log.hpp"
#include "math/quaternion.hpp"
#include "math/vector3.hpp"

namespace blunted {

s_treeentry::~s_treeentry() {
  DO_VALIDATION;
  if (subtree) {
    DO_VALIDATION;
    delete subtree;
    subtree = NULL;
  }
}

  // ----- load .ase file into a tree

s_tree *tree_load(std::string asefile) {
  DO_VALIDATION;
  asefile = GetGameConfig().updatePath(asefile);
  const std::string &datafile = GetFile(asefile);
  const char *data = datafile.c_str();
  int len = datafile.size();
  s_tree *tree = tree_readblock(data, len);
  return tree;
}

s_tree *tree_readblock(const char *&datafile, int &len) {
  DO_VALIDATION;
  s_tree *content = new s_tree();

  bool quit = false;

  while (len > 0 && quit == false) {
    DO_VALIDATION;
    char tmp[2048];
    int l = 0;
    for (l = 0; datafile[0] != '\n'; l++) {
      DO_VALIDATION;
      tmp[l] = datafile[0];
      datafile++;
      len--;
    }
    tmp[l] = 0;
    datafile++;
    len--;
    std::string line;
    line.assign(tmp);
    std::vector<std::string> tokens;

    // delete CR character, if it's there
    if (line.length() > 1) {
      DO_VALIDATION;
      if (line[line.length() - 1] == '\r')
        line = line.substr(0, line.length() - 1);
    }

    line = stringchomp(line, '\t');
    line = stringchomp(line, ' ');
    tokenize(line, tokens, " \t");

    if (tokens.size() > 0) {
      DO_VALIDATION;
      if (tokens.at(0).compare("}") == 0) {
        DO_VALIDATION;
        quit = true;
      } else {
        s_treeentry *entry = new s_treeentry();
        if (tokens.at(0).substr(0, 1).compare("*") == 0) {
          DO_VALIDATION;
          entry->name = tokens.at(0).substr(1);
        } else {
          entry->name = tokens.at(0);
        }
        for (unsigned int i = 1; i < tokens.size(); i++) {
          DO_VALIDATION;
          entry->values.push_back(tokens[i]);
        }

        if (tokens.at(tokens.size() - 1).compare("{") == 0) {
          DO_VALIDATION;  // iterate
          entry->values.pop_back();
          entry->subtree = tree_readblock(datafile, len);
        }
        content->entries.push_back(entry);
      }
    }
  }

  return content;
}

  // tree structure utility functions

const s_treeentry *treeentry_find(const s_tree *tree,
                                  const std::string needle) {
  DO_VALIDATION;
  assert(tree);

  for (unsigned int i = 0; i < tree->entries.size(); i++) {
    DO_VALIDATION;
    assert(tree->entries[i]);
    if (tree->entries[i]->name.compare(needle) == 0) return tree->entries[i];
  }
  return NULL;
}

const s_tree *tree_find(const s_tree *tree, const std::string needle) {
  DO_VALIDATION;
  // assert(tree);

  for (unsigned int i = 0; i < tree->entries.size(); i++) {
    DO_VALIDATION;
    assert(tree->entries[i]);
    if (tree->entries[i]->name.compare(needle) == 0) {
      DO_VALIDATION;
      assert(tree->entries[i]->subtree);
      return tree->entries[i]->subtree;
    }
  }
  return NULL;
}

  // string functions

std::string stringchomp(std::string input, char chomp) {
  DO_VALIDATION;
  if (input.find_first_not_of(chomp) < input.length())
    return (input.substr(input.find_first_not_of(chomp)));
  return "";
}

  // tokenizer code from oopweb.com
void tokenize(const std::string &str, std::vector<std::string> &tokens,
              const std::string &delimiters) {
  DO_VALIDATION;
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos) {
    DO_VALIDATION;
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

std::string file_to_string(std::string filename) {
  DO_VALIDATION;
  return GetFile(GetGameConfig().updatePath(filename));
}

void file_to_vector(std::string filename,
                    std::vector<std::string> &destination) {
  DO_VALIDATION;
  std::string file = GetFile(GetGameConfig().updatePath(filename));
  int last_pos = 0;
  for (int x = 0; x < file.length(); x++) {
    DO_VALIDATION;
    if (file[x] == '\n') {
      DO_VALIDATION;
      destination.push_back(file.substr(last_pos, x - last_pos));
      last_pos = x + 1;
    }
  }
  if (last_pos < file.length()) {
    DO_VALIDATION;
    destination.push_back(file.substr(last_pos, file.length() - last_pos));
  }
}

std::string get_file_name(const std::string &filename) {
  DO_VALIDATION;
#ifdef WIN32
  std::string chompedFilename =
      boost::filesystem::path(filename).filename().string();
#else
  std::string chompedFilename =
      filename.substr(filename.find_last_of('\\') + 1);
  chompedFilename = chompedFilename.substr(filename.find_last_of('/') + 1);
#endif
  return chompedFilename;
}

std::string get_file_extension(const std::string &filename) {
  DO_VALIDATION;
  return filename.substr(filename.find_last_of('.') + 1);
}

std::string int_to_str(int i) {
  DO_VALIDATION;
  std::string i_str;
  char i_c[16];
  snprintf(i_c, 16, "%i", i);
  i_str.assign(i_c);
  return i_str;
}

std::string real_to_str(real r) {
  DO_VALIDATION;
  std::string r_str;
  char r_c[32];
  snprintf(r_c, 32, "%f", r);
  r_str.assign(r_c);
  return r_str;
}

std::string GetStringFromVector(const Vector3 &vec) {
  DO_VALIDATION;
  std::string tmp;
  tmp = "";
  char tmpC[1000];
  sprintf(tmpC, "%f, %f, %f", vec.coords[0], vec.coords[1], vec.coords[2]);
  tmp.assign(tmpC);
  return tmp;
}

Vector3 GetVectorFromString(const std::string &vecString) {
  DO_VALIDATION;
  if (vecString.compare("") == 0) {
    DO_VALIDATION;
    printf("vectorfromstring warning, no value\n");
    return Vector3(0.0f);
  }
  std::vector<std::string> tokenizedString;
  std::string delimiter = ",";
  tokenize(vecString, tokenizedString, delimiter);
  assert(tokenizedString.size() > 0);
  assert(tokenizedString.size() <= 3);
  Vector3 vector;
  vector.coords[0] = atof(tokenizedString.at(0).c_str());
  if (tokenizedString.size() > 1)
    vector.coords[1] = atof(tokenizedString.at(1).c_str());
  if (tokenizedString.size() > 2)
    vector.coords[2] = atof(tokenizedString.at(2).c_str());
  return vector;
}

Quaternion GetQuaternionFromString(const std::string &quatString) {
  DO_VALIDATION;
  std::vector<std::string> tokenizedString;
  std::string delimiter = ",";
  tokenize(quatString, tokenizedString, delimiter);
  assert(tokenizedString.size() == 4);
  radian angle;
  Vector3 vector;
  angle = atof(tokenizedString.at(0).c_str()) / 360.0 * 2.0 * pi;
  vector.coords[0] = atof(tokenizedString.at(1).c_str());
  vector.coords[1] = atof(tokenizedString.at(2).c_str());
  vector.coords[2] = atof(tokenizedString.at(3).c_str());
  Quaternion quaternion;
  quaternion.SetAngleAxis(angle, vector);
  return quaternion;
}

  }  // namespace blunted
