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

#ifndef _HPP_MESSAGEQUEUE
#define _HPP_MESSAGEQUEUE

#include "../defines.hpp"

#include "../base/properties.hpp"

#include "../types/command.hpp"

namespace blunted {

  template <typename T = boost::intrusive_ptr<Command> >
  class MessageQueue {

    public:
      MessageQueue() { DO_VALIDATION;}
      virtual ~MessageQueue() { DO_VALIDATION;}

      inline void PushMessage(T message, bool notify = true) { DO_VALIDATION;
        queue.push_back(message);
        if (notify) NotifyWaiting();
      }

      inline void NotifyWaiting() { DO_VALIDATION;
        messageNotification.notify_one();
      }

      inline T GetMessage(bool &MsgAvail) { DO_VALIDATION;
        T message;
        if (queue.size() > 0) { DO_VALIDATION;
          message = *queue.begin();
          queue.pop_front();
          MsgAvail = true;
        } else {
          MsgAvail = false;
        }
        return message;
      }

    protected:
      std::list < T > queue;
      boost::condition messageNotification;

  };

}

#endif
