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

#ifndef _HPP_TYPES_SINGLETON
#define _HPP_TYPES_SINGLETON

#include "../defines.hpp"

namespace blunted {

  template <typename T> class Singleton {

    public:
      Singleton(void) {
        assert(!singleton);
        singleton = static_cast<T*>(this);
      }

      virtual ~Singleton(void) {
        assert(singleton); // isn't this a paradox? :D
        singleton = 0;
      }

      static T& GetInstance(void) {
        assert(singleton);
        return(*singleton);
      }

      static T* GetInstancePtr(void) {
        return singleton;
      }

      virtual void Destroy() {
        delete singleton;
        singleton = 0;
      }

    protected:
      static T* singleton;

  };

}

#endif
