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

#include "database.hpp"

#include "../base/log.hpp"

#include <sqlite3.h>

namespace blunted {

  Database::Database() : db(0) {
  }

  Database::~Database() {
    if (db) sqlite3_close(db);
  }


  bool Database::Load(const std::string &filename) {

    // close previously opened db
    if (db) sqlite3_close(db);

    int value = sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READWRITE, 0);
    if (value) {
      //Log(e_FatalError, "Database", "Database", "Could not open database '" + filename + "'");
      return false;
    } else return true;
  }

  DatabaseResult *Database::Query(const std::string &query) {

    int rows, columns;
    char **result;
    char *errorMsg = 0;

    sqlite3_get_table(db, query.c_str(), &result, &rows, &columns, &errorMsg);
    if (errorMsg) {
      std::string errorMsgStr = errorMsg;
      Log(e_FatalError, "Database", "Query", "SQLite error message: '" + errorMsgStr + "'");
      sqlite3_free(errorMsg);
    }

    DatabaseResult *dbresult = new DatabaseResult();

    // header
    for (int i = 0; i < columns; i++) {
      //printf("1st row heading: %s\n", result[i]);
      dbresult->header.push_back(result[i]);
    }

    // data
    for (int r = 0; r < rows; r++) {
      std::vector<std::string> tmpRow;
      for (int c = 0; c < columns; c++) {
        if (result[(r + 1) * columns + c] == NULL) {
          tmpRow.push_back("");
        } else {
          //printf("data: %s\n", result[(r + 1) * columns + c]);
          tmpRow.push_back(result[(r + 1) * columns + c]);
        }
      }
      dbresult->data.push_back(tmpRow);
    }

    sqlite3_free_table(result);

    return dbresult;

  }

}
