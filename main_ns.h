#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <sqlite3.h>

#define MEASURE_SPACE

namespace main_ns
{
  using namespace std;

  enum mode_t {
    blackbox,
    eliminate_reorder,
    node_copying
  };

  enum operation_type_t {
    insert,
    modify,
    remove,
    access
  };

  uint64_t start_time;
  size_t count = 1000UL;
  size_t max_no_snapshots = 2000UL;
  size_t max_snapshot_dist = 65UL;
  bool store_results = false;
  bool randomize_operations = false;
  size_t current_op_no = 0;

  string
  mode_to_string (mode_t mode)
  {
    switch (mode) {
    case blackbox:
      return "blackbox";
    case eliminate_reorder:
      return "eliminate_reorder";
    case node_copying:
      return "node_copying";
    default:
      return "unknown";
    }
  }

  double
  nano_time ()
  {
    timespec ts;
    clock_gettime (CLOCK_REALTIME, &ts);

    return ts.tv_sec * 1e9 + ts.tv_nsec;
  }

  std::string exec (string cmd)
  {
    FILE* pipe = popen (cmd.c_str (), "r");
    if (!pipe)
      return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof (pipe)) {
      if (fgets (buffer, 128, pipe) != NULL)
        result += buffer;
    }
    pclose (pipe);
    return result;
  }

  int
  callback (void* NotUsed, int argc, char** argv, char** azColName)
  {
    for (int i = 0; i < argc; ++i) {
      cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    return 0;
  }

#ifndef MEASURE_SPACE
  void
  log_operation_to_db (const mode_t mode, size_t count, const string operation,
                       size_t max_no_snapshots, size_t max_snapshot_dist,
                       const double begin_operation,
                       const double end_operation)
  {
    sqlite3* db;
    char* zErrMsg;
    int rc;
    rc = sqlite3_open ("sqlite.db", &db);
    if (rc) {
      stringstream ss;
      ss << "Can't open database \"sqlite.db\": " << sqlite3_errmsg (db);
      sqlite3_close (db);
      throw ss.str ();
    }
    string git_hash = exec ("git rev-parse HEAD");
    git_hash.erase (std::remove (git_hash.begin(), git_hash.end(), '\n'), git_hash.end());
    stringstream sql;
    sql.precision (15);
    sql.setf (ios::fixed);
    sql <<
        "insert into results (start_time, implementation, count, max_no_snapshots, max_snapshot_dist, version, begin_time, end_time, operation, duration) values (" << start_time << ", '"
        << mode_to_string (mode) << "', " << count << ", " << max_no_snapshots << ", " << max_snapshot_dist << ", " << "'" << git_hash << "', " << (long
            long)
        begin_operation << ", " << (long long) end_operation << ", '" << operation
        << "', " << (long long) (end_operation - begin_operation) << ")";
    rc = sqlite3_exec (db, sql.str ().c_str (), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
      stringstream ss;
      ss << "SQL error: " << zErrMsg << endl;
      sqlite3_free (zErrMsg);
      throw ss.str ();
    }
    sqlite3_close (db);
  }
#else
  void
  log_space_to_db (const string usage_scenario, const mode_t mode, const size_t count, const size_t max_no_snapshots, const size_t max_snapshot_dist, const long long space) {
    sqlite3* db;
    char* zErrMsg;
    int rc;
    rc = sqlite3_open ("sqlite.db", &db);
    if (rc) {
      stringstream ss;
      ss << "Can't open database \"sqlite.db\": " << sqlite3_errmsg (db);
      sqlite3_close (db);
      throw ss.str ();
    }
    string git_hash = exec ("git rev-parse HEAD");
    git_hash.erase (std::remove (git_hash.begin(), git_hash.end(), '\n'), git_hash.end());
    stringstream sql;
    sql.precision (15);
    sql.setf (ios::fixed);
    sql <<
        "insert into space (usage_scenario, implementation, count, max_no_snapshots, max_snapshot_dist, version, space) values ('" << usage_scenario << "', '"
        << mode_to_string (mode) << "', " << count << ", " << max_no_snapshots << ", " << max_snapshot_dist << ", " << "'" << git_hash << "', " << space << ")";
    rc = sqlite3_exec (db, sql.str ().c_str (), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
      stringstream ss;
      ss << "SQL error: " << zErrMsg << endl;
      sqlite3_free (zErrMsg);
      throw ss.str ();
    }
    sqlite3_close (db);
  }
#endif
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
