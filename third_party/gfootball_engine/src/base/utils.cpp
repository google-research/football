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
#include "log.hpp"
#include "math/quaternion.hpp"
#include "math/vector3.hpp"

namespace blunted {

  s_treeentry::~s_treeentry() {
    if (subtree) {
      delete subtree;
      subtree = NULL;
    }
  }


  // ----- load .ase file into a tree

  s_tree *tree_load(std::string asefile) {
    asefile = GetGameConfig().updatePath(asefile);
    std::ifstream datafile(asefile.c_str(), std::ios::in);
    if (datafile.fail()) {
      Log(e_FatalError, "tree_load", "", "could not open " + asefile);
      return NULL;
    }

    s_tree *tree = tree_readblock(datafile);

    datafile.close();

    return tree;
  }

  s_tree *tree_readblock(std::ifstream &datafile) {
    s_tree *content = new s_tree();

    bool quit = false;

    while (!datafile.eof() && quit == false) {
      char tmp[2048];
      datafile.getline(tmp, 2048);
      std::string line;
      line.assign(tmp);
      std::vector <std::string> tokens;

      // delete CR character, if it's there
      if (line.length() > 1) {
        if (line[line.length() - 1] == '\r') line = line.substr(0, line.length() - 1);
      }

      line = stringchomp(line, '\t');
      line = stringchomp(line, ' ');
      tokenize(line, tokens, " \t");

      if (tokens.size() > 0) {
        if (tokens.at(0).compare("}") == 0) {
          quit = true;
        } else {
          s_treeentry *entry = new s_treeentry();
          if (tokens.at(0).substr(0, 1).compare("*") == 0) {
            entry->name = tokens.at(0).substr(1);
          } else {
            entry->name = tokens.at(0);
          }
          for (unsigned int i = 1; i < tokens.size(); i++) {
            entry->values.push_back(tokens[i]);
          }

          if (tokens.at(tokens.size() - 1).compare("{") == 0) { // iterate
            entry->values.pop_back();
            entry->subtree = tree_readblock(datafile);
          }
          content->entries.push_back(entry);
        }
      }
    }

    return content;
  }


  // tree structure utility functions

  const s_treeentry *treeentry_find(const s_tree *tree, const std::string needle) {
    assert(tree);

    for (unsigned int i = 0; i < tree->entries.size(); i++) {
      assert(tree->entries[i]);
      if (tree->entries[i]->name.compare(needle) == 0) return tree->entries[i];
    }
    return NULL;
  }

  const s_tree *tree_find(const s_tree *tree, const std::string needle) {
    //assert(tree);

    for (unsigned int i = 0; i < tree->entries.size(); i++) {
      assert(tree->entries[i]);
      if (tree->entries[i]->name.compare(needle) == 0) {
        assert(tree->entries[i]->subtree);
        return tree->entries[i]->subtree;
      }
    }
    return NULL;
  }


  // string functions

  std::string stringchomp(std::string input, char chomp) {
    if (input.find_first_not_of(chomp) < input.length()) return (input.substr(input.find_first_not_of(chomp)));
    return "";
  }

  // tokenizer code from oopweb.com
  void tokenize(const std::string& str, std::vector<std::string> &tokens, const std::string &delimiters) {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
  }

  std::string file_to_string(std::string filename) {
    filename = GetGameConfig().updatePath(filename);

    char line[1024];
    std::ifstream file;

    file.open(filename.c_str(), std::ios::in);

    if (file.fail()) Log(e_FatalError, "utils", "file_to_vector", "file not found or empty: " + filename);

    std::string source;
    while (file.getline(line, 1024)) {
      source.append(line);
    }

    // remove possible windows CR
    source.erase( std::remove(source.begin(), source.end(), '\r'), source.end() );

    file.close();

    return source;
  }

  void file_to_vector(std::string filename, std::vector<std::string> &destination) {

    char line[32767];
    std::ifstream file;
    filename = GetGameConfig().updatePath(filename);
    file.open(filename.c_str(), std::ios::in);

    if (file.fail()) Log(e_FatalError, "utils", "file_to_vector", "file not found or empty: " + filename);

    while (file.getline(line, 32767)) {
      std::string line_str;
      line_str.assign(line);
      // remove possible windows CR
      line_str.erase( std::remove(line_str.begin(), line_str.end(), '\r'), line_str.end() );
      destination.push_back(line_str);
    }

    file.close();
  }

  std::string get_file_name(const std::string &filename) {
    std::string chompedFilename = filename.substr(filename.find_last_of("\\") + 1);
    chompedFilename = chompedFilename.substr(filename.find_last_of("/") + 1);
    return chompedFilename;
  }

  std::string get_file_extension(const std::string &filename) {
    return filename.substr(filename.find_last_of(".") + 1);
  }

  std::string int_to_str(int i) {
    std::string i_str;
    char i_c[16];
    snprintf(i_c, 16, "%i", i);
    i_str.assign(i_c);
    return i_str;
  }

  std::string real_to_str(real r) {
    std::string r_str;
    char r_c[32];
    snprintf(r_c, 32, "%f", r);
    r_str.assign(r_c);
    return r_str;
  }

  std::string GetStringFromVector(const Vector3 &vec) {
    std::string tmp;
    tmp = "";
    char tmpC[1000];
    sprintf(tmpC, "%f, %f, %f", vec.coords[0], vec.coords[1], vec.coords[2]);
    tmp.assign(tmpC);
    return tmp;
  }

  Vector3 GetVectorFromString(const std::string &vecString) {
    if (vecString.compare("") == 0) {
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
    if (tokenizedString.size() > 1) vector.coords[1] = atof(tokenizedString.at(1).c_str());
    if (tokenizedString.size() > 2) vector.coords[2] = atof(tokenizedString.at(2).c_str());
    return vector;
  }

  Quaternion GetQuaternionFromString(const std::string &quatString) {
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
