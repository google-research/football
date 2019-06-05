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

#include "synchronizationTask.hpp"
#include "main.hpp"

SynchronizationTask::SynchronizationTask() {
}

SynchronizationTask::~SynchronizationTask() {
}

bool SynchronizationTask::inPlay() {
  if (!GetGameTask()->GetMatch()) {
    return false;
  }
  return true;
}

void SynchronizationTask::GetPhase() {
  if (!inPlay()) {
    return;
  }
  steps_--;
}

void SynchronizationTask::ProcessPhase() {
}

void SynchronizationTask::PutPhase() {
}

void SynchronizationTask::Step(int steps) {
  steps_ = steps;
}

int SynchronizationTask::Steps() {
  return steps_;
}
