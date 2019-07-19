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

#include "properties.hpp"

#include "../defines.hpp"
#include "utils.hpp"
#include "log.hpp"

#include <fstream>
#include <iostream>

using namespace std;

namespace blunted {

  std::string Properties::emptyString = "";

  Properties::Properties() {
  }

  Properties::~Properties() {
  }

  bool Properties::Exists(const std::string &name) const {
    auto iter = properties.find(name);
    if (iter == properties.end()) {
      return false;
    } else {
      return true;
    }
  }

  void Properties::Set(const std::string &name, const std::string &value) {
    properties[name] = value;
  }

  void Properties::SetInt(const std::string &name, int value) {
    std::string value_str = int_to_str(value);
    Set(name, value_str);
  }

  void Properties::Set(const std::string &name, real value) {
    std::string value_str = real_to_str(value);
    Set(name, value_str);
  }

  void Properties::SetBool(const std::string &name, bool value) {
    std::string value_str = value ? "true" : "false";
    Set(name, value_str);
  }

  const std::string &Properties::Get(const std::string &name, const std::string &defaultValue) const {
    auto iter = properties.find(name);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return iter->second;
    }
  }

  bool Properties::GetBool(const std::string &name, bool defaultValue) const {
    auto iter = properties.find(name);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      if (iter->second.compare("true") == 0) return true; else return false;
    }
  }

  real Properties::GetReal(const std::string& name, real defaultValue) const {
    auto iter = properties.find(name);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return atof(iter->second.c_str());
    }
  }

  int Properties::GetInt(const std::string &name, int defaultValue) const {
    auto iter = properties.find(name);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return int(floor(atof(iter->second.c_str())));
    }
  }

  void Properties::AddProperties(const Properties *userprops) {
    if (!userprops) return;

    const map_Properties *userpropdata = userprops->GetProperties();
    map_Properties::const_iterator iter = userpropdata->begin();

    while (iter != userpropdata->end()) {
      properties[iter->first] = iter->second;
      iter++;
    }
  }

  void Properties::AddProperties(const Properties &userprops) {
    const map_Properties *userpropdata = userprops.GetProperties();
    map_Properties::const_iterator iter = userpropdata->begin();

    while (iter != userpropdata->end()) {
      properties[iter->first] = iter->second;
      iter++;
    }
  }

  const map_Properties *Properties::GetProperties() const {
    return &properties;
  }

  }  // namespace blunted
