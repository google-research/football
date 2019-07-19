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

#ifndef _HPP_SCHEDULER
#define _HPP_SCHEDULER

#include "../defines.hpp"

#include "../systems/isystemtask.hpp"
#include "../types/iusertask.hpp"
#include "tasksequence.hpp"

namespace blunted {

  struct TaskSequenceProgram {
    boost::shared_ptr<TaskSequence> taskSequence;
    int programCounter = 0;
    int previousProgramCounter = 0;
    unsigned long sequenceStartTime = 0;
    unsigned long lastSequenceTime = 0;
    unsigned long startTime = 0;
    int timesRan = 0;
    // set to true if sequence is finished
    bool readyToQuit = false;
  };

  // sort of 'light version' of the above, meant as informative to return to nosey enquirers
  struct TaskSequenceInfo {
    TaskSequenceInfo() {
      sequenceStartTime_ms = 0;
      lastSequenceTime_ms = 0;
      startTime_ms = 0;
      sequenceTime_ms = 0;
      timesRan = 0;
    }
    unsigned long sequenceStartTime_ms = 0;
    unsigned long lastSequenceTime_ms = 0;
    unsigned long startTime_ms = 0;
    int sequenceTime_ms = 0;
    int timesRan = 0;
  };

  struct TaskSequenceQueueEntry {
    TaskSequenceQueueEntry() {
      timeUntilDueEntry_ms = 0;
    }
    boost::shared_ptr<TaskSequenceProgram> program;
    long timeUntilDueEntry_ms = 0;
  };

  class Scheduler {

    public:
      Scheduler();
      virtual ~Scheduler();

      int GetSequenceCount();
      void RegisterTaskSequence(boost::shared_ptr<TaskSequence> sequence);
      void ResetTaskSequenceTime(const std::string &name);
      TaskSequenceInfo GetTaskSequenceInfo(const std::string &name);

      /// send due system tasks a SystemTaskMessage_StartFrame message
      /// invoke due user tasks with an Execute() call
      bool Run();

    protected:
      unsigned long previousTime_ms = 0;
      std::vector < boost::shared_ptr<TaskSequenceProgram> > sequences;

  };

}

#endif
