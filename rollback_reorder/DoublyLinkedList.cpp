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

DoublyLinkedList::DoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist):DoublyLinkedList ()
  {
    this->max_no_snapshots = max_no_snapshots;
    this->max_snapshot_dist = max_snapshot_dist;
  }

  size_t DoublyLinkedList::num_records () {
    return records.size ();
  }


  void DoublyLinkedList::insert (size_t node_data, std::size_t index) {
    ensure_version (records.size ());

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

    ensure_version (records.size ());

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

    ensure_version (records.size ());

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
        for (size_t i = 0; i < snapshots.size (); i += exponent) {
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

  bool remove_insert_index (const pair < record_t, int64_t > &a,
                            const pair < record_t, int64_t > &b) {
    if (a.first.operation > b.first.operation) {
      return true;
    } else if (a.first.operation == b.first.operation) {
      if (a.first.operation == INSERT) {
        return a.first.index < b.first.index;
      } else if (a.first.operation == REMOVE) {
        return a.first.index > b.first.index;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  record_t reverse_record (record_t record) {
    switch (record.operation) {
    case INSERT:
      record.operation = REMOVE;
      record.old_data = record.data;
      break;
    case REMOVE:
      record.operation = INSERT;
      record.data = record.old_data;
      break;
    case MODIFY:
      size_t temp = record.old_data;
      record.old_data = record.data;
      record.data = temp;
      break;
    }
    return record;
  }

  size_t DoublyLinkedList::print_at_version (size_t v) {
    ensure_version (v);
    ephemeral_current.print ();
    return ephemeral_current.size;
  }

  size_t DoublyLinkedList::size () {
    ensure_version (records.size ());
    return ephemeral_current.size;
  }

  ephemeral::Node * DoublyLinkedList::head () {
    ensure_version (records.size ());
    return ephemeral_current.head;
  }

  size_t DoublyLinkedList::size_at (size_t v) {
    ensure_version (v);
    return ephemeral_current.size;
  }

  ephemeral::Node * DoublyLinkedList::head_at (size_t v) {
    ensure_version (v);
    return ephemeral_current.head;
  }

  void DoublyLinkedList::ensure_version (std::size_t v) {
    if (next_record_index == v || v == -1) {
      return;
    }

    jump_to_snapshot (v);

    if (next_record_index != v) {
      // 1. Work on copy of records from current to v
      vector < pair < record_t, int64_t >> recs;
      if (next_record_index < v) {
        for (size_t i = next_record_index; i < v; ++i) {
          record_t r = records[i];
          recs.push_back (make_pair (r, r.index));
        }
      } else {
        for (size_t i = next_record_index; i > v; --i) {
          record_t r = reverse_record (records[i]);
          recs.push_back (make_pair (r, r.index));
        }
      }

      // 2. Remove matching inserts and removes
      for (size_t i = 0; i < recs.size (); ++i) {
        record_t r = recs[i].first;
        if (r.operation == INSERT) {
          size_t index = r.index;
          for (size_t j = i + 1; j < recs.size (); ++j) {
            if (recs[j].first.operation == INSERT
                && recs[j].first.index <= index) {
              ++index;
            } else if (recs[j].first.operation == MODIFY
                       && recs[j].first.index == index) {
              recs[i].first.data = recs[j].first.data;
              recs.erase (recs.begin () + j);
              --j;
              continue;
            } else if (recs[j].first.operation == REMOVE) {
              if (recs[j].first.index < index) {
                --index;
              } else if (recs[j].first.index == index) {
                // Remove both, adjust in-between indices and move on.
                for (size_t k = i + 1; k < j; ++k) {
                  if (recs[k].first.operation != REMOVE
                      && recs[k].first.index >= r.index
                      || recs[k].first.index > r.index) {
                    --recs[k].first.index;
                  }
                }

                recs.erase (recs.begin () + j);
                recs.erase (recs.begin () + i);
                // count from same index now that Xi was removed
                --i;
                break;
              }
            }
          }
        }
      }

      // 3. Alter indices on operations prior to sorting
      for (size_t i = 0; i < recs.size (); ++i) {
        record_t ri = recs[i].first;
        switch (ri.operation) {
        case INSERT:
        case MODIFY:{
            size_t index = ri.index;
            for (size_t j = i + 1; j < recs.size (); ++j) {
              record_t rj = recs[j].first;
              if (rj.operation == INSERT && rj.index < index) {
                ++index;
              } else if (rj.operation == REMOVE && rj.index < index) {
                --index;
              }
            }
            recs[i].first.index = index;
            break;
          }
        case REMOVE:{
            size_t index = ri.index;
            for (size_t j = 0; j < recs.size (); ++j) {
              record_t rj = recs[j].first;
              if (j < i && rj.operation == INSERT && rj.index < index) {
                --index;
              } else if (j > i && rj.operation == REMOVE && rj.index < index) {
                ++index;
              }
            }
            recs[i].first.index = index;
            break;
          }
        }
      }

      // 4. Sort records by operation desc, index asc
      sort (recs.begin (), recs.end (), remove_insert_index);

      ephemeral::Node * node = ephemeral_current.head;
      size_t index = 0;

      while (node != 0x0 && node->next && index < recs[0].first.index) {
        node = node->next;
        ++index;
      }

      for (size_t i = 0; i < recs.size (); ++i) {
        record_t ri = recs[i].first;
        if (ri.operation == REMOVE) {
          while (index > ri.index) {
            node = node->prev;
            --index;
          }
          if (node->prev) {
            node->prev->next = node->next;
          }
          if (node->next) {
            node->next->prev = node->prev;
          }
          if (index == 0) {
            ephemeral_current.head = node->next;
          }
          if (node->next) {
            node = node->next;
          } else {
            node = node->prev;
            if (index > 0) {
              --index;
            }
          }

          --ephemeral_current.size;
        } else if (ri.operation == INSERT) {
          ephemeral::Node * new_node = new ephemeral::Node ();
          new_node->data = ri.data;
          bool already_done = false;
          while (index < recs[i].first.index) {
            ++index;
            if (node->next) {
              node = node->next;
            } else {
              node->next = new_node;
              new_node->prev = node;
              node = new_node;
              already_done = true;
              break;
            }
          }
          if (already_done) {
            break;
          }

          if (node && node->prev) {
            node->prev->next = new_node;
            new_node->prev = node->prev;
          } else {
            ephemeral_current.head = new_node;
          }
          if (node) {
            node->prev = new_node;
          }
          new_node->next = node;

          node = new_node;
          ++ephemeral_current.size;
        }
      }

      next_record_index = v;
    }
  }


  void DoublyLinkedList::jump_to_snapshot (std::size_t v) {
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

  void DoublyLinkedList::i (size_t data, size_t index) {
    record_t r;
    r.index = index;
    r.data = data;
    r.old_data = data;
    r.operation = INSERT;
    records.push_back (r);
  }

  void DoublyLinkedList::m (std::size_t data, size_t index) {
    record_t r;
    r.operation = MODIFY;
    r.index = index;
    r.data = data;
//     r.old_data = data; // FIXME: ??? Maybe at rollforward time?
    records.push_back (r);
  }

  void DoublyLinkedList::r (size_t index) {
    record_t r;
    r.operation = REMOVE;
    r.index = index;
//    r.old_data = data; // FIXME: ??? Maybe at rollforward time?
    records.push_back (r);
  }

  void DoublyLinkedList::test_reorder () {
    insert (0, 0);
    insert (3, 0);
    insert (5, 0);
    insert (7, 0);
    insert (2, 0);
    print_at_version (5);

    i (4, 3);
    i (6, 1);
    r (3);
    i (1, 5);
    i (8, 3);
    r (5);
    r (0);
    i (9, 2);
    r (6);
    r (1);
    r (3);
    r (1);
    r (0);

    print_at_version (records.size ());
    print_at_version (records.size () - 4);
    print_at_version (records.size () - 1);
    print_at_version (records.size () - 9);
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
