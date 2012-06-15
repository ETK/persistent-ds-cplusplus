#include <iostream>
#include <vector>
#include "DoublyLinkedList.h"

using namespace std;

int
main (int argc, char **argv) {

  DoublyLinkedList list;
  Node d, c, b, a;

  d.data = 4;
  c.data = 3;
  b.data = 2;
  a.data = 1;

  list.insert (c);              // v1
  list.insert (b);              // v2
  list.insert (a);              // v3

  list.set_field (*(list.get_heads ().back ().second->live_next ()), DATA, (void *) (20ul));    // v4
  list.set_field (*(list.get_heads ().back ().second->live_next ()), DATA, (void *) (200ul));   // v5

  list.insert (d);              // v6

  list.set_field (*(list.get_heads ().back ().second->live_next ()), DATA, (void *) (10ul));    // v7
  list.set_field (*(list.get_heads ().back ().second->live_next ()->live_next ()), DATA, (void *) (2000ul));    // v8
  list.set_field (*(list.get_heads ().back ().second->live_next ()->live_next ()->live_next ()), DATA, (void *) (30ul));        // v9

  for (vector < pair < size_t, Node * >>::size_type i = 0;
       i < list.get_heads ().size (); ++i) {
    cout << "List at version " << list.get_heads ()[i].first << ": ";
    list.print_at_version (list.get_heads ()[i].first);
  }

  std::string s = "Hello, world!";
  std::cout << s << std::endl;
  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
