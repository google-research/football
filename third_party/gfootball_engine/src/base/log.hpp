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

#ifndef _HPP_LOG
#define _HPP_LOG

#include <string>

void DoValidation(int line, const char* file);
// Uncomment line below to enable validation.
// #define FULL_VALIDATION 1
#ifdef FULL_VALIDATION
  #define DO_VALIDATION DoValidation(__LINE__, __FILE__);
#else
  #define DO_VALIDATION ;
#endif

namespace blunted {

  enum e_LogType {
    e_Warning,
    e_Error,
    e_FatalError
  };

  void Log(e_LogType logType, std::string className, std::string methodName, std::string message);

}

#endif
