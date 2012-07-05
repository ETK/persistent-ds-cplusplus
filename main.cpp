#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>

#include "ephemeral/DoublyLinkedList.h"
#include "partiallypersistent/DoublyLinkedList.h"
#include "rollback_naive/DoublyLinkedList.h"

#undef PROFILE_TIME
// #undef RANDOMIZE
#define RANDOMIZE

using namespace std;

void dump_list_dot_graph (partiallypersistent::DoublyLinkedList & list) {
  ofstream file;
  file.open ("graph", ios_base::out);
  list.print_dot_graph (list.version, file);
  file.close ();
}

void process_mem_usage (double &vm_usage, double &resident_set) {
  using std::ios_base;
  using std::ifstream;
  using std::string;

  vm_usage = 0.0;
  resident_set = 0.0;

  // 'file' stat seems to give the most reliable results
  //
  ifstream stat_stream ("/proc/self/stat", ios_base::in);

  // dummy vars for leading entries in stat that we don't care about
  //
  string pid, comm, state, ppid, pgrp, session, tty_nr;
  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice;
  string O, itrealvalue, starttime;

  // the two fields we want
  //
  unsigned long vsize;
  long rss;

  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime >> cutime >> cstime >> priority >> nice >> O >> itrealvalue >> starttime >> vsize >> rss;      // don't care about the rest

  stat_stream.close ();

  long page_size_kb = sysconf (_SC_PAGE_SIZE) / 1024;   // in case x86-64 is configured to use 2MB pages
  vm_usage = vsize / 1024.0;
  resident_set = rss * page_size_kb;
}

void print_all_versions (partiallypersistent::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       0; i < list.get_versions ().size (); ++i) {
    cout << "List at version " << list.get_versions()[i].version << " (size " << list.get_versions()[i].size << "): ";
    size_t printed_size =
      list.print_at_version (list.get_versions()[i].version);
    cout << "printed size: " << printed_size << endl;
  }
}

void print_all_versions (rollback_naive::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       0; i < list.num_records (); ++i) {
    cout << "List at version " << i << " (size " << list.
      size_at (i) << "): ";
    size_t printed_size = list.print_at_version (i);
    cout << "printed size: " << printed_size << endl;
  }
}

void test_insert_modify_remove_rollback_naive (size_t count) {
#ifdef PROFILE_TIME
  clock_t begin = clock ();
#endif

  rollback_naive::DoublyLinkedList list;

  cout << "inserting " << count << " nodes..." << endl;
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i, list.size() > 0 ? rand () * list.size() / RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "modifying data for " << count << " nodes, iteration " << i +
      1 << "..." << endl;
    for (size_t j = 0; j < count; ++j) {
      ephemeral::Node * node = list.head();
#ifdef RANDOMIZE
      size_t index = rand () * list.size () / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      list.modify_data (index, j);
    }
  }

  cout << "removing " << count << " nodes..." << endl;
  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index = rand () * list.size() / RAND_MAX;
#endif
      list.remove (index);
    } else {
      cout << "List empty at version " << list.num_records() << endl;
    }
  }

#ifdef PROFILE_TIME
  clock_t end = clock ();

  cout << "Partially persistent: " << count << " insertions and deletions: "
    << ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
  double vm, rss;
  process_mem_usage (vm, rss);
  cout << "VM: " << vm << "KB; RSS: " << rss << "KB" << endl;
#endif
}
void test_insert_modify_remove_partiallypersistent (size_t count) {
#ifdef PROFILE_TIME
  clock_t begin = clock ();
#endif

  partiallypersistent::DoublyLinkedList list;

  cout << "inserting " << count << " nodes..." << endl;
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.get_versions ().size () >
                 0 ? rand () * list.get_versions ().back ().size /
                 RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "modifying data for " << count << " nodes, iteration " << i +
      1 << "..." << endl;
    for (size_t j = 0; j < count; ++j) {
      partiallypersistent::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index = rand () * list.get_versions ().back ().size / RAND_MAX;
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
  }

  cout << "removing " << count << " nodes..." << endl;
  for (size_t i = 0; i < count; ++i) {
    partiallypersistent::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index = rand () * list.get_versions ().back ().size / RAND_MAX;
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

#ifdef PROFILE_TIME
  clock_t end = clock ();

  cout << "Partially persistent: " << count << " insertions and deletions: "
    << ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
  double vm, rss;
  process_mem_usage (vm, rss);
  cout << "VM: " << vm << "KB; RSS: " << rss << "KB" << endl;
#endif
}

void test_insert_modify_remove_ephemeral (size_t count) {
#ifdef PROFILE_TIME
  clock_t begin = clock ();
#endif

  ephemeral::DoublyLinkedList list;

  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node n;
    n.data = i;
    list.insert (n, list.size > 0 ? rand () * list.size / RAND_MAX : 0);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.size / RAND_MAX;
    ephemeral::Node * node = list.head;
    for (size_t j = 0; j < index; ++j) {
      if (node->next) {
        node = node->next;
      } else {
        break;
      }
    }
    node->data = i;
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.size / RAND_MAX;
    ephemeral::Node * node = list.head;
    for (size_t j = 0; j < index; ++j) {
      node = node->next;
    }
    list.remove (*node);
  }

#ifdef PROFILE_TIME
  clock_t end = clock ();

  cout << "Ephemeral: " << count << " insertions and deletions: " <<
    ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
  double vm, rss;
  process_mem_usage (vm, rss);
  cout << "VM: " << vm << "KB; RSS: " << rss << "KB" << endl;
#endif

}

int main (int argc, char **argv) {

  int count = 100;
  if (argc == 2) {
    count = atoi (argv[1]);
  }

  test_insert_modify_remove_partiallypersistent (count);
  test_insert_modify_remove_rollback_naive (count);
  test_insert_modify_remove_ephemeral (count);

  return 0;
}

// kate: indent-mode cstyle; indent-width 1; replace-tabs on; ;
