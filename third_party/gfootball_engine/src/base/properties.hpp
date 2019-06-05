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

#ifndef _HPP_BASE_PROPERTIES
#define _HPP_BASE_PROPERTIES

#include "../defines.hpp"

#include "../base/math/vector3.hpp"

namespace blunted {

  typedef std::map <std::string, std::string> map_Properties;

  class Vector3;

  class Properties {

    public:
      Properties();
      virtual ~Properties();

      bool Exists(const char *name) const;

      void Set(const char *name, const std::string &value);
      void SetInt(const char *name, int value);
      void Set(const char *name, real value);
      void SetBool(const char *name, bool value);
      const std::string &Get(const char *name, const std::string &defaultValue = emptyString) const;
      bool GetBool(const char *name, bool defaultValue = false) const;
      real GetReal(const char *name, real defaultValue = 0) const;
      int GetInt(const char *name, int defaultValue = 0) const;
      void AddProperties(const Properties *userprops);
      void AddProperties(const Properties &userprops);
      const map_Properties *GetProperties() const;

      void LoadFile(const std::string &filename);
      void SaveFile(const std::string &filename) const;

     protected:
      map_Properties properties;

      static std::string emptyString;
  };

}

#endif
