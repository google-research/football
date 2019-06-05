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

#ifndef _HPP_SUBJECT
#define _HPP_SUBJECT

#include "../defines.hpp"

#include "../types/command.hpp"
#include "../types/interpreter.hpp"

#include "../base/log.hpp"

namespace blunted {

  // this class is not reentrant, derived classes and their users take this responsibility (by using Subject::subjectMutex)
  template <class T = Observer>
  class Subject {

    public:
      Subject() {
      }
      
      virtual ~Subject() {
        observers.clear();
      }

      virtual void Attach(boost::intrusive_ptr<T> observer, void *thisPtr = 0) {

        observer->SetSubjectPtr(thisPtr);

        observers.push_back(observer);
      }

      virtual void Detach(boost::intrusive_ptr<T> observer) {
        typename std::vector < boost::intrusive_ptr<T> >::iterator o_iter = observers.begin();
        while (o_iter != observers.end()) {
          if ((*o_iter).get() == observer.get()) {
            (*o_iter).reset();
            o_iter = observers.erase(o_iter);
          } else {
            o_iter++;
          }
        }
      }

      virtual void DetachAll() {
        typename std::vector < boost::intrusive_ptr<T> >::iterator o_iter = observers.begin();
        while (o_iter != observers.end()) {
          (*o_iter).reset();
          o_iter = observers.erase(o_iter);
        }
      }

    protected:
      std::vector < boost::intrusive_ptr<T> > observers;

  };

}

#endif
