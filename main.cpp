#include <iostream>
#include <vector>
#include "DoublyLinkedList.h"

using namespace std;

int main (int argc, char **argv) {

  DoublyLinkedList list;
  Node d, c, b, a;

  d.data = 4;
  c.data = 3;
  b.data = 2;
  a.data = 1;

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

  for (vector < pair < size_t, Node * >>::size_type i = 0;
       i < list.get_versions ().size (); ++i) {
    cout << "List at version " << list.get_versions ()[i].version << " (size " << list.get_versions()[i].size << "): ";
    list.print_at_version (list.get_versions ()[i].version);
  }

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
