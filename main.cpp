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
  list.set_field (*(list.head ()->next ()), DATA, (void *) (21ul));     // v4
  list.set_field (*(list.head ()->next ()), DATA, (void *) (22ul));     // v4
  list.set_field (*(list.head ()->next ()), DATA, (void *) (23ul));     // v4
  list.set_field (*(list.head ()->next ()), DATA, (void *) (24ul));     // v4
  list.set_field (*(list.head ()->next ()), DATA, (void *) (200ul));    // v5

  list.insert (d);              // v6

  list.set_field (*(list.head ()->next ()), DATA, (void *) (10ul));     // v7
  list.set_field (*(list.head ()->next ()), DATA, (void *) (11ul));     // v7
  list.set_field (*(list.head ()->next ()), DATA, (void *) (12ul));     // v7
  list.set_field (*(list.head ()->next ()), DATA, (void *) (13ul));     // v7
  list.set_field (*(list.head ()->next ()), DATA, (void *) (14ul));     // v7
  list.set_field (*(list.head ()), DATA, (void *) (40ul));      // v7
  list.set_field (*(list.head ()), DATA, (void *) (41ul));      // v7
  list.set_field (*(list.head ()), DATA, (void *) (42ul));      // v7
  list.set_field (*(list.head ()), DATA, (void *) (43ul));      // v7
  list.set_field (*(list.head ()), DATA, (void *) (44ul));      // v7
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2000ul));  // v8
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2001ul));  // v8
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2002ul));  // v8
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2003ul));  // v8
  list.set_field (*(list.head ()->next ()->next ()), DATA, (void *) (2004ul));  // v8
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (30ul));   // v9
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (31ul));   // v9
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (32ul));   // v9
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (33ul));   // v9
  list.set_field (*(list.head ()->next ()->next ()->next ()), DATA, (void *) (34ul));   // v9

  for (vector < pair < size_t, Node * >>::size_type i = 0;
       i < list.get_heads ().size (); ++i) {
    cout << "List at version " << list.get_heads ()[i].first << ": ";
    list.print_at_version (list.get_heads ()[i].first);
  }

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
