#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include <algorithm>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <iomanip>

#include <sqlite3.h>

#include "ephemeral/DoublyLinkedList.h"
#include "partiallypersistent/DoublyLinkedList.h"
#include "rollback_naive/DoublyLinkedList.h"
#include "rollback_reorder/DoublyLinkedList.h"

// #undef RANDOMIZE
#define RANDOMIZE

using namespace std;

namespace main_ns {
enum mode_t {
  rollback_naive,
  rollback_reorder,
  partiallypersistent
};

uint64_t start_time;

string
mode_to_string (mode_t mode) {
  switch (mode) {
  case rollback_naive:
    return "rollback_naive";
  case rollback_reorder:
    return "rollback_reorder";
  case partiallypersistent:
    return "partiallypersistent";
  default:
    return "unknown";
  }
}

std::string exec (string cmd) {
  FILE *pipe = popen (cmd.c_str (), "r");
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
callback (void *NotUsed, int argc, char **argv, char **azColName) {
  for (int i = 0; i < argc; ++i) {
    cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
  }
  return 0;
}

double
nano_time () {
  timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);

  return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void
dump_list_dot_graph (partiallypersistent::DoublyLinkedList & list) {
  ofstream file;
  file.open ("graph", ios_base::out);
  list.print_dot_graph (list.version, file);
  file.close ();
}

void
print_all_versions (partiallypersistent::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       0; i < list.get_versions ().size (); ++i) {
    cout << "List at version " << list.get_versions ()[i].
      version << " (size " << list.get_versions ()[i].size << "): ";
    size_t printed_size =
      list.print_at_version (list.get_versions ()[i].version);
    cout << "printed size: " << printed_size << endl;
  }
}

void
print_all_versions (rollback_naive::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       1; i <= list.num_records (); ++i) {
    cout << "List at version " << i << " (size " << list.size_at (i) << "): ";
    size_t printed_size = list.print_at_version (i);
    cout << "printed size: " << printed_size << endl;
  }
}

void
print_all_versions (rollback_reorder::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       1; i <= list.num_records (); ++i) {
    cout << "List at version " << i << " (size " << list.size_at (i) << "): ";
    size_t printed_size = list.print_at_version (i);
    cout << "printed size: " << printed_size << endl;
  }
}

void
log_operation_to_db (const mode_t mode, size_t count, const string operation,
                     size_t max_no_snapshots, size_t max_snapshot_dist,
                     const double begin_operation,
                     const double end_operation) {
  sqlite3 *db;
  char *zErrMsg;
  int rc;
  rc = sqlite3_open ("sqlite.db", &db);
  if (rc) {
    stringstream ss;
    ss << "Can't open database \"sqlite.db\": " << sqlite3_errmsg (db);
    sqlite3_close (db);
    throw ss.str ();
  }
  string git_hash = exec ("git rev-parse HEAD");
  git_hash.erase(std::remove(git_hash.begin(), git_hash.end(), '\n'), git_hash.end());
  stringstream sql;
  sql.precision (15);
  sql.setf (ios::fixed);
  sql <<
    "insert into results (start_time, implementation, count, max_no_snapshots, max_snapshot_dist, version, begin_time, end_time, operation, duration) values (" << start_time << ", '"
    << mode_to_string(mode) << "', " << count << ", " << max_no_snapshots << ", " << max_snapshot_dist << ", " << "'" << git_hash << "', " << (long
                                                                      long)
    begin_operation << ", " << (long long) end_operation << ", '" << operation
    << "', " << (long long) (end_operation - begin_operation) << ")";
//   cout << sql.str () << endl;
  rc = sqlite3_exec (db, sql.str ().c_str (), callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    stringstream ss;
    ss << "SQL error: " << zErrMsg << endl;
    sqlite3_free (zErrMsg);
    throw ss.str ();
  }
  sqlite3_close (db);
}

