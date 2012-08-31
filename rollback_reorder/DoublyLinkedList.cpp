/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DoublyLinkedList.h"
#include <iostream>
#include <algorithm>

using namespace std;

namespace rollback_reorder {
  DoublyLinkedList::DoublyLinkedList () {
    ephemeral_current = ephemeral::DoublyLinkedList ();
    next_record_index = 0ul;
    records = vector < record_t > ();

    snapshots =
      vector < std::pair < std::size_t, ephemeral::DoublyLinkedList > >();
    max_snapshot_dist = INIT_MAX_SNAPSHOT_DIST;
    max_no_snapshots = MAX_NO_SNAPSHOTS;

    snapshots.push_back (std::make_pair (next_record_index,
                                         ephemeral::DoublyLinkedList
                                         (ephemeral_current)));
#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
  };

  DoublyLinkedList::DoublyLinkedList (size_t max_no_snapshots,
                                      size_t max_snapshot_dist) : DoublyLinkedList() {
    this->max_no_snapshots = max_no_snapshots;
    this->max_snapshot_dist = max_snapshot_dist;
  }

  size_t DoublyLinkedList::num_records () {
    return records.size ();
  }


  void DoublyLinkedList::insert (size_t node_data, std::size_t index) {
    if (records.size () > 0) {
      if (next_record_index < records.size () - 1) {
        next_record_index = snapshots.back ().first;
        ephemeral_current = snapshots.back ().second;

        while (next_record_index < records.size () - 1) {
          rollforward ();
        }
      }
    }

    record_t record;
    record.operation = INSERT;
    record.index = index;
    record.old_data = node_data;
    record.data = node_data;
    if (records.size () > 0) {
      record.size = 1 + records.back ().size;
    } else {
      record.size = 1;
    }
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::modify_data (std::size_t index, std::size_t value) {
    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    if (records.size () > 0) {
      if (next_record_index < records.size () - 1) {
        next_record_index = snapshots.back ().first;
        ephemeral_current = snapshots.back ().second;
        while (next_record_index < records.size () - 1) {
          rollforward ();
        }
      }
    }

    record_t record;
    record.operation = MODIFY;
    record.old_data = node->data;
    record.data = value;
    record.index = index;
    record.size = records.back ().size;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::remove (std::size_t index) {
    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    if (records.size () > 0) {
      if (next_record_index < records.size () - 1) {
        next_record_index = snapshots.back ().first;
        ephemeral_current = snapshots.back ().second;
        while (next_record_index < records.size () - 1) {
          rollforward ();
        }
      }
    }

    record_t record;
    record.operation = REMOVE;
    record.index = index;
    record.size = records.back ().size - 1;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::rollback () {
    record_t & record = records[next_record_index - 1];

    ephemeral::Node * node = ephemeral_current.head;
    size_t begin = 1;
    if (record.operation == INSERT) {
      begin = 0;
    }
    for (size_t i = begin; i < record.index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    switch (record.operation) {
    case INSERT:
      ephemeral_current.remove (*node);
      delete node;
      break;
    case REMOVE:{
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = record.data;
        ephemeral_current.insert (*new_node, record.index);
        break;
      }
    case MODIFY:
      node->data = record.old_data;
      break;
    }
    next_record_index--;
  }

  void DoublyLinkedList::rollforward () {
    record_t & record = records[next_record_index];

    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < record.index; ++i) {
      if (node->next) {
        node = node->next;
      }
    }

    switch (record.operation) {
    case INSERT:{
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = record.data;
        ephemeral_current.insert (*new_node, record.index);
        break;
      }
    case REMOVE:
      record.data = node->data;
      record.old_data = node->data;
      ephemeral_current.remove (*node);
      delete node;
      break;
    case MODIFY:
      node->data = record.data;
      break;
    }

    next_record_index++;

    if (next_record_index >= max_snapshot_dist * snapshots.size () - 1) {

      if (snapshots.size () == max_no_snapshots - 1) {

        size_t exponent = 2;
//         size_t exponent = MAX_NO_SNAPSHOTS / INIT_MAX_SNAPSHOT_DIST;
//         if (exponent <= 1) {
//           exponent = 2;
//         }

//         cout << "Increasing max snapshot distance from " << max_snapshot_dist << " to " << max_snapshot_dist * exponent << endl;
        
        max_snapshot_dist *= exponent;

        vector < pair < size_t, ephemeral::DoublyLinkedList > >new_snapshots;
        size_t index = 0;
        for (size_t i = 0; i < snapshots.size(); i += exponent) {
          new_snapshots.push_back (snapshots[i]);
        }
        snapshots = new_snapshots;
      }

      snapshots.push_back (std::make_pair (next_record_index,
                                           ephemeral::DoublyLinkedList
                                           (ephemeral_current)));

#ifdef DEBUG_SNAPSHOT_FEATURE
      cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
    }
  }

  bool first_only (const pair<record_t, int64_t>& a, const pair<record_t, int64_t>& b) {
    return a.first < b.first;
  }

  size_t DoublyLinkedList::print_at_version (size_t v) {
    if (next_record_index < v) {
      vector < pair < record_t, int64_t >> recs;
      for (size_t i = next_record_index; i < v; ++i) {
        recs.push_back (make_pair (records[i], records[i].index));
      }

      int64_t new_order[recs.size ()];
      for (int64_t i = 0; i < recs.size (); ++i) {
        new_order[i] = 0;
        for (int64_t j = 0; j < i; ++j) {
          if (recs[j].second <= recs[i].second) {
            new_order[i]--;
          }
        }
      }
      for (int64_t i = 0; i < recs.size (); ++i) {
        recs[i].second += new_order[i];
      }
      sort (recs.begin (), recs.end (), first_only);

      for (int64_t i = 0; i < recs.size (); ++i) {
        recs[i].second += i;
      }

      sort (recs.begin (), recs.end (), first_only);

      ephemeral::Node * node = ephemeral_current.head;
      size_t index = 0;
      for (size_t i = 0; i < recs.size (); ++i) {
        while (index < recs[i].second) {
          ++index;
          node = node->next;
        }
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = recs[i].first.data;

        if (node->prev) {
          node->prev->next = new_node;
          new_node->prev = node->prev;
        } else {
          ephemeral_current.head = new_node;
        }
        node->prev = new_node;
        new_node->next = node;

        node = new_node;
      }

      next_record_index = v;
    }


#ifdef DEBUG_SNAPSHOT_FEATURE
    size_t rolls = 0;
#endif

    jump_to_snapshot (v);

    while (v > next_record_index) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      ++rolls;
#endif
      rollforward ();
    }

    while (v < next_record_index) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      ++rolls;
#endif
      rollback ();
    }

#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << rolls << " rolls" << endl;
#endif
    ephemeral_current.print ();
    return ephemeral_current.size;
  }

  size_t DoublyLinkedList::size () {
    if (next_record_index < records.size () - 1) {
      next_record_index = snapshots.back ().first;
      ephemeral_current = snapshots.back ().second;

      while (num_records () > next_record_index) {
        rollforward ();
      }
    }

    return ephemeral_current.size;
  }

  ephemeral::Node * DoublyLinkedList::head () {
    if (next_record_index < records.size () - 1) {
      next_record_index = snapshots.back ().first;
      ephemeral_current = snapshots.back ().second;

      while (num_records () > next_record_index) {
        rollforward ();
      }
    }

    return ephemeral_current.head;
  }

  size_t DoublyLinkedList::size_at (size_t v) {
    jump_to_snapshot (v);

    while (v > next_record_index) {
      rollforward ();
    }

    while (v < next_record_index) {
      rollback ();
    }

    return ephemeral_current.size;
  }

  ephemeral::Node * DoublyLinkedList::head_at (size_t v) {
#ifdef DEBUG_SNAPSHOT_FEATURE
    size_t rolls = 0;
#endif
    jump_to_snapshot (v);

    while (v > next_record_index) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      ++rolls;
#endif
      rollforward ();
    }

    while (v < next_record_index) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      ++rolls;
#endif
      rollback ();
    }

#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << rolls << " rolls" << endl;
#endif
    return ephemeral_current.head;
  }

  void DoublyLinkedList::jump_to_snapshot (size_t v) {
    if (labs ((v - next_record_index)) <= max_snapshot_dist / 2) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      cout << "Closer than " << max_snapshot_dist / 2 +
        1 << ", not jumping" << endl;
#endif
      return;
    }

    size_t snapshot_index =
      (v + max_snapshot_dist / 2 + 1) / max_snapshot_dist;
    if (snapshot_index >= snapshots.size ()) {
      snapshot_index = snapshots.size () - 1;
    }

