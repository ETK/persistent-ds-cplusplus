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

namespace rollback_reorder_lazy {
  DoublyLinkedList::DoublyLinkedList () {
    ephemeral_current = new ephemeral::DoublyLinkedList ();
    ephemeral_current->size = 0;
    next_record_index = 0ul;
    records = vector < record_t > ();

    snapshots =
      vector < std::pair < std::size_t, ephemeral::DoublyLinkedList * > >();
    max_snapshot_dist = INIT_MAX_SNAPSHOT_DIST;
    max_no_snapshots = MAX_NO_SNAPSHOTS;

    snapshots.push_back (std::
                         make_pair (next_record_index, ephemeral_current));
#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
  };

DoublyLinkedList::DoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist):DoublyLinkedList ()
  {
    this->max_no_snapshots = max_no_snapshots;
    this->max_snapshot_dist = max_snapshot_dist;
  }


  const std::size_t DoublyLinkedList::a_access (const std::size_t version,
                                                const std::size_t index) {
    ephemeral::Node* node = head_at(version);
    for (size_t i = 0; i < index; ++i) {
      node = node->next;
    }
    if (node) {
      return node->data;
    } else {
      return -1;
    }
  }

  void DoublyLinkedList::a_insert (const std::size_t index,
                                   const std::size_t value) {
    insert(value, index);
  }

  void DoublyLinkedList::a_modify (const std::size_t index,
                                   const std::size_t value) {
    modify_data(index, value);
  }

  void DoublyLinkedList::a_remove (const std::size_t index) {
    remove(index);
  }

  const std::size_t DoublyLinkedList::a_size_at (const std::size_t version) {
    return size_at(version);
  }

  const std::size_t DoublyLinkedList::a_size () {
    return size();
  }

  const std::size_t DoublyLinkedList::a_num_versions () {
    return records.size() + 1;
  }

  void DoublyLinkedList::a_print_at (std::size_t version) {
    ensure_version(version);
    ephemeral_current->print();
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
    ensure_version (records.size ());

    ephemeral::Node * node = ephemeral_current->head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
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
    ensure_version (records.size ());

    ephemeral::Node * node = ephemeral_current->head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    record_t record;
    record.operation = REMOVE;
    record.index = index;
    record.size = records.back ().size - 1;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::rollforward () {
    record_t & record = records[next_record_index];

    next_record_index++;

    if (next_record_index > max_snapshot_dist * snapshots.size () - 1) {

      if (snapshots.size () == max_no_snapshots - 1) {

        size_t exponent = 2;

        max_snapshot_dist *= exponent;

        vector < pair < size_t,
          ephemeral::DoublyLinkedList * > >new_snapshots;
        size_t index = 0;
        for (size_t i = 0; i < snapshots.size (); ++i) {
          if (i % exponent == 0) {
            new_snapshots.push_back (snapshots[i]);
          } else {
            delete snapshots[i].second;
          }
        }
        snapshots = new_snapshots;
      }

      ephemeral_current = new ephemeral::DoublyLinkedList
        (*ephemeral_current);
      snapshots.push_back (std::make_pair (next_record_index,
                                           ephemeral_current));

#ifdef DEBUG_SNAPSHOT_FEATURE
      cout << "Snapshot no. " << snapshots.size () << " made" << endl;
#endif
    }


    ephemeral::Node * node = ephemeral_current->head;
    for (size_t i = 0; i < record.index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "fuck off you moron";
      }
    }

    switch (record.operation) {
    case INSERT:{
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = record.data;
        new_node->next = node;
        if (node) {
          new_node->prev = node->prev;
          if (node->prev) {
            node->prev->next = new_node;
          }
          node->prev = new_node;
        }
        if (node == 0x0 || node == ephemeral_current->head) {
          ephemeral_current->head = new_node;
        }
        ++ephemeral_current->size;
        break;
      }
    case REMOVE:
      record.data = node->data;
      record.old_data = node->data;
      if (ephemeral_current->head == node) {
        ephemeral_current->head = node->next;
      }
      if (node) {
        if (node->prev) {
          node->prev->next = node->next;
        }
        if (node->next) {
          node->next->prev = node->prev;
        }
      }
      --ephemeral_current->size;
//       ephemeral_current.remove (*node);
      node->prev = 0x0;
      node->next = 0x0;
      delete node;
      break;
    case MODIFY:
      node->data = record.data;
      break;
    }

    snapshots[get_snapshot_index (next_record_index)].first =
      next_record_index;
  }

  bool remove_insert_index (const pair < record_t, int64_t > &a,
                            const pair < record_t, int64_t > &b) {
    if (a.first.operation > b.first.operation) {
      if (a.first.operation == MODIFY && b.first.operation == INSERT) {
        return a.first.index < b.first.index;
      }
      return true;
    } else if (a.first.operation == b.first.operation) {
      if (a.first.operation == REMOVE) {
        return a.first.index > b.first.index;
      } else {
        return a.first.index < b.first.index;
      }
    } else if (b.first.operation == MODIFY && a.first.operation == INSERT) {
      return a.first.index < b.first.index;
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
    ephemeral_current->print ();
    return ephemeral_current->size;
  }

  size_t DoublyLinkedList::size () {
    ensure_version (records.size ());
    return ephemeral_current->size;
  }

  ephemeral::Node * DoublyLinkedList::head () {
    ensure_version (records.size ());
    return ephemeral_current->head;
  }

  size_t DoublyLinkedList::size_at (size_t v) {
    ensure_version (v);
    size_t size = ephemeral_current->size;
    return size;
  }

  ephemeral::Node * DoublyLinkedList::head_at (size_t v) {
    ensure_version (v);
    return ephemeral_current->head;
  }

  bool unsorted (const vector < record_t > &v) {
    size_t last_index = 0;
    for (vector < record_t >::const_iterator c_iter = v.cbegin ();
         c_iter != v.cend (); ++c_iter) {
      size_t index = (*c_iter).index;
      if (index < last_index) {
        return true;
      }
      last_index = index;
    }
    return false;
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


  void print_operations_batch (const vector < record_t > &recs) {
    for (size_t i = 0; i < recs.size (); ++i) {
      const record_t & r = recs[i];
      cout << i << ": ";
      switch (r.operation) {
      case INSERT:
        cout << "insert(" << r.data << "," << r.index << ")" << endl;
        break;
      case MODIFY:
        cout << "modify(" << r.data << "," << r.index << ")" << endl;
        break;
      case REMOVE:
        cout << "remove(" << r.index << ")" << endl;
        break;
      }
    }
  }
  void DoublyLinkedList::ensure_version (std::size_t v) {
    if (next_record_index == v || v == -1) {
      return;
    }

    jump_to_snapshot (v);

    if (next_record_index != v) {
      // 1. Work on copy of records from current to v
      vector < record_t > recs;
      if (next_record_index < v) {
        for (size_t i = next_record_index; i < v; ++i) {
          record_t r = records[i];
          recs.push_back (r);
        }
      } else {
        for (size_t i = next_record_index - 1; i >= v && i != -1; --i) {
          record_t r = reverse_record (records[i]);
          recs.push_back (r);
        }
      }
#ifdef DEBUG_ELIMINATE_OPS
      cout << "Original sequence:" << endl;
      print_operations_batch (recs);
      cout << endl;
#endif

      if (recs.size () > 0) {
        for (size_t i = recs.size () - 1; i != -1; --i) {
          switch (recs[i].operation) {
          case INSERT:
          case MODIFY:
            size_t c = recs[i].index;
            for (size_t j = i + 1; j < recs.size (); ++j) {
              record_t rj = recs[j];

              if (rj.operation == INSERT) {
                if (rj.index <= c) {
                  ++c;
                }
              } else if (rj.operation == MODIFY) {
                if (rj.index == c) {
#ifdef DEBUG_ELIMINATE_OPS
                  cout << "Updating " << i << ", then removing " << j << ":"
                    << endl;
#endif
                  recs[i].data = rj.data;
                  recs.erase (recs.begin () + j);
                  --j;
#ifdef DEBUG_ELIMINATE_OPS
                  print_operations_batch (recs);
                  cout << endl;
#endif
                }
              } else if (rj.operation == REMOVE) {
                if (rj.index < c) {
                  --c;
                } else if (rj.index == c) {
                  c = recs[i].index;
                  switch (recs[i].operation) {
                  case INSERT:
                    for (size_t k = i + 1; k < j; ++k) {
                      if (recs[k].index > c) {
                        --recs[k].index;
                      } else {
                        switch (recs[k].operation) {
                        case INSERT:
                          if (recs[k].index <= c) {
                            ++c;
                          }
                          break;
                        case REMOVE:
                          if (recs[k].index <= c) {
                            --c;
                          }
                        }
                      }
                    }
#ifdef DEBUG_ELIMINATE_OPS
                    cout << "Removing " << j << ", then " << i << ":" << endl;
#endif
                    recs.erase (recs.begin () + j);
                    recs.erase (recs.begin () + i);
                    break;
                  case MODIFY:
#ifdef DEBUG_ELIMINATE_OPS
                    cout << "Removing " << i << ":" << endl;
#endif
                    recs.erase (recs.begin () + i);
                    break;
                  }
#ifdef DEBUG_ELIMINATE_OPS
                  print_operations_batch (recs);
                  cout << endl;
#endif
                  break;
                }
              }
            }
            break;
          }
        }
      }
#ifdef DEBUG_ELIMINATE_OPS
      cout << "Final sequence:" << endl;
      print_operations_batch (recs);
#endif

      // 3. Sort by index until nice and clean
      while (unsorted (recs)) {
        recs = reorder (recs);
#ifdef DEBUG_REORDER_OPS
        cout << endl;
        cout << "After reorder:" << endl;
        print_operations_batch (recs);
#endif
      }

      ephemeral::Node * node = ephemeral_current->head;
      size_t index = 0;

      for (size_t i = 0; i < recs.size (); ++i) {
        record_t ri = recs[i];
        if (ri.operation == REMOVE) {
          while (index < ri.index) {
            node = node->next;
            ++index;
          }
          if (node->prev) {
            node->prev->next = node->next;
          }
          if (node->next) {
            node->next->prev = node->prev;
          }
          if (index == 0) {
            ephemeral_current->head = node->next;
          }
          if (node->next) {
            node = node->next;
          } else {
            node = node->prev;
            if (index > 0) {
              --index;
            }
          }

//           node->next = 0x0;
//           node->prev = 0x0;
//           delete node;

          --ephemeral_current->size;
        } else if (ri.operation == INSERT) {
          ephemeral::Node * new_node = new ephemeral::Node ();
          new_node->data = ri.data;
          bool already_done = false;
          while (index < recs[i].index) {
            ++index;
            if (node->next) {
              node = node->next;
            } else {
              node->next = new_node;
              new_node->prev = node;
              node = new_node;
              ++ephemeral_current->size;
              already_done = true;
              break;
            }
          }
          if (already_done) {
            continue;
          }

          if (node && node->prev) {
            node->prev->next = new_node;
            new_node->prev = node->prev;
          } else {
            ephemeral_current->head = new_node;
          }
          if (node) {
            node->prev = new_node;
          }
          new_node->next = node;

          node = new_node;
          ++ephemeral_current->size;
        } else if (ri.operation == MODIFY) {
          while (index < recs[i].index) {
            ++index;
            if (node->next) {
              node = node->next;
            } else {
              break;
            }
          }
          node->data = recs[i].data;
        }
      }

      next_record_index = v;
      snapshots[get_snapshot_index (next_record_index)].first =
        next_record_index;
    }
  }

  size_t DoublyLinkedList::get_snapshot_index (size_t v) {
    size_t snapshot_index =
      (v + max_snapshot_dist / 2 + 1) / max_snapshot_dist;
    if (snapshot_index >= snapshots.size ()) {
      snapshot_index = snapshots.size () - 1;
    }
    return snapshot_index;
  }

  void DoublyLinkedList::jump_to_snapshot (std::size_t v) {
    pair < size_t, ephemeral::DoublyLinkedList * >&snapshot =
      snapshots[get_snapshot_index (v)];
#ifdef DEBUG_SNAPSHOT_FEATURE
    cout << "Jumping to snapshot no. " << snapshot_index +
      1 << " (" << snapshot.first << ", size " << snapshot.
      second.size << ")" << endl;
#endif
    next_record_index = snapshot.first;
    ephemeral_current = snapshot.second;
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;

