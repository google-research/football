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

#include <stdio.h>
#ifndef WIN32
#include <execinfo.h>
#include <unistd.h>
#endif
#include <signal.h>
#include "backtrace.h"
#include <stdlib.h>

void print_stacktrace() {
  void *array[20];
  size_t size;
#ifndef WIN32
  size = backtrace(array, 20);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
}


void handler(int sig) {
  print_stacktrace();
  printf("Error: signal %d:\n", sig);
  exit(1);
}

void install_stacktrace() {
  signal(SIGSEGV, handler);
}