void
test_insert_modify_remove_rollback_naive (bool store_results,
                                          size_t count,
                                          size_t max_no_snapshots,
                                          size_t max_snapshot_dist) {
  double begin_operation, end_operation;

  rollback_naive::DoublyLinkedList list (max_no_snapshots, max_snapshot_dist);

  cout << "rollback_naive;insert;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.size () >
                 0 ? (double) rand () * list.size () / RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;

  if (store_results) {
    log_operation_to_db (rollback_naive, count, "insert", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "rollback_naive;modify;" << count << ";";
#ifdef RANDOMIZE
    cout << "random;";
#else
    cout << "sequential;";
#endif
    begin_operation = nano_time ();
    for (size_t j = 0; j < count; ++j) {
      ephemeral::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      list.modify_data (index, j);
    }
    end_operation = nano_time ();
    cout << (end_operation - begin_operation) << endl;
    if (store_results) {
      log_operation_to_db (rollback_naive, count, "modify", max_no_snapshots,
                         max_snapshot_dist, begin_operation, end_operation);
    }
  }

  cout << "rollback_naive;remove;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#endif
      list.remove (index);
    } else {
      cout << "List empty at version " << list.num_records () << endl;
    }
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (rollback_naive, count, "remove", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }
    
  cout << "rollback_naive;access;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  size_t sum = 0UL;
  begin_operation = nano_time ();
  for (size_t i = 0UL; i < count; ++i) {
    size_t version_index = (double) rand () * list.num_records () / RAND_MAX;
    size_t v = version_index + 1;
    ephemeral::Node * n = list.head_at (v);
    size_t list_size = list.size_at (v);
    if (list_size == 0) {
      continue;
    }
    size_t list_index = (double) rand () * list_size / RAND_MAX;
    for (size_t j = 0; j < list_index; ++j) {
      if (n->next) {
        n = n->next;
      } else {
        break;
      }
    }
    size_t data = n->data;
    sum += data;
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (rollback_naive, count, "access", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }
}

void
test_insert_modify_remove_rollback_reorder (bool store_results,
                                            size_t count,
                                            size_t max_no_snapshots,
                                            size_t max_snapshot_dist) {
  double begin_operation, end_operation;

  rollback_reorder::DoublyLinkedList list (max_no_snapshots, max_snapshot_dist);

  cout << "rollback_reorder;insert;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.size () >
                 0 ? (double) rand () * list.size () / RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;

  if (store_results) {
    log_operation_to_db (rollback_reorder, count, "insert", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "rollback_reorder;modify;" << count << ";";
#ifdef RANDOMIZE
    cout << "random;";
#else
    cout << "sequential;";
#endif
    begin_operation = nano_time ();
    for (size_t j = 0; j < count; ++j) {
      ephemeral::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      list.modify_data (index, j);
    }
    end_operation = nano_time ();
    cout << (end_operation - begin_operation) << endl;
    if (store_results) {
      log_operation_to_db (rollback_reorder, count, "modify", max_no_snapshots,
                           max_snapshot_dist, begin_operation, end_operation);
    }
  }

  cout << "rollback_reorder;remove;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#endif
      list.remove (index);
    } else {
      cout << "List empty at version " << list.num_records () << endl;
    }
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (rollback_reorder, count, "remove", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }

  cout << "rollback_reorder;access;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  size_t sum = 0UL;
  begin_operation = nano_time ();
  for (size_t i = 0UL; i < count; ++i) {
    size_t version_index = (double) rand () * list.num_records () / RAND_MAX;
    size_t v = version_index + 1;
    ephemeral::Node * n = list.head_at (v);
    size_t list_size = list.size_at (v);
    if (list_size == 0) {
      continue;
    }
    size_t list_index = (double) rand () * list_size / RAND_MAX;
    for (size_t j = 0; j < list_index; ++j) {
      if (n->next) {
        n = n->next;
      } else {
        break;
      }
    }
    size_t data = n->data;
    sum += data;
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (rollback_reorder, count, "access", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }
}

void
test_insert_modify_remove_partiallypersistent (bool store_results, size_t count) {
  double begin_operation, end_operation;

  partiallypersistent::DoublyLinkedList list;

  cout << "partiallypersistent;insert;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.get_versions ().size () >
                 0 ? (double) rand () * list.get_versions ().back ().size /
                 RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (partiallypersistent, count, "insert", 0,
                       0, begin_operation, end_operation);
  }
  

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "partiallypersistent;modify;" << count << ";";
#ifdef RANDOMIZE
    cout << "random;";
#else
    cout << "sequential;";
#endif
    begin_operation = nano_time ();
    for (size_t j = 0; j < count; ++j) {
      partiallypersistent::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index =
        (double) rand () * list.get_versions ().back ().size / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      for (size_t k = 0; k < index; ++k) {
        if (node->next ()) {
          node = node->next ();
        } else {
          break;
        }
      }
      list.set_data (node, j);
    }
    end_operation = nano_time ();
    cout << (end_operation - begin_operation) << endl;
    if (store_results) {
      log_operation_to_db (partiallypersistent, count, "modify", 0,
                         0, begin_operation, end_operation);
    }
  }

  cout << "partiallypersistent;remove;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = nano_time ();
  for (size_t i = 0; i < count; ++i) {
    partiallypersistent::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index =
        (double) rand () * list.get_versions ().back ().size / RAND_MAX;
      for (size_t j = 0; j < index; ++j) {
        if (node->next ()) {
          node = node->next ();
        } else {
          break;
        }
      }
#endif
      list.remove (node);
    } else {
      cout << "List empty at version " << list.version << endl;
    }
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (partiallypersistent, count, "remove", 0,
                       0, begin_operation, end_operation);
  }

  cout << "partiallypersistent;access;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  size_t sum = 0UL;
  begin_operation = nano_time ();
  for (size_t i = 0UL; i < count; ++i) {
    size_t version_index =
      (double) rand () * list.get_versions ().size () / RAND_MAX;
    partiallypersistent::DoublyLinkedList::version_info_t version_info =
      list.get_versions ()[version_index];
    partiallypersistent::Node * n = version_info.head;
    size_t list_index = (double) rand () * version_info.size / RAND_MAX;
    for (size_t j = 0; j < list_index; ++j) {
      if (n->next ()) {
        n = n->next ();
      } else {
        break;
      }
    }
    size_t data = n->data_at (version_info.version);
    sum += data;
  }
  end_operation = nano_time ();
  cout << (end_operation - begin_operation) << endl;
  if (store_results) {
    log_operation_to_db (partiallypersistent, count, "access", 0,
                       0, begin_operation, end_operation);
  }
}

