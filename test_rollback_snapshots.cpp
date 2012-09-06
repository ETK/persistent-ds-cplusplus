#include <iostream>
#include <vector>
#include <algorithm>

#define DEBUG_SNAPSHOT_FEATURE

#include "rollback_naive/DoublyLinkedList.h"
#include "rollback_reorder/DoublyLinkedList.h"

using namespace std;

void test_rollback_snapshots () {
  rollback_naive::DoublyLinkedList list;
  vector<size_t> versions = vector<size_t>();
//   versions.push_back(0);
  for (size_t i = 0; i < 50; ++i) {
    list.insert(i, i);
    versions.push_back(i);
  }

  random_shuffle(versions.begin(), versions.end());

  for (vector<size_t>::iterator iter = versions.begin(); iter != versions.end(); ++iter) {
    cout << "Trying to print version " << *iter << ":" << endl;
    list.print_at_version(*iter);
  }
}

void test_rollback_reorder() {
  rollback_reorder::DoublyLinkedList list(5000, 5000);
  list.test_reorder ();
}

int main (int argc, char **argv) {
//   test_rollback_snapshots();
  test_rollback_reorder();
  return 0;
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on;
