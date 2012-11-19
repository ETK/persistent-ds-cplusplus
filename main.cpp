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
#include "rollback_reorder_lazy/DoublyLinkedList.h"

// #undef RANDOMIZE
#define RANDOMIZE

using namespace std;

size_t current_op_no = 0;

namespace main_ns {
enum mode_t {
  rollback_naive,
  rollback_reorder,
  rollback_reorder_lazy,
  partiallypersistent
};

enum operation_type_t {
  insert,
  modify,
  remove,
  access
};

uint64_t start_time;
size_t count = 1000UL;
size_t max_no_snapshots = 1200UL;
size_t max_snapshot_dist = 100UL;
bool store_results = false;
bool randomize_operations = false;

string
mode_to_string (mode_t mode) {
  switch (mode) {
  case rollback_naive:
    return "rollback_naive";
  case rollback_reorder:
    return "rollback_reorder";
  case rollback_reorder_lazy:
    return "rollback_reorder_lazy";
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
print_all_versions (rollback_reorder_lazy::DoublyLinkedList & list) {
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
test_insert_modify_remove_rollback_naive () {
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
test_insert_modify_remove_rollback_reorder () {
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
test_insert_modify_remove_rollback_reorder_lazy () {
  double begin_operation, end_operation;

  rollback_reorder_lazy::DoublyLinkedList list (max_no_snapshots, max_snapshot_dist);

  cout << "rollback_reorder_lazy;insert;" << count << ";";
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
    log_operation_to_db (rollback_reorder_lazy, count, "insert", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "rollback_reorder_lazy;modify;" << count << ";";
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
      log_operation_to_db (rollback_reorder_lazy, count, "modify", max_no_snapshots,
                           max_snapshot_dist, begin_operation, end_operation);
    }
  }

  cout << "rollback_reorder_lazy;remove;" << count << ";";
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
    log_operation_to_db (rollback_reorder_lazy, count, "remove", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }

  cout << "rollback_reorder_lazy;access;" << count << ";";
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
    log_operation_to_db (rollback_reorder_lazy, count, "access", max_no_snapshots,
                       max_snapshot_dist, begin_operation, end_operation);
  }
}

void
test_insert_modify_remove_partiallypersistent () {
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

}

double rand01() {
  return (double) rand()/RAND_MAX;
}

#include <csignal>
#include <cstring>
#include <iomanip>
#include <cerrno>

static void hdl (int sig, siginfo_t *siginfo, void *context) {
  if (sig == SIGUSR1) {
    cout.setf(ios::showpoint);
    cout << "Progress: " << current_op_no << " / " << main_ns::count << " (" << setprecision(2) << (100.0 * current_op_no)/main_ns::count << "%)" << endl;
  }
}

int
main (int argc, char **argv) {
  main_ns::start_time = main_ns::nano_time();

  using namespace main_ns;

  main_ns::mode_t mode = main_ns::rollback_reorder_lazy;

  struct sigaction act;
  memset (&act, '\0', sizeof(act));
  act.sa_sigaction = &hdl;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGUSR1, &act, NULL) < 0) {
    perror("sigaction");
    return 1;
  }

  try {
    for (int i = 1; i < argc; ++i) {
      string arg = argv[i];
      if ("-c" == arg || "--count" == arg) {
        if (++i < argc) {
          main_ns::count = atoi (argv[i]);
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
      } else if ("-o" == arg || "--rollback-reorder" == arg) {
        mode = main_ns::rollback_reorder;
      } else if ("-l" == arg || "--rollback-reorder-lazy" == arg) {
        mode = main_ns::rollback_reorder_lazy;
      } else if ("-p" == arg || "--partially-persistent" == arg) {
        mode = main_ns::partiallypersistent;
      } else if ("-s" == arg || "--store-results" == arg) {
        store_results = true;
      } else if ("--randomize-operations" == arg) {
        randomize_operations = true;
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

    if (randomize_operations) {
      srand(0);
      AbstractDoublyLinkedList* list = 0x0;
      switch (mode) {
        case main_ns::partiallypersistent:
          list = new partiallypersistent::DoublyLinkedList();
          break;
        case main_ns::rollback_reorder:
          list = new rollback_reorder::DoublyLinkedList(max_no_snapshots, max_snapshot_dist);
          break;
        case main_ns::rollback_reorder_lazy:
          list = new rollback_reorder_lazy::DoublyLinkedList(max_no_snapshots, max_snapshot_dist);
          break;
      }
      size_t insert_count = 0;
      long long insert_duration = 0;
      size_t modify_count = 0;
      long long modify_duration = 0;
      size_t remove_count = 0;
      long long remove_duration = 0;
      size_t access_count = 0;
      long long access_duration = 0;

      double begin_operation;
      for (size_t i = 0; i < main_ns::count; ++i) {
        double r = rand01();
        size_t list_size = list->a_size();
        size_t index = (size_t) (rand01() * list_size);
        size_t version = list->a_num_versions();
        if (r < 0.25 && list_size > 0) {
          ++remove_count;
          begin_operation = nano_time();
          list->a_remove(index);
          remove_duration += (long long) (nano_time() - begin_operation);
        } else if (r < 0.25 + 0.25 && list_size > 0) {
          ++modify_count;
          begin_operation = nano_time();
          list->a_modify(index, i);
          modify_duration += (long long) (nano_time() - begin_operation);
        } else if (r < 0.25 + 0.25 + 0.25 && list->a_size_at(version = rand01() * list->a_num_versions()) > 0) {
          ++access_count;
          begin_operation = nano_time();
          index = rand01() * list->a_size_at(version);
          access_duration += (long long) (nano_time() - begin_operation);
        } else {
          ++insert_count;
          begin_operation = nano_time();
          list->a_insert(index, i);
          insert_duration += (long long) (nano_time() - begin_operation);
        }
        ++current_op_no;
      }

      if (store_results) {
        log_operation_to_db(mode, insert_count, "insert", max_no_snapshots, max_snapshot_dist, 0, insert_duration);
        log_operation_to_db(mode, modify_count, "modify", max_no_snapshots, max_snapshot_dist, 0, modify_duration);
        log_operation_to_db(mode, remove_count, "remove", max_no_snapshots, max_snapshot_dist, 0, remove_duration);
        log_operation_to_db(mode, access_count, "access", max_no_snapshots, max_snapshot_dist, 0, access_duration);
      }
    } else {
      switch (mode) {
      case main_ns::rollback_naive:
        test_insert_modify_remove_rollback_naive ();
        break;
      case main_ns::rollback_reorder:
        test_insert_modify_remove_rollback_reorder ();
        break;
      case main_ns::rollback_reorder_lazy:
        test_insert_modify_remove_rollback_reorder_lazy ();
        break;
      case main_ns::partiallypersistent:
        test_insert_modify_remove_partiallypersistent ();
        break;
      }
    }

    sqlite3_close (db);
    return 0;
  }
  catch (string e) {
    cerr << "E: " << e << endl;
    return 1;
  }
}

// kate: indent-mode cstyle; indent-width 1; replace-tabs on; ;
