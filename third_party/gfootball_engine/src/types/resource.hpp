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

#ifndef _HPP_RESOURCE
#define _HPP_RESOURCE

#include "../defines.hpp"

#include "../managers/resourcemanager.hpp"

#include "../types/refcounted.hpp"

namespace blunted {

  enum e_ResourceType {
    e_ResourceType_GeometryData = 1,
    e_ResourceType_Surface = 2,
    e_ResourceType_Texture = 3,
    e_ResourceType_VertexBuffer = 4,
  };

  template <typename T>
  class Resource : public RefCounted {

    public:
      Resource(std::string identString) : resource(0), identString(identString) {
        resource = new T();
      }

      virtual ~Resource() {
        delete resource;
        resource = 0;
      }

      Resource(const Resource &src, const std::string &identString) : identString(identString) {
        this->resource = new T(*src.resource);
      }



      T *GetResource() {
        return resource;
      }

      std::string GetIdentString() {
        return identString;
      }

      T *resource;

    protected:
      const std::string identString;

  };

}

#endif
