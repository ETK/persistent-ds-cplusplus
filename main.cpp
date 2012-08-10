#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>

#include "ephemeral/DoublyLinkedList.h"
#include "partiallypersistent/DoublyLinkedList.h"
#include "rollback_naive/DoublyLinkedList.h"

// #undef RANDOMIZE
#define RANDOMIZE

using namespace std;

void
dump_list_dot_graph (partiallypersistent::DoublyLinkedList & list) {
  ofstream file;
  file.open ("graph", ios_base::out);
  list.print_dot_graph (list.version, file);
  file.close ();
}

void
print_all_versions (partiallypersistent::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       0; i < list.get_versions ().size (); ++i) {
    cout << "List at version " << list.get_versions ()[i].
      version << " (size " << list.get_versions ()[i].size << "): ";
    size_t printed_size =
      list.print_at_version (list.get_versions ()[i].version);
    cout << "printed size: " << printed_size << endl;
  }
}

void
print_all_versions (rollback_naive::DoublyLinkedList & list) {
  for (vector < pair < size_t, partiallypersistent::Node * > >::size_type i =
       1; i <= list.num_records (); ++i) {
    cout << "List at version " << i << " (size " << list.size_at (i) << "): ";
    size_t printed_size = list.print_at_version (i);
    cout << "printed size: " << printed_size << endl;
  }
}

void
test_insert_modify_remove_rollback_naive (size_t count) {
  clock_t begin_operation, end_operation;

  rollback_naive::DoublyLinkedList list;

  cout << "rollback;insert;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = clock ();
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.size () >
                 0 ? (double) rand () * list.size () / RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "rollback;modify;" << count << ";";
#ifdef RANDOMIZE
    cout << "random;";
#else
    cout << "sequential;";
#endif
    begin_operation = clock ();
    for (size_t j = 0; j < count; ++j) {
      ephemeral::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      list.modify_data (index, j);
    }
    end_operation = clock ();
    cout << ((end_operation - begin_operation) * 1000.0 /
             CLOCKS_PER_SEC) << endl;
  }

  cout << "rollback;remove;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = clock ();
  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index = (double) rand () * list.size () / RAND_MAX;
#endif
      list.remove (index);
    } else {
      cout << "List empty at version " << list.num_records () << endl;
    }
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;

  cout << "rollback;access;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  size_t sum = 0UL;
  begin_operation = clock ();
  for (size_t i = 0UL; i < count; ++i) {
    size_t version_index = (double) rand () * list.num_records () / RAND_MAX;
    size_t v = version_index + 1;
    ephemeral::Node * n = list.head_at (v);
    size_t list_size = list.size_at (v);
    if (list_size == 0) {
      continue;
    }
    size_t list_index = (double) rand () * list_size / RAND_MAX;
    for (size_t j = 0; j < list_index; ++j) {
      if (n->next) {
        n = n->next;
      } else {
        break;
      }
    }
    size_t data = n->data;
    sum += data;
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;
}

void
test_insert_modify_remove_partiallypersistent (size_t count) {
  clock_t begin_operation, end_operation;

  partiallypersistent::DoublyLinkedList list;

  cout << "partiallypersistent;insert;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = clock ();
  for (size_t i = 0; i < count; ++i) {
#ifdef RANDOMIZE
    list.insert (i,
                 list.get_versions ().size () >
                 0 ? (double) rand () * list.get_versions ().back ().size /
                 RAND_MAX : 0);
#else
    list.insert (i, 0);
#endif
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;

  if (count <= 100) {
    print_all_versions (list);
  }

  for (size_t i = 0; i < 10; ++i) {
    cout << "partiallypersistent;modify;" << count << ";";
#ifdef RANDOMIZE
    cout << "random;";
#else
    cout << "sequential;";
#endif
    begin_operation = clock ();
    for (size_t j = 0; j < count; ++j) {
      partiallypersistent::Node * node = list.head ();
#ifdef RANDOMIZE
      size_t index =
        (double) rand () * list.get_versions ().back ().size / RAND_MAX;
#else
      size_t index = count / 10;
#endif

      for (size_t k = 0; k < index; ++k) {
        if (node->next ()) {
          node = node->next ();
        } else {
          break;
        }
      }
      list.set_data (node, j);
    }
    end_operation = clock ();
    cout << ((end_operation - begin_operation) * 1000.0 /
             CLOCKS_PER_SEC) << endl;
  }

  cout << "partiallypersistent;remove;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  begin_operation = clock ();
  for (size_t i = 0; i < count; ++i) {
    partiallypersistent::Node * node = list.head ();
    if (node) {
#ifdef RANDOMIZE
      size_t index =
        (double) rand () * list.get_versions ().back ().size / RAND_MAX;
      for (size_t j = 0; j < index; ++j) {
        if (node->next ()) {
          node = node->next ();
        } else {
          break;
        }
      }
#endif
      list.remove (node);
    } else {
      cout << "List empty at version " << list.version << endl;
    }
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;

  cout << "partiallypersistent;access;" << count << ";";
#ifdef RANDOMIZE
  cout << "random;";
#else
  cout << "sequential;";
#endif
  size_t sum = 0UL;
  begin_operation = clock ();
  for (size_t i = 0UL; i < count; ++i) {
    size_t version_index =
      (double) rand () * list.get_versions ().size () / RAND_MAX;
    partiallypersistent::DoublyLinkedList::version_info_t version_info =
      list.get_versions ()[version_index];
    partiallypersistent::Node * n = version_info.head;
    size_t list_index = (double) rand () * version_info.size / RAND_MAX;
    for (size_t j = 0; j < list_index; ++j) {
      if (n->next ()) {
        n = n->next ();
      } else {
        break;
      }
    }
    size_t data = n->data_at (version_info.version);
    sum += data;
  }
  end_operation = clock ();
  cout << ((end_operation - begin_operation) * 1000.0 /
           CLOCKS_PER_SEC) << endl;
}

void
test_insert_modify_remove_ephemeral (size_t count) {
#ifdef PROFILE_TIME
  clock_t begin = clock ();
#endif

  ephemeral::DoublyLinkedList list;

  for (size_t i = 0; i < count; ++i) {
    ephemeral::Node n;
    n.data = i;
    list.insert (n, list.size > 0 ? rand () * list.size / RAND_MAX : 0);
  }
  for (size_t i = 0; i < count; ++i) {
    size_t index = rand () * list.size / RAND_MAX;
    ephemeral::Node * node = list.head;
    for (size_t j = 0; j < index; ++j) {
      if (node->next) {
        node = node->next;
      } else {
        break;
      }
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
//   double vm, rss;
//   process_mem_usage (vm, rss);
//   cout << "VM: " << vm << "KB; RSS: " << rss << "KB" << endl;
#endif

}

int
main (int argc, char **argv) {

  int count = 10000;
  if (argc == 2) {
    count = atoi (argv[1]);
  }
//   cout << "================================================================================" << endl;
//   cout << "Testing partially persistent implementation: " << endl;
//   cout << "================================================================================" << endl;
  test_insert_modify_remove_partiallypersistent (count);
//   cout << endl;
//   cout << "================================================================================" << endl;
//   cout << "Testing naïve rollback implementation: " << endl;
//   cout << "================================================================================" << endl;
  test_insert_modify_remove_rollback_naive (count);
//   test_insert_modify_remove_ephemeral (count);

  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
