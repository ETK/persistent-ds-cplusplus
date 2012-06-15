#include <iostream>
#include <vector>
#include "DoublyLinkedList.h"

using namespace std;

int
main (int argc, char **argv) {

  DoublyLinkedList list;
  Node a, b, c;
  a.data = 3;
  b.data = 2;
  c.data = 1;
  list.insert (a);
  list.insert (b);
  list.insert (c);

  list.set_field (b, DATA, (void *) (20ul));
  list.set_field (b, DATA, (void *) (200ul));

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
