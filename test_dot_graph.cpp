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
  
  DoublyLinkedList list;
  
  list.insert (5, 0);
  list.insert (4, 2);
  list.insert (3, 1);
  list.insert (2, 0);
  list.insert (1, 0);
  
  for (size_t i = 0; i < list.get_versions ().size (); ++i) {
    ofstream file;
    stringstream fname;
    fname << "v" << list.get_versions ()[i].version;
    file.open (fname.str ().c_str(), ios::out);
    list.print_dot_graph (list.get_versions ()[i].version, file);
    file.close();
  }
  
  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
