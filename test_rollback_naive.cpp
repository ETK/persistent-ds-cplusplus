#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>

#include "rollback_naive/DoublyLinkedList.h"

using namespace std;

void test_insert_print (size_t count) {
  rollback_naive::DoublyLinkedList list;
  for (size_t i = 0; i < count; ++i) {
    list.insert (i + 1, i);
  }

  vector<size_t> versions;
  for (size_t i = 0; i <= count; ++i) {
    versions.push_back(i);
  }

  std::random_shuffle(versions.begin(), versions.end());

  for (vector<size_t>::iterator iter = versions.begin(); iter != versions.end(); ++iter) {
    cout << "List at version ";
    printf("%2ld", *iter);
    cout << ": ";
    list.print_at_version(*iter);
  }
}

int main (int argc, char **argv) {
  test_insert_print (20);
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
