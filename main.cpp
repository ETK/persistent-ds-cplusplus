#include "main_ns.h"

#include <csignal>
#include <cerrno>
#include <iomanip>

#include <cstring>

#include "rollback/AbstractRollbackDoublyLinkedList.h"
#include "rollback/blackbox/DoublyLinkedList.h"
#include "rollback/eliminate_reorder/DoublyLinkedList.h"
#include "rollback/reorder/DoublyLinkedList.h"
#include "node_copying/DoublyLinkedList.h"

using namespace std;


double
rand01 ()
{
  return (double) rand () / RAND_MAX;
}

static void
hdl (int sig, siginfo_t* siginfo, void* context)
{
  if (sig == SIGUSR1) {
    cout.setf (ios::showpoint);
    cout << "Progress: " << setprecision (15) << main_ns::current_op_no <<
         " / " << main_ns::count << " (" << setprecision (4)
         << (100.0 * main_ns::current_op_no) / main_ns::count << "%)" << endl;
  }
}

int
main (int argc, char** argv)
{
#ifndef MEASURE_SPACE
  main_ns::start_time = main_ns::nano_time ();
#endif

  using namespace main_ns;

  main_ns::mode_t mode = main_ns::eliminate_reorder;
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
      } else if ("-r" == arg || "--rollback-blackbox" == arg) {
        mode = main_ns::blackbox;
      } else if ("-l" == arg || "--rollback-eliminate-reorder" == arg) {
        mode = main_ns::eliminate_reorder;
      } else if ("-o" == arg || "--rollback-reorder" == arg) {
        mode = main_ns::reorder;
      } else if ("-p" == arg || "--node-copying" == arg) {
        mode = main_ns::node_copying;
      } else if ("-s" == arg || "--store-results" == arg) {
        store_results = true;
      } else if ("--randomize-operations" == arg) {
        randomize_operations = true;
      } else if ("-h" == arg || "--head-only" == arg) {
        only_measure_time_to_head = true;
      }
    }

    cout.precision (15);

#ifndef MEASURE_SPACE
    sqlite3* db;
    char* zErrMsg;
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
#endif

    srand (0);
    AbstractDoublyLinkedList* list = 0x0;
    switch (mode) {
    case main_ns::node_copying:
      list = new node_copying::DoublyLinkedList ();
      break;
    case main_ns::blackbox:
      list =
        new rollback::blackbox::DoublyLinkedList (max_no_snapshots,
            max_snapshot_dist);
      break;
    case main_ns::reorder:
      list = new rollback::reorder::DoublyLinkedList (max_no_snapshots,
          max_snapshot_dist);
      break;
    case main_ns::eliminate_reorder:
      list =
        new rollback::eliminate_reorder::DoublyLinkedList (max_no_snapshots,
            max_snapshot_dist);
      break;
    }
#ifndef MEASURE_SPACE
    size_t insert_count = 0;
    size_t modify_count = 0;
    size_t remove_count = 0;
    size_t access_count = 0;
    long long insert_duration = 0;
    long long modify_duration = 0;
    long long remove_duration = 0;
    long long access_duration = 0;
#endif

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
        double progress = ( (double) i) / main_ns::count;
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
#ifndef MEASURE_SPACE
      begin_operation = nano_time ();
#endif
      size_t index = 0;
      size_t version;
      if (op == main_ns::access
          && list->a_size_at (version =
                                rand01 () * list->a_num_versions ()) > 0) {
#ifndef MEASURE_SPACE
        ++access_count;
#endif
        if (!only_measure_time_to_head) {
          index = rand01 () * list->a_size_at (version);
        }
        list->a_access (version, index);
#ifndef MEASURE_SPACE
        access_duration += (long long) (nano_time () - begin_operation);
#endif
      } else {
        version = list->a_num_versions ();
        size_t list_size = list->a_size ();
        if (!only_measure_time_to_head) {
          index = (size_t) (rand01 () * list_size);
        }
        if (op == main_ns::remove && list_size > 0) {
          list->a_remove (index);
#ifndef MEASURE_SPACE
          ++remove_count;
          remove_duration += (long long) (nano_time () - begin_operation);
#endif
        } else if (op == modify && list_size > 0) {
          list->a_modify (index, i);
#ifndef MEASURE_SPACE
          ++modify_count;
          modify_duration += (long long) (nano_time () - begin_operation);
#endif
        } else {
          list->a_insert (index, i);
#ifndef MEASURE_SPACE
          ++insert_count;
          insert_duration += (long long) (nano_time () - begin_operation);
#endif
        }
      }
      ++current_op_no;
    }

#ifdef MEASURE_SPACE
    cout << main_ns::count << ";" << mode_to_string (mode) << ";" <<
         (randomize_operations ? "randomized" : "sequential") << ";";
    switch (mode) {
    case main_ns::node_copying :
      cout << ( (node_copying::DoublyLinkedList*) list)->space + sizeof (*list) + sizeof (node_copying::DoublyLinkedList::version_info_t) * list->a_num_versions();
      break;
    case main_ns::eliminate_reorder:
    case main_ns::reorder:
    case main_ns::blackbox: {
      rollback::AbstractRollbackDoublyLinkedList* rlist = ( (rollback::AbstractRollbackDoublyLinkedList*) list);
      size_t space = sizeof (*rlist);
      const std::vector < std::pair < std::size_t,
            ephemeral::DoublyLinkedList* >> & snapshots = rlist->get_snapshots();
      for (std::vector < std::pair < std::size_t,
           ephemeral::DoublyLinkedList* >>::const_iterator iter = snapshots.cbegin(); iter != snapshots.cend(); ++iter) {

        pair < size_t, ephemeral::DoublyLinkedList* > snaphot = *iter;
        space += snaphot.second->size * sizeof (ephemeral::Node);
        space += sizeof (*snaphot.second);
      }
      space += rlist->num_records() * sizeof (rollback::record_t);
      cout << space;
      break;
    }
    }
    cout << "" << endl;
#endif

#ifndef MEASURE_SPACE
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
#endif

    return 0;
  } catch (string e) {
    cerr << "E: " << e << endl;
    return 1;
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;



