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

#include "refcounted.hpp"

namespace blunted {

  RefCounted::RefCounted() : refCount(0) {
  }

  RefCounted::~RefCounted() {
  }

  RefCounted::RefCounted(const RefCounted &src) : refCount(0) {
  }

  RefCounted& RefCounted::operator=(const RefCounted &src) {
    return *this;
  }

  unsigned long RefCounted::GetRefCount() {
    int i = refCount;
    return i;
  }

  void intrusive_ptr_add_ref(RefCounted *p) {
    assert(p);
    ++(p->refCount);
  }

  void intrusive_ptr_release(RefCounted *p) {
    assert(p);
    if (--(p->refCount) == 0) {
      delete p;
    }
  }

}