// void
// test_insert_modify_remove_ephemeral (size_t count) {
// #ifdef PROFILE_TIME
//   timespec begin;
//   clock_gettime(CLOCK_REALTIME, &begin);
//   begin.tv_sec * 1e9 + begin.tv_nsec;
// #endif
// 
//   ephemeral::DoublyLinkedList list;
// 
//   for (size_t i = 0; i < count; ++i) {
//     ephemeral::Node n;
//     n.data = i;
//     list.insert (n, list.size > 0 ? rand () * list.size / RAND_MAX : 0);
//   }
//   for (size_t i = 0; i < count; ++i) {
//     size_t index = rand () * list.size / RAND_MAX;
//     ephemeral::Node * node = list.head;
//     for (size_t j = 0; j < index; ++j) {
//       if (node->next) {
//         node = node->next;
//       } else {
//         break;
//       }
//     }
//     node->data = i;
//   }
//   for (size_t i = 0; i < count; ++i) {
//     size_t index = rand () * list.size / RAND_MAX;
//     ephemeral::Node * node = list.head;
//     for (size_t j = 0; j < index; ++j) {
//       node = node->next;
//     }
//     list.remove (*node);
//   }
// 
// #ifdef PROFILE_TIME
//   clock_t end = clock ();
// 
//   cout << "Ephemeral: " << count << " insertions and deletions: " <<
//     ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
// //   double vm, rss;
// //   process_mem_usage (vm, rss);
// //   cout << "VM: " << vm << "KB; RSS: " << rss << "KB" << endl;
// #endif
// 
// }

}

