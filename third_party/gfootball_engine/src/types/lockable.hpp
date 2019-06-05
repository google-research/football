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

#ifndef _HPP_LOCKABLE
#define _HPP_LOCKABLE

#include "../defines.hpp"

namespace blunted {

  enum e_NotificationSubject {
    e_NotificationSubject_One,
    e_NotificationSubject_All,
  };

  template <typename T>
  class Lockable {

    public:

      Lockable() {}

      T GetData() const {
        mutex.lock();
        T tmp = data;
        mutex.unlock();
        return tmp;
      }

      void SetData(const T &newdata) {
        mutex.lock();
        this->data = newdata;
        mutex.unlock();
      }

      inline T operator=(const T &param) {
        SetData(param);
        return param;
      }

      inline T *operator->() {
        return &data;
      }

      inline void Lock() {
        mutex.lock();
      }

      inline void Unlock() {
        mutex.unlock();
      }

      T data;

      mutable boost::mutex mutex;

    protected:

  };

}

#endif
