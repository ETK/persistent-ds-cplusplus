#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "ephemeral/DoublyLinkedList.h"
#include "partiallypersistent/DoublyLinkedList.h"

#define PROFILE_TIME

using namespace std;

void test_abcd (partiallypersistent::DoublyLinkedList & list) {
  partiallypersistent::Node d, c, b, a;

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

  cout << "Partially persistent: " << count << " insertions and deletions: " <<
    ((end - begin) * 1000.0 / CLOCKS_PER_SEC) << "ms" << endl;
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
#endif

}

int main (int argc, char **argv) {

  int count = 100000;
  if (argc == 2) {
    count = atoi (argv[1]);
  }

  test_insert_modify_remove_partiallypersistent (count);
  test_insert_modify_remove_ephemeral (count);

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
