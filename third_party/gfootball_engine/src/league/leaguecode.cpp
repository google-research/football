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

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "leaguecode.hpp"

#define BOOST_FILESYSTEM_VERSION 3

#include "../base/utils.hpp"
#include "../utils/database.hpp"
#include "../utils/xmlloader.hpp"

int CreateNewLeagueSave(const std::string &srcDbName, const std::string &saveName) {

  // copy db file

  int errorCode = 0;
  // 1 == could not create save dir
  // 2 == could not copy db file
  // 3 == could not open copied database
  // 4 == could not copy file

  boost::filesystem::path source("databases");
  source /= srcDbName;
  source /= "database.sqlite";

  boost::filesystem::path dest("saves");
  dest /= saveName;

  if (!CreateDirectory(dest)) {
    errorCode = 1; // could not create dir
  } else {
    if (!CopyFile(source, dest)) errorCode = 2; // could not copy file
  }


  // copy league db to tmp db

  boost::system::error_code error;
  namespace fs = boost::filesystem;
  fs::copy_file(dest / "database.sqlite", dest / "autosave.sqlite", error);


  // check db for graphics files and copy those

  Database *database;

  if (errorCode == 0) {
    database = GetDB();
    if (!database->Load(dest.string() + "/autosave.sqlite")) {
      errorCode = 3; // could not open database
    }
  }

  if (errorCode == 0) {
    std::vector<std::string> imageList;
    DatabaseResult *result = database->Query("select logo_url, kit_url from teams");
    for (unsigned int r = 0; r < result->data.size(); r++) {
      imageList.push_back(result->data.at(r).at(0));
      imageList.push_back(result->data.at(r).at(1));
    }
    delete result;
    result = database->Query("select logo_url from competitions");
    for (unsigned int r = 0; r < result->data.size(); r++) {
      imageList.push_back(result->data.at(r).at(0));
    }
    delete result;


    // create directories, copy files

    for (unsigned int i = 0; i < imageList.size(); i++) {
      //printf("copying %s\n", imageList.at(i).c_str());
      std::vector<std::string> tokens;
      tokenize(imageList.at(i), tokens, "/\\");

      boost::filesystem::path newdir = dest;
      for (unsigned int x = 0; x < tokens.size() - 1; x++) {
        // does directory exist?
        newdir /= tokens.at(x);
        if (!boost::filesystem::exists(newdir)) {
          boost::filesystem::create_directory(newdir);
          //printf("created dir: %s\n", newdir.string().c_str());
        }
        //printf("%s ", tokens.at(x).c_str());
      }
      //printf("\n");
      boost::filesystem::path destfile = newdir / tokens.at(tokens.size() - 1);
      boost::filesystem::path sourcefile("databases");
      sourcefile /= srcDbName;
      sourcefile /= imageList.at(i);
      boost::system::error_code error;
      //printf("copying from %s to %s\n", sourcefile.string().c_str(), destfile.string().c_str());
      if (!boost::filesystem::exists(destfile)) boost::filesystem::copy_file(sourcefile, destfile, error);
      if (error) errorCode = 4;
      //if (error) printf("file %s could not be copied\n", imageList.at(i).c_str());
      //printf("\n");
    }
  } // if !error

  return errorCode;
}

bool PrepareDatabaseForLeague() {

  DatabaseResult *result = GetDB()->Query("CREATE TABLE settings(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                "managername VARCHAR(32), "
                                                                "team_id INTEGER, "
                                                                "currency VARCHAR(32), "
                                                                "difficulty FLOAT, "
                                                                "seasonyear INTEGER, "
                                                                "timestamp DATETIME)");
  delete result;

  result = GetDB()->Query("CREATE TABLE calendar(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                "timestamp DATETIME, "
                                                                "team1_id INTEGER, "
                                                                "team2_id INTEGER, "
                                                                "competition_id INTEGER, "
                                                                "tournament_id INTEGER, "
                                                                "timestamp DATETIME)");
  delete result;

  result = GetDB()->Query("ALTER TABLE players ADD COLUMN stats_temporal BLOB");
  delete result;

  // copy stats XML tree into a new XML tree as subset 'current' (this tree is also going to contain the archive per year)

  result = GetDB()->Query("SELECT id, stats FROM players");

  std::string insertTemporalStatsQuery = "begin transaction;";

  for (unsigned int r = 0; r < result->data.size(); r++) {
    std::string playerIDString = result->data.at(r).at(0);
    std::string statsString = result->data.at(r).at(1);

    XMLLoader loader;
    XMLTree tree = loader.Load(statsString);
//    loader.PrintTree(tree);
//    printf("\n\n\n");
    XMLTree resultTree;
    resultTree.children.insert(std::pair<std::string, XMLTree>("current", tree));

    std::string resultTreeString = loader.GetSource(resultTree);
    insertTemporalStatsQuery += "UPDATE players SET stats_temporal ='" + resultTreeString + "' WHERE id = " + playerIDString + ";";
  }

  delete result;

  insertTemporalStatsQuery += "commit;";
  DatabaseResult *insertTemporalStats = GetDB()->Query(insertTemporalStatsQuery);
  delete insertTemporalStats;

  return true;
}

bool SaveAutosaveToDatabase() {

  namespace fs = boost::filesystem;
  fs::path dest("saves");
  dest /= GetActiveSaveDirectory();

  boost::system::error_code error;

  // remove previous database
  if (fs::exists(dest / "database.sqlite")) fs::remove(dest / "database.sqlite");

  // copy autosave to database
  fs::copy_file(dest / "autosave.sqlite", dest / "database.sqlite", error);

  if (error) return false; else return true;
}

bool SaveDatabaseToAutosave() {

  namespace fs = boost::filesystem;
  fs::path dest("saves");
  dest /= GetActiveSaveDirectory();

  boost::system::error_code error;

  // remove previous autosave
  if (fs::exists(dest / "autosave.sqlite")) fs::remove(dest / "autosave.sqlite");

  // copy database to autosave
  fs::copy_file(dest / "database.sqlite", dest / "autosave.sqlite", error);

  if (error) return false; else return true;
}

void GenerateSeasonCalendars() {
  DatabaseResult *result = GetDB()->Query("SELECT strftime(\"%w\", timestamp) FROM settings LIMIT 1"); // day where season starts
  int dayOfWeek = atoi(result->data.at(0).at(0).c_str());
  delete result;

  // we want to start on a saturday (day of week '6')
  int offset = 6 - dayOfWeek;
  int daysToSkip = 4;
  int weekCounter = 0;
  bool weekend = true;

  bool ready = false;
  while (!ready) {

    offset += daysToSkip;
    if (daysToSkip == 3) {
      daysToSkip = 4;
      weekend = true;
      weekCounter++;
    } else {
      daysToSkip = 3;
      weekend = false;
    }
  }

  //result = GetDB()->Query("UPDATE settings SET timestamp = date(timestamp, '+" + int_to_str(offset) + " day')");
  //delete result;
}
