#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "DoublyLinkedList.h"

#define PROFILE_TIME 1

using namespace std;

void test_abcd (DoublyLinkedList & list) {
  Node d, c, b, a;

  d.data_val = 4;
  c.data_val = 3;
  b.data_val = 2;
  a.data_val = 1;

  list.insert (c);              // v1
  list.insert (b);              // v2
  list.insert (a);              // v3

  list.set_field (*(list.head ()->next ()), DATA, (void *) (20ul));     // v4
  list.set_field (*(list.head ()->next ()), DATA, (void *) (21ul));     // v5
  list.set_field (*(list.head ()->next ()), DATA, (void *) (22ul));     // v6
  list.set_field (*(list.head ()->next ()), DATA, (void *) (23ul));     // v7
  list.set_field (*(list.head ()->next ()), DATA, (void *) (24ul));     // v8
  list.set_field (*(list.head ()->next ()), DATA, (void *) (200ul));    // v9

  list.insert (d, 1);           // v10

  list.set_field (*(list.head ()), DATA, (void *) (10ul));      // v11
  list.set_field (*(list.head ()), DATA, (void *) (11ul));      // v12
  list.set_field (*(list.head ()), DATA, (void *) (12ul));      // v13
  list.set_field (*(list.head ()), DATA, (void *) (13ul));      // v14
  list.set_field (*(list.head ()), DATA, (void *) (14ul));      // v15
  list.set_field (*(list.head ()->next ()), DATA, (void *) (40ul));     // v16
  list.set_field (*(list.head ()->next ()), DATA, (void *) (41ul));     // v17
  list.set_field (*(list.head ()->next ()), DATA, (void *) (42ul));     // v18
  list.set_field (*(list.head ()->next ()), DATA, (void *) (43ul));     // v19
  list.set_field (*(list.head ()->next ()), DATA, (void *) (44ul));     // v20
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2000ul));  // v21
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2001ul));  // v22
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2002ul));  // v23
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2003ul));  // v24
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2004ul));  // v25
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (30ul));   // v26
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (31ul));   // v27
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (32ul));   // v28
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (33ul));   // v29
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (34ul));   // v30

  list.remove (*list.head ());  // v31
  list.remove (*list.head ());  // v32
  list.remove (*list.head ());  // v33
  list.remove (*list.head ());  // v34
}

void print_all_versions (DoublyLinkedList & list) {
  for (vector < pair < size_t, Node * > >::size_type i = 0;
       i < list.get_versions ().size (); ++i) {
    cout << "List at version " << list.get_versions ()[i].version
      << " (size " << list.get_versions ()[i].size << "): ";
    list.print_at_version (list.get_versions ()[i].version);
  }
}

void test_insert_modify_remove (size_t count) {
#if PROFILE_TIME
  clock_t begin = clock ();
#endif

  DoublyLinkedList list;

  for (size_t i = 0; i < count; ++i) {
    Node n;
    list.insert (n, list.get_versions().size() > 0 ? rand() * list.get_versions().back().size / RAND_MAX : 0);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand() * list.get_versions().back().size / RAND_MAX;
    Node* node = list.head();
    for (size_t j = 0; j < index; ++j) {
      node = node->next();
    }
    list.set_field (*node, DATA, (void*) i);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand() * list.get_versions().back().size / RAND_MAX;
    Node* node = list.head();
    for (size_t j = 0; j < index; ++j) {
      node = node->next();
    }
    list.remove (*node);
  }

#if PROFILE_TIME
  clock_t end = clock ();

  cout << "Time elapsed for " << count << " insertions and deletions: " <<
    ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
#endif
}

int main (int argc, char **argv) {
//   test_insert_remove (1);
//   test_insert_remove (10);
//   test_insert_remove (100);
//   test_insert_remove (1000);
  if (argc == 2) {
    test_insert_modify_remove (atoi(argv[1]));
  } else {
    test_insert_modify_remove (10000);
  }
//   test_insert_remove (100000);
//   test_insert_remove (1000000);
//   test_insert_remove (4000000);

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