//     if ((next_record_index + max_snapshot_dist / 2 + 1) / max_snapshot_dist ==
//         snapshot_index) {
// #ifdef DEBUG_SNAPSHOT_FEATURE
//       cout << "Would jump to snapshot no. " << snapshot_index +
//         1 << ", but we're close enough" << endl;
// #endif
//       return;
//     }
// 
    pair < size_t, ephemeral::DoublyLinkedList > &snapshot =
      snapshots[snapshot_index];
#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << "Jumping to snapshot no. " << snapshot_index +
      1 << " (" << snapshot.first << ", size " << snapshot.
      second.size << ")" << endl;
#endif
    next_record_index = snapshot.first;
    ephemeral_current = ephemeral::DoublyLinkedList (snapshot.second);
  }

  void DoublyLinkedList::test_reorder () {
    insert(6, 0);
    insert(2, 0);
    insert(4, 0);
    insert(5, 0);
    insert(1, 0);
    print_at_version(5);
    record_t insert_record;
    insert_record.operation = INSERT;
    
    insert_record.data = 7;
    insert_record.index = 0;
    insert_record.size = 6;
    records.push_back(insert_record);
    
    insert_record.data = 9;
    insert_record.index = 4;
    insert_record.size = 7;
    records.push_back(insert_record);
    
    insert_record.data = 3;
    insert_record.index = 2;
    insert_record.size = 8;
    records.push_back(insert_record);
    
    insert_record.data = 0;
    insert_record.index = 7;
    insert_record.size = 9;
    records.push_back(insert_record);
    
    insert_record.data = 8;
    insert_record.index = 0;
    insert_record.size = 10;
    records.push_back(insert_record);

    print_at_version(10);
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;

