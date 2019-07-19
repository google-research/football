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

#include "tasksequence.hpp"

#include "../framework/scheduler.hpp"
#include "../blunted.hpp"

namespace blunted {

  
  // TaskSequenceEntry_SystemTaskMessage

  TaskSequenceEntry_SystemTaskMessage::TaskSequenceEntry_SystemTaskMessage(boost::intrusive_ptr<ISystemTaskMessage> command) : command(command) {
  }

  TaskSequenceEntry_SystemTaskMessage::~TaskSequenceEntry_SystemTaskMessage() {
  }

  bool TaskSequenceEntry_SystemTaskMessage::Execute() {
    command->Handle(command->GetTask());
    //command->GetTask()->messageQueue.PushMessage(command, true);
    return true;
  }

  bool TaskSequenceEntry_SystemTaskMessage::IsReady() {
    return command->IsReady();
  }

  // TaskSequenceEntry_UserTaskMessage

  TaskSequenceEntry_UserTaskMessage::TaskSequenceEntry_UserTaskMessage(boost::intrusive_ptr<IUserTaskMessage> command) : command(command) {
  }

  TaskSequenceEntry_UserTaskMessage::~TaskSequenceEntry_UserTaskMessage() {
  }

  bool TaskSequenceEntry_UserTaskMessage::Execute() {
    command->Handle();
    return true;
  }

  bool TaskSequenceEntry_UserTaskMessage::IsReady() {
    return command->IsReady();
  }

  // TaskSequenceEntry_Terminator

  TaskSequenceEntry_Terminator::TaskSequenceEntry_Terminator() {
  }

  TaskSequenceEntry_Terminator::~TaskSequenceEntry_Terminator() {
  }

  bool TaskSequenceEntry_Terminator::Execute() {
    return true;
  }

  bool TaskSequenceEntry_Terminator::IsReady() {
    return true;
  }


  // TaskSequence

  TaskSequence::TaskSequence(const std::string &name, int sequenceTime_ms, bool skipOnTooLate) : name(name), sequenceTime_ms(sequenceTime_ms), skipOnTooLate(skipOnTooLate) {
  }

  TaskSequence::~TaskSequence() {
    entries.clear();
  }

  void TaskSequence::AddEntry(boost::shared_ptr<ITaskSequenceEntry> entry) {
    entries.push_back(entry);
  }

  void TaskSequence::AddSystemTaskEntry(ISystem *system, e_TaskPhase taskPhase) {
    boost::intrusive_ptr<ISystemTaskMessage> message;

    std::string name = "sequence:" + GetName() + "/systemtask:" + system->GetName() + "/";

    switch (taskPhase) {
      case e_TaskPhase_Get:
        name.append("get");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_GetPhase(name, system->GetTask()));
        break;
      case e_TaskPhase_Process:
        name.append("process");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_ProcessPhase(name, system->GetTask()));
        break;
      case e_TaskPhase_Put:
        name.append("put");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_PutPhase(name, system->GetTask()));
        break;
    }
    boost::shared_ptr<TaskSequenceEntry_SystemTaskMessage> taskSequenceEntry(new TaskSequenceEntry_SystemTaskMessage(message));

    AddEntry(taskSequenceEntry);
  }

  void TaskSequence::AddUserTaskEntry(boost::shared_ptr<IUserTask> userTask, e_TaskPhase taskPhase) {
    boost::intrusive_ptr<IUserTaskMessage> message;

    std::string name = "sequence:" + GetName() + "/usertask:" + userTask->GetName() + "/";

    switch (taskPhase) {
      case e_TaskPhase_Get:
        name.append("get");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_GetPhase(name, userTask));
        break;
      case e_TaskPhase_Process:
        name.append("process");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_ProcessPhase(name, userTask));
        break;
      case e_TaskPhase_Put:
        name.append("put");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_PutPhase(name, userTask));
        break;
    }
    boost::shared_ptr<TaskSequenceEntry_UserTaskMessage> taskSequenceEntry(new TaskSequenceEntry_UserTaskMessage(message));

    AddEntry(taskSequenceEntry);
  }

  void TaskSequence::AddTerminator() {
    boost::shared_ptr<ITaskSequenceEntry> arnie(new TaskSequenceEntry_Terminator());
    AddEntry(arnie);
  }


  int TaskSequence::GetEntryCount() const {
    return entries.size();
  }

  boost::shared_ptr<ITaskSequenceEntry> TaskSequence::GetEntry(int num) {
    assert(num < (signed int)entries.size());
    return entries.at(num);
  }

  int TaskSequence::GetSequenceTime() const {
    return sequenceTime_ms;
  }

  const std::string TaskSequence::GetName() const {
    return name;
  }

}
