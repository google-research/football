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

#include "main.hpp"

int main(int argc, char** argv) {
  try {
    Properties* input_config = new Properties();
    std::string configFile = "football.config";
    if (argc > 1) configFile = argv[1];
    input_config->LoadFile(configFile.c_str());

    run_game(input_config);
    Run();
    quit_game();
  } catch (...) {
    printf("CRASH\n");
    exit(-1);
  }
  return 0;
}
