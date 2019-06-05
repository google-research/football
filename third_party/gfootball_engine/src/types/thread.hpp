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

#ifndef _HPP_THREAD
#define _HPP_THREAD

#include "../defines.hpp"

#include "../types/messagequeue.hpp"
#include "../types/lockable.hpp"

#include "boost/thread.hpp"

namespace blunted {

  enum e_ThreadState {
    e_ThreadState_Idle,
    e_ThreadState_Sleeping,
    e_ThreadState_Busy,
    e_ThreadState_Exiting
  };

  class Thread {

    public:
      Thread() {
      }

      virtual ~Thread() {
      }

      // --- /CARE


      void Run() {
        operator()();
      }

      void Join() {
        thread.join();
      }

      // thread main loop
      virtual void operator()() = 0;

      //MessageQueue < boost::intrusive_ptr<Command> > messageQueue;

      boost::thread thread;

    protected:
      Lockable<e_ThreadState> state;

  };


  // messages

  class Message_Shutdown : public Command {

    public:
      Message_Shutdown() : Command("shutdown") {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        return false;
      }

  };

}

#endif
