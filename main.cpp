#include "main_ns.h"

#include <csignal>
#include <cerrno>
#include <iomanip>

#include <cstring>

#include "rollback_lazy/DoublyLinkedList.h"
#include "partiallypersistent/DoublyLinkedList.h"
#include "rollback_reorder_lazy/DoublyLinkedList.h"

using namespace std;


double
rand01 () {
  return (double) rand () / RAND_MAX;
}

static void
hdl (int sig, siginfo_t * siginfo, void *context) {
  if (sig == SIGUSR1) {
    cout.setf (ios::showpoint);
    cout << "Progress: " << setprecision (15) << main_ns::current_op_no <<
      " / " << main_ns::count << " (" << setprecision (4)
      << (100.0 * main_ns::current_op_no) / main_ns::count << "%)" << endl;
  }
}

int
main (int argc, char **argv) {
  main_ns::start_time = main_ns::nano_time ();

  using namespace main_ns;

  main_ns::mode_t mode = main_ns::rollback_reorder_lazy;
  bool only_measure_time_to_head = false;

  double p_remove = 0.25;
  double p_modify = 0.25;
  double p_access = 0.25;
  double p_insert = 0.25;

  struct sigaction act;
  memset (&act, '\0', sizeof (act));
  act.sa_sigaction = &hdl;
  act.sa_flags = SA_SIGINFO;
  if (sigaction (SIGUSR1, &act, NULL) < 0) {
    perror ("sigaction");
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
      } else if ("-r" == arg || "--rollback-lazy" == arg) {
        mode = main_ns::rollback_lazy;
      } else if ("-l" == arg || "--rollback-reorder-lazy" == arg) {
        mode = main_ns::rollback_reorder_lazy;
      } else if ("-p" == arg || "--partially-persistent" == arg) {
        mode = main_ns::partiallypersistent;
      } else if ("-s" == arg || "--store-results" == arg) {
        store_results = true;
      } else if ("--randomize-operations" == arg) {
        randomize_operations = true;
      } else if ("-h" == arg || "--head-only" == arg) {
        only_measure_time_to_head = true;
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

    srand (0);
    AbstractDoublyLinkedList *list = 0x0;
    switch (mode) {
    case main_ns::partiallypersistent:
      list = new partiallypersistent::DoublyLinkedList ();
      break;
    case main_ns::rollback_lazy:
      list =
        new rollback_lazy::DoublyLinkedList (max_no_snapshots,
                                             max_snapshot_dist);
      break;
    case main_ns::rollback_reorder_lazy:
      list =
        new rollback_reorder_lazy::DoublyLinkedList (max_no_snapshots,
                                                     max_snapshot_dist);
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

    double p_sum = p_remove + p_modify + p_access + p_insert;
    p_remove /= p_sum;
    p_modify /= p_sum;
    p_access /= p_sum;
    p_insert /= p_sum;

    double begin_operation;
    for (size_t i = 0; i < main_ns::count; ++i) {
      operation_type_t op;
      if (randomize_operations) {
        double r = rand01 ();
        if (r < p_remove) {
          op = main_ns::remove;
        } else if (r < p_remove + p_modify) {
          op = modify;
        } else if (r < p_remove + p_modify + p_access) {
          op = main_ns::access;
        } else {
          op = insert;
        }
      } else {
        double progress = ((double) i) / main_ns::count;
        if (progress < p_insert) {
          op = insert;
        } else if (progress < p_insert + p_modify) {
          op = modify;
        } else if (progress < p_insert + p_modify + p_remove) {
          op = main_ns::remove;
        } else {
          op = main_ns::access;
        }
      }
      begin_operation = nano_time ();
      size_t index = 0;
      size_t version;
      if (op == main_ns::access
          && list->a_size_at (version =
                              rand01 () * list->a_num_versions ()) > 0) {
        ++access_count;
        if (!only_measure_time_to_head) {
          index = rand01 () * list->a_size_at (version);
        }
        list->a_access(version, index);
        access_duration += (long long) (nano_time () - begin_operation);
      } else {
        version = list->a_num_versions ();
        size_t list_size = list->a_size ();
        if (!only_measure_time_to_head) {
          index = (size_t) (rand01 () * list_size);
        }
        if (op == main_ns::remove && list_size > 0) {
          ++remove_count;
          list->a_remove (index);
          remove_duration += (long long) (nano_time () - begin_operation);
        } else if (op == modify && list_size > 0) {
          ++modify_count;
          list->a_modify (index, i);
          modify_duration += (long long) (nano_time () - begin_operation);
        } else {
          ++insert_count;
          list->a_insert (index, i);
          insert_duration += (long long) (nano_time () - begin_operation);
        }
      }
      ++current_op_no;
    }

    if (store_results) {
      log_operation_to_db (mode, insert_count, "insert", max_no_snapshots,
                           max_snapshot_dist, 0, insert_duration);
      log_operation_to_db (mode, modify_count, "modify", max_no_snapshots,
                           max_snapshot_dist, 0, modify_duration);
      log_operation_to_db (mode, remove_count, "remove", max_no_snapshots,
                           max_snapshot_dist, 0, remove_duration);
      log_operation_to_db (mode, access_count, "access", max_no_snapshots,
                           max_snapshot_dist, 0, access_duration);
    }

    sqlite3_close (db);
    return 0;
  }
  catch (string e) {
    cerr << "E: " << e << endl;
    return 1;
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
