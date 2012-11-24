#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include <unistd.h>
#include <ios>
#include <fstream>
#include <sstream>
#include <string>

#include "partiallypersistent/DoublyLinkedList.h"

using namespace std;
using namespace partiallypersistent;

int main (int argc, char **argv) {
  
  AbstractDoublyLinkedList* list = new DoublyLinkedList();

  cout << "insert (0, 5)" << endl;
  list->a_insert (0, 5);
  list->a_print_at(1);
  cout << "insert (1, 4)" << endl;
  list->a_insert (1, 4);
  list->a_print_at(2);
  cout << "insert (1, 3)" << endl;
  list->a_insert (1, 3);
  list->a_print_at(3);
  cout << "modify (1, 7)" << endl;
  list->a_modify (1, 7);
  list->a_print_at(4);
  cout << "insert (0, 2)" << endl;
  list->a_insert (0, 2);
  list->a_print_at(5);
  cout << "remove (1)" << endl;
  list->a_remove (1);
  list->a_print_at(6);
  cout << "insert (0, 1)" << endl;
  list->a_insert (0, 1);
  list->a_print_at(7);
  
  for (size_t i = 0; i < list->a_num_versions(); ++i) {
    ofstream file;
    stringstream fname;
    fname << "v" << i;
    file.open (fname.str ().c_str(), ios::out);
    (static_cast<DoublyLinkedList*>(list))->print_dot_graph (i, file);
    file.close();
  }

  delete list;
  
  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