int
main (int argc, char **argv) {
  main_ns::start_time = main_ns::nano_time();
  
  using namespace main_ns;

  size_t count = 1000UL;
  size_t max_no_snapshots = 1200UL;
  size_t max_snapshot_dist = 100UL;
  bool store_results = false;

  main_ns::mode_t mode = main_ns::rollback_reorder;

  try {
    for (int i = 1; i < argc; ++i) {
      string arg = argv[i];
      if ("-c" == arg || "--count" == arg) {
        if (++i < argc) {
          count = atoi (argv[i]);
        } else {
          stringstream ss;
          ss << "No argument given to " << arg;
          throw ss.str ();
        }
      } else if ("-m" == arg || "--max-no-snapshots" == arg) {
        if (++i < argc) {
          max_no_snapshots = atoi (argv[i]);
        } else {
          stringstream ss;
          ss << "No argument given to " << arg;
          throw ss.str ();
        }
      } else if ("-d" == arg || "--max-snapshot-dist" == arg) {
        if (++i < argc) {
          max_snapshot_dist = atoi (argv[i]);
        } else {
          stringstream ss;
          ss << "No argument given to " << arg;
          throw ss.str ();
        }
      } else if ("-r" == arg || "--rollback-naive" == arg) {
        mode = main_ns::rollback_naive;
      } else if ("-o" == arg || "--rollback-naive" == arg) {
        mode = main_ns::rollback_reorder;
      } else if ("-p" == arg || "--partially-persistent" == arg) {
        mode = main_ns::partiallypersistent;
      } else if ("-s" == arg || "--store-results" == arg) {
        store_results = true;
      }
    }

    cout.precision (15);

    sqlite3 *db;
    char *zErrMsg;
    int rc;
    rc = sqlite3_open ("sqlite.db", &db);
    if (rc) {
      cerr << "Can't open database \"sqlite.db\": " << sqlite3_errmsg (db) <<
        endl;
      sqlite3_close (db);
      return 1;
    }

//     rc =
//       sqlite3_exec (db,
//                     "drop table if exists results", callback, 0, &zErrMsg);
//     if (rc != SQLITE_OK) {
//       cerr << "SQL error: " << zErrMsg << endl;
//       sqlite3_free (zErrMsg);
//     }
    rc = sqlite3_exec (db,
                       "create table if not exists results (id integer primary key autoincrement not null, start_time integer not null, version text not null, implementation text not null, count integer not null, max_no_snapshots integer, max_snapshot_dist integer, begin_time integer not null, end_time integer not null, operation text not null, duration integer not null)",
                       callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
      cerr << "SQL error: " << zErrMsg << endl;
      sqlite3_free (zErrMsg);
    }

    switch (mode) {
    case main_ns::rollback_naive:
      test_insert_modify_remove_rollback_naive (store_results, count, max_no_snapshots,
                                                max_snapshot_dist);
      break;
    case main_ns::rollback_reorder:
      test_insert_modify_remove_rollback_reorder (store_results, count, max_no_snapshots,
                                                max_snapshot_dist);
      break;
    case main_ns::partiallypersistent:
      test_insert_modify_remove_partiallypersistent (store_results, count);
      break;
    }

    sqlite3_close (db);
//   test_insert_modify_remove_ephemeral (count);
    return 0;
  }
  catch (string e) {
    cerr << "E: " << e << endl;
    return 1;
  }
}

// kate: indent-mode cstyle; indent-width 1; replace-tabs on; ;
