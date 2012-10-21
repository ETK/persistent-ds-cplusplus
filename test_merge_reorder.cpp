#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <utility>
#include <limits>

#include "rollback_reorder/DoublyLinkedList.h"

using namespace std;
using namespace rollback_reorder;

double
rand01 () {
  return (double) rand () / RAND_MAX;
}

vector < record_t > merge (vector < record_t > left,
                           vector < record_t > right) {
  vector < record_t > result;
  while (left.size () > 0 || right.size () > 0) {
    if (left.size () > 0 && right.size () > 0) {
      if (left[0].index < right[0].index) {
        result.push_back (left[0]);
        left.erase (left.begin ());
      } else {
        result.push_back (right[0]);
        if (right[0].operation == REMOVE) {
          left[0].index -= 1;
        } else if (right[0].operation == INSERT) {
          left[0].index += 1;
        }
        right.erase (right.begin ());
      }
    } else if (left.size () > 0) {
      result.push_back (left[0]);
      left.erase (left.begin ());
    } else if (right.size () > 0) {
      result.push_back (right[0]);
      right.erase (right.begin ());
    }
  }
  return result;
}

vector < record_t > merge_sort (vector < record_t > v) {
  if (v.size () == 1) {
    return v;
  }
  vector < record_t > left, right;
  vector < record_t >::size_type middle = v.size () / 2;
  for (vector < record_t >::size_type i = 0; i < v.size (); ++i) {
    if (i < middle) {
      left.push_back (v[i]);
    } else {
      right.push_back (v[i]);
    }
  }
  left = merge_sort (left);
  right = merge_sort (right);
  return merge (left, right);
}

void
print_vector (vector < record_t > v) {
  for (vector < record_t >::const_iterator iter = v.begin ();
       iter != v.end (); ++iter) {
    switch ((*iter).operation) {
    case INSERT:
      cout << "insert(" << (*iter).
        data << "," << (*iter).index << ")" << endl;
      break;
    case MODIFY:
      cout << "modify(" << (*iter).
        data << "," << (*iter).index << ")" << endl;
      break;
    case REMOVE:
      cout << "remove(" << (*iter).index << ")" << endl;
      break;
    }
  }
  cout << endl;
}

vector < record_t > reorder (vector < record_t > v) {
  vector < record_t > result;
  while (v.size () > 0) {

    // Pick out record with lowest index
    size_t min_i;
    size_t min_index = -1;
    for (size_t i = 0; i < v.size (); ++i) {
      if (v[i].index < min_index) {
        min_i = i;
        min_index = v[i].index;
      }
    }
    record_t ri = v[min_i];
    v.erase (v.begin () + min_i);

    // Compensate where relevant
    switch (ri.operation) {
    case INSERT:
      for (int64_t i = min_i - 1; i >= 0; --i) {
        if (v[i].index > ri.index) {
          v[i].index++;
        }
      }
      break;
    case MODIFY:
      break;
    case REMOVE:
      for (int64_t i = min_i - 1; i >= 0; --i) {
        if (v[i].index > ri.index) {
          v[i].index--;
        }
      }
      break;
    }
    result.push_back (ri);
  }
  return result;
}

void
test_merge_reorder () {
  vector < record_t > records;
  record_t r;
  r.operation = INSERT;
  r.data = 9;
  r.index = 6;
  records.push_back (r);
  r.operation = REMOVE;
  r.index = 5;
  records.push_back (r);
  r.operation = INSERT;
  r.data = 13;
  r.index = 15;
  records.push_back (r);
  r.operation = MODIFY;
  r.data = 25;
  r.index = 9;
  records.push_back (r);
  r.operation = REMOVE;
  r.index = 4;
  records.push_back (r);

  cout << "before sorting:" << endl;
  print_vector (records);

//   records = merge_sort(records);
  records = reorder (records);

  cout << "after sorting:" << endl;
  print_vector (records);
}

int
main (int argc, char **argv) {
  test_merge_reorder ();
  return 0;
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
