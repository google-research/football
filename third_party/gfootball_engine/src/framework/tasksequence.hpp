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

#ifndef _HPP_TASKSEQUENCE
#define _HPP_TASKSEQUENCE

#include "../defines.hpp"

#include "../types/command.hpp"

#include "../systems/isystem.hpp"
#include "../systems/isystemtask.hpp"
#include "../types/iusertask.hpp"

namespace blunted {

  class ITaskSequenceEntry {

    public:
      virtual ~ITaskSequenceEntry() {};
      virtual bool Execute() = 0;
      virtual bool IsReady() = 0;

    protected:

  };

  class TaskSequenceEntry_SystemTaskMessage : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_SystemTaskMessage(boost::intrusive_ptr<ISystemTaskMessage> command);
      virtual ~TaskSequenceEntry_SystemTaskMessage();

      virtual bool Execute();
      virtual bool IsReady();

    protected:
      boost::intrusive_ptr<ISystemTaskMessage> command;

  };

  class TaskSequenceEntry_UserTaskMessage : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_UserTaskMessage(boost::intrusive_ptr<IUserTaskMessage> command);
      virtual ~TaskSequenceEntry_UserTaskMessage();

      virtual bool Execute();
      virtual bool IsReady();

    protected:
      boost::intrusive_ptr<IUserTaskMessage> command;

  };

  class TaskSequenceEntry_Terminator : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_Terminator();
      virtual ~TaskSequenceEntry_Terminator();

      virtual bool Execute();
      virtual bool IsReady();

  };

  enum e_TaskPhase {
    e_TaskPhase_Get,
    e_TaskPhase_Process,
    e_TaskPhase_Put
  };

  enum e_LockAction {
    e_LockAction_Lock,
    e_LockAction_Unlock
  };

  class TaskSequence {

    public:
      TaskSequence(const std::string &name, int sequenceTime_ms, bool skipOnTooLate = true);
      virtual ~TaskSequence();

      void AddEntry(boost::shared_ptr<ITaskSequenceEntry> entry);
      void AddSystemTaskEntry(ISystem *system, e_TaskPhase taskPhase);
      void AddUserTaskEntry(boost::shared_ptr<IUserTask> userTask,
                            e_TaskPhase taskPhase);
      void AddTerminator();

      int GetEntryCount() const;
      boost::shared_ptr<ITaskSequenceEntry> GetEntry(int num);
      int GetSequenceTime() const;
      const std::string GetName() const;
      bool GetSkippable() const { return skipOnTooLate; }

    protected:
      std::string name;

      std::vector < boost::shared_ptr<ITaskSequenceEntry> > entries;

      // time assigned for 1 run of this sequence
      // if 0, run continuously
      int sequenceTime_ms = 0;

      // if at due start time the previous run is not ready yet,
      // if true: just forget about the lost time
      // if false: start as soon as previous run is ready, to keep up with the sync
      bool skipOnTooLate = false;

  };

}

#endif
