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

// #define DEBUG_SNAPSHOT_FEATURE
// #include <iostream>

#include "DoublyLinkedList.h"
#include <iostream>

using namespace std;

namespace rollback_naive {
  DoublyLinkedList::DoublyLinkedList () {
    ephemeral_current = ephemeral::DoublyLinkedList ();
    next_record_index = 0ul;
    records = vector < record_t > ();

    snapshots =
      vector < std::pair < std::size_t, ephemeral::DoublyLinkedList > >();
    snapshots.push_back (std::make_pair < std::size_t,
                         ephemeral::DoublyLinkedList > (next_record_index,
                                                        ephemeral::DoublyLinkedList
                                                        (ephemeral_current)));
#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
    max_snapshot_dist = MAX_SNAPSHOT_DIST;
  };

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
      snapshots.push_back (std::make_pair < std::size_t,
                           ephemeral::DoublyLinkedList > (next_record_index,
                                                          ephemeral::DoublyLinkedList
                                                          (ephemeral_current)));

#ifdef DEBUG_SNAPSHOT_FEATURE
      cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
    }
  }

  size_t DoublyLinkedList::print_at_version (size_t v) {
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
    jump_to_snapshot (v);

    while (v > next_record_index) {
      rollforward ();
    }

    while (v < next_record_index) {
      rollback ();
    }

    return ephemeral_current.head;
  }

  void DoublyLinkedList::jump_to_snapshot (size_t v) {
    if (abs ((int) (v - next_record_index)) <= max_snapshot_dist / 2) {
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

    if ((next_record_index + max_snapshot_dist / 2 + 1) / max_snapshot_dist ==
        snapshot_index) {
#ifdef DEBUG_SNAPSHOT_FEATURE
      cout << "Would jump to snapshot no. " << snapshot_index +
        1 << ", but we're close enough" << endl;
#endif
      return;
    }

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
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
