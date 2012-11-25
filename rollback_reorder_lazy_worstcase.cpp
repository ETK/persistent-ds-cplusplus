#include "main_ns.h"

#include <csignal>
#include <cerrno>
#include <iomanip>
#include <cstring>

#include "partiallypersistent/DoublyLinkedList.h"
#include "rollback_lazy/DoublyLinkedList.h"
#include "rollback_reorder_lazy/DoublyLinkedList.h"

using namespace std;

size_t no_inserts = 100000;
size_t no_accesses = 100000;

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
  using namespace main_ns;

  struct sigaction act;
  memset (&act, '\0', sizeof(act));
  act.sa_sigaction = &hdl;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGUSR1, &act, NULL) < 0) {
    perror("sigaction");
    return 1;
  }
  
  main_ns::count = no_inserts + no_accesses;
  main_ns::current_op_no = 0;
  main_ns::mode_t mode = main_ns::rollback_reorder_lazy;

  try {
    for (int i = 1; i < argc; ++i) {
      string arg = argv[i];
      if ("-r" == arg || "--rollback-lazy" == arg) {
        mode = main_ns::rollback_lazy;
      } else if ("-l" == arg || "--rollback-reorder-lazy" == arg) {
        mode = main_ns::rollback_reorder_lazy;
      } else if ("-p" == arg || "--partially-persistent" == arg) {
        mode = main_ns::partiallypersistent;
      } else if ("-s" == arg || "--store-results" == arg) {
        store_results = true;
      } else {
        cerr << "Unrecognized argument \"" << arg << "\".";
        exit (1);
      }
    }

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

    start_time = nano_time();

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
    for (size_t i = 0; i < no_inserts; ++i) {
      double begin_operation = nano_time();
      list->a_insert (0, 0);
      insert_duration += (long long) (nano_time() - begin_operation);
      ++insert_count;
      ++current_op_no;
    }

    size_t num_versions = list->a_num_versions ();
    size_t v_base =
      max_snapshot_dist * (num_versions / max_snapshot_dist) -
      2 * max_snapshot_dist;
    size_t v1 = v_base - (max_snapshot_dist / 2) - 1;
    size_t v2 = v_base + ((max_snapshot_dist + 1) / 2) - 2;

    size_t access_count = 0;
    long long access_duration = 0;
    for (size_t i = 0; i < no_accesses; ++i) {
      double begin_operation = nano_time();
      if (i % 2 == 0) {
        list->a_access (v1, 0);
      } else {
        list->a_access (v2, 0);
      }
      access_duration += (long long) (nano_time() - begin_operation);
      ++access_count;
      ++current_op_no;
    }

    if (store_results) {
      log_operation_to_db(mode, insert_count, "insert", max_no_snapshots, max_snapshot_dist, 0, insert_duration);
      log_operation_to_db(mode, access_count, "access", max_no_snapshots, max_snapshot_dist, 0, access_duration);
    } else {
      cout << mode_to_string(mode) << ", " << insert_count << ", insert, " << max_no_snapshots << ", " << max_snapshot_dist << ", 0, " << insert_duration << endl;
      cout << mode_to_string(mode) << ", " << access_count << ", access, " << max_no_snapshots << ", " << max_snapshot_dist << ", 0, " << access_duration << endl;
    }

  }
  catch (string e) {
    cerr << "E: " << e << endl;
    return 1;
  }
  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
