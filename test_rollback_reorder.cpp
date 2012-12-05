#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

#include "rollback/reorder/DoublyLinkedList.h"

using namespace std;

double rand01()
{
  return (double) rand() / RAND_MAX;
}

void test_rollback_reorder ()
{
  rollback::reorder::DoublyLinkedList list = rollback::reorder::DoublyLinkedList (4000, 65);
  vector<size_t> versions = vector<size_t>();
  size_t i = 0;
  cout << i << ":";
  list.a_print_at (i);
  for (i = 0; i < 500; ++i) {
    double r = rand01();
    size_t index = (size_t) (rand01() * list.a_size());
    if (r < 0.1 && list.a_size() > 0) {
      list.a_remove (index);
      cout << "remove(" << index << ")" << endl;
    } else if (r < 0.1 + 0.2 && list.a_size() > 0) {
      list.a_modify (index, i);
      cout << "modify(" << i << ", " << index << ")" << endl;
    } else {
      list.a_insert (index, i);
      cout << "insert(" << i << ", " << index << ")" << endl;
    }
    cout << i + 1 << ":";
    list.a_print_at (i + 1);

    versions.push_back (i);
  }
  versions.push_back (i);

  random_shuffle (versions.begin(), versions.end());

  for (vector<size_t>::iterator iter = versions.begin(); iter != versions.end(); ++iter) {
    cout << *iter << ":";
    list.a_print_at (*iter);
  }
}

int main (int argc, char** argv)
{
//   srand(3);
  unsigned seed;
  if (argc == 1) {
    seed = (unsigned) time (NULL);
  } else if (argc == 2) {
    seed = atoi (argv[1]);
  }
  srand (seed);
  test_rollback_reorder();
  cout << "seed: " << seed << endl;
  return 0;
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

