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

#define PROFILE_TIME

using namespace std;

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
    cout << "List at version " << list.
      get_versions ()[i].version << " (size " << list.get_versions ()[i].
      size << "): ";
    list.print_at_version (list.get_versions ()[i].version);
  }
}

void test_insert_modify_remove_partiallypersistent (size_t count) {
#ifdef PROFILE_TIME
  clock_t begin = clock ();
#endif

  partiallypersistent::DoublyLinkedList list;

  for (size_t i = 0; i < count; ++i) {
    partiallypersistent::Node n;
    list.insert (n,
                 list.get_versions ().size () >
                 0 ? rand () * list.get_versions ().back ().size /
                 RAND_MAX : 0);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.get_versions ().back ().size / RAND_MAX;
    partiallypersistent::Node * node = list.head ();
    for (size_t j = 0; j < index; ++j) {
      node = node->next ();
    }
    list.set_field (*node, DATA, (void *) i);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.get_versions ().back ().size / RAND_MAX;
    partiallypersistent::Node * node = list.head ();
    for (size_t j = 0; j < index; ++j) {
      node = node->next ();
    }
    list.remove (*node);
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
    list.insert (n, list.size > 0 ? rand () * list.size / RAND_MAX : 0);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.size / RAND_MAX;
    ephemeral::Node * node = list.head;
    for (size_t j = 0; j < index; ++j) {
      node = node->next;
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

  int count = 10000;
  if (argc == 2) {
    count = atoi (argv[1]);
  }

  test_insert_modify_remove_partiallypersistent (count);
  test_insert_modify_remove_ephemeral (count);

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
