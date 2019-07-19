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

#include "scheduler.hpp"

#include "../managers/resourcemanagerpool.hpp"
#include "../managers/environmentmanager.hpp"
#include "../base/log.hpp"

namespace blunted {

  Scheduler::Scheduler() {
    previousTime_ms = 0;
  }

  Scheduler::~Scheduler() {
  }

  int Scheduler::GetSequenceCount() {
    int size = sequences.size();
    return size;
  }

  void Scheduler::RegisterTaskSequence(boost::shared_ptr<TaskSequence> sequence) {
    if (sequence->GetEntryCount() == 0) Log(e_FatalError, "Scheduler", "RegisterTaskSequence", "Trying to add a sequence without entries");
    sequence->AddTerminator();
    boost::shared_ptr<TaskSequenceProgram> program(new TaskSequenceProgram());
    unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
    program->taskSequence = sequence;
    program->programCounter = 0;
    program->previousProgramCounter = -1;
    program->sequenceStartTime = time_ms;
    program->lastSequenceTime = 0;
    program->startTime = time_ms;
    program->timesRan = 0;
    program->readyToQuit = false;
    sequences.push_back(program);
  }

  void Scheduler::ResetTaskSequenceTime(const std::string &name) {
    for (unsigned int i = 0; i < sequences.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences[i];
      if (program->taskSequence->GetName() == name) {
        program->startTime = EnvironmentManager::GetInstance().GetTime_ms();// - startTime_ms;
        program->timesRan = 0;
        break;
      }
    }
  }

  TaskSequenceInfo Scheduler::GetTaskSequenceInfo(const std::string &name) {
    TaskSequenceInfo info;
    for (unsigned int i = 0; i < sequences.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences[i];
      if (program->taskSequence->GetName() == name) {

        info.sequenceStartTime_ms = program->sequenceStartTime;
        info.lastSequenceTime_ms = program->lastSequenceTime;
        info.startTime_ms = program->startTime;
        info.sequenceTime_ms = program->taskSequence->GetSequenceTime();
        info.timesRan = program->timesRan;

        break;
      }
    }
    return info;
  }

  bool Scheduler::Run() {

    unsigned int firstSequence = 0;
    //while (true) {

      unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
      unsigned long timeDiff_ms = time_ms - previousTime_ms;
      previousTime_ms = time_ms;

      // find first sequence entry that needs to be started

      TaskSequenceQueueEntry dueEntry;

      bool someSequenceNeedsDeleting = false;

      for (unsigned int i = 0; i < sequences.size(); i++) {
        int programIndex = (i + firstSequence) % sequences.size();
        boost::shared_ptr<TaskSequenceProgram> program = sequences.at(programIndex);

        // check if previous entry is ready
        bool previousEntryIsReady = true;
        if (program->previousProgramCounter != -1) {

          previousEntryIsReady = program->taskSequence->GetEntry(program->previousProgramCounter)->IsReady();

          if (previousEntryIsReady && program->programCounter == program->taskSequence->GetEntryCount()) {
            program->programCounter = 0;
            program->timesRan++;
            program->lastSequenceTime = time_ms - program->sequenceStartTime;
          }

        }

        if (previousEntryIsReady) {

          long timeUntilDueEntry_ms = 0; // if programCounter != 0, we just want to start the next entry ASAP

          if (program->programCounter == 0) { // else, (re)starting sequence; find out when it's due

            if (program->taskSequence->GetSkippable()) {
              // use relative time: don't mind if last frame lasted too long
              timeUntilDueEntry_ms = (program->sequenceStartTime + program->taskSequence->GetSequenceTime()) - time_ms;
            } else {
              // use absolute time: if not enough iterations have been done to get to frametime * timesran, start immediately
              timeUntilDueEntry_ms = (program->startTime + program->taskSequence->GetSequenceTime() * program->timesRan) - time_ms;
            }
          }

          if (!program->readyToQuit && (timeUntilDueEntry_ms < dueEntry.timeUntilDueEntry_ms || dueEntry.program == boost::shared_ptr<TaskSequenceProgram>())) {
            dueEntry.program = program;
            dueEntry.timeUntilDueEntry_ms = timeUntilDueEntry_ms;
          }

        }

      } // checked all sequences and (hopefully) found an entry that is due next


      // delete sequences that are ready to pussy out

      if (someSequenceNeedsDeleting) {

        std::vector < boost::shared_ptr<TaskSequenceProgram> >::iterator quiterator = sequences.begin();
        while (quiterator != sequences.end()) {
          boost::shared_ptr<TaskSequenceProgram> program = *quiterator;
          if (program->readyToQuit == true) {
            quiterator = sequences.erase(quiterator);
          } else {
            quiterator++;
          }
        }

        // //sequences.Unlock();
        // continue;

      } else { // (no deletes)

        // switch first sequence to handle next time (so they all get a turn)

        firstSequence++;
        if (firstSequence >= sequences.size()) firstSequence = 0;

        //sequences.Unlock();
        //bool isMessage = somethingIsDone. timed_wait(lock, tAbsoluteTime);
        //sequences.Lock();

        if (dueEntry.program != boost::shared_ptr<TaskSequenceProgram>()) {

          if (dueEntry.timeUntilDueEntry_ms <= 0) { // the first and/or other entries are due

            // run!
            if (dueEntry.program->programCounter == 0) {
              dueEntry.program->sequenceStartTime = time_ms;
            }

            dueEntry.program->taskSequence->GetEntry(dueEntry.program->programCounter)->Execute();

            dueEntry.program->previousProgramCounter = dueEntry.program->programCounter;
            dueEntry.program->programCounter++;

          } else {
            EnvironmentManager::GetInstance().IncrementTime_ms(10);
          }
        }

      }
    return GetSequenceCount() > 0;
  }


}
