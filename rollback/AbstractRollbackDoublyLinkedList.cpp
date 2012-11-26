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


#include "AbstractRollbackDoublyLinkedList.h"

namespace rollback
{
  using namespace std;

  AbstractRollbackDoublyLinkedList::AbstractRollbackDoublyLinkedList() : max_no_snapshots (MAX_NO_SNAPSHOTS), max_snapshot_dist (INIT_MAX_SNAPSHOT_DIST), AbstractDoublyLinkedList()
  {
    ephemeral_current = new ephemeral::DoublyLinkedList ();
    ephemeral_current->size = 0;
    next_record_index = 0ul;
    records = vector < record_t > ();

    snapshots =
      vector < std::pair < std::size_t, ephemeral::DoublyLinkedList* > >();

    snapshots.
    push_back (std::make_pair (next_record_index, ephemeral_current));
  }


  AbstractRollbackDoublyLinkedList::AbstractRollbackDoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist) : AbstractRollbackDoublyLinkedList()
  {
    max_snapshot_dist = max_snapshot_dist;
    max_no_snapshots = max_no_snapshots;
  }

  const size_t AbstractRollbackDoublyLinkedList::a_access (const std::size_t version,
      const std::size_t index)
  {
    ephemeral::Node* node = head_at (version);
    for (size_t i = 0; i < index; ++i) {
      node = node->next;
    }
    if (node) {
      return node->data;
    }
    else {
      return -1;
    }
  }

  void AbstractRollbackDoublyLinkedList::a_insert (const std::size_t index,
      const std::size_t value)
  {
    insert (value, index);
  }

  void AbstractRollbackDoublyLinkedList::a_modify (const std::size_t index,
      const std::size_t value)
  {
    modify_data (index, value);
  }

  void AbstractRollbackDoublyLinkedList::a_remove (const std::size_t index)
  {
    remove (index);
  }

  const std::size_t AbstractRollbackDoublyLinkedList::a_size_at (const std::size_t version)
  {
    return size_at (version);
  }

  const std::size_t AbstractRollbackDoublyLinkedList::a_size ()
  {
    return size ();
  }

  const std::size_t AbstractRollbackDoublyLinkedList::a_num_versions ()
  {
    return records.size () + 1;
  }

  void AbstractRollbackDoublyLinkedList::a_print_at (std::size_t version)
  {
    ensure_version (version);
    ephemeral_current->print ();
  }

  size_t AbstractRollbackDoublyLinkedList::num_records ()
  {
    return records.size ();
  }

  void AbstractRollbackDoublyLinkedList::insert (size_t node_data, std::size_t index)
  {
    ensure_version (records.size ());

    record_t record;
    record.operation = INSERT;
    record.index = index;
    record.old_data = node_data;
    record.data = node_data;
    if (records.size () > 0) {
      record.size = 1 + records.back ().size;
    }
    else {
      record.size = 1;
    }
    records.push_back (record);
    rollforward ();
  }

  void AbstractRollbackDoublyLinkedList::modify_data (size_t index, size_t value)
  {
    ensure_version (records.size ());

    ephemeral::Node* node = ephemeral_current->head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      }
      else {
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

  void AbstractRollbackDoublyLinkedList::remove (std::size_t index)
  {
    ensure_version (records.size ());

    ephemeral::Node* node = ephemeral_current->head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      }
      else {
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

  size_t AbstractRollbackDoublyLinkedList::print_at_version (size_t v)
  {
    ensure_version (v);
    ephemeral_current->print ();
    return ephemeral_current->size;
  }

  size_t AbstractRollbackDoublyLinkedList::size ()
  {
    ensure_version (records.size ());
    return ephemeral_current->size;
  }

  ephemeral::Node* AbstractRollbackDoublyLinkedList::head ()
  {
    ensure_version (records.size ());
    return ephemeral_current->head;
  }

  size_t AbstractRollbackDoublyLinkedList::size_at (size_t v)
  {
    ensure_version (v);
    size_t size = ephemeral_current->size;
    return size;
  }

  ephemeral::Node* AbstractRollbackDoublyLinkedList::head_at (size_t v)
  {
    ensure_version (v);
    return ephemeral_current->head;
  }


  size_t AbstractRollbackDoublyLinkedList::get_snapshot_index (size_t v)
  {
    size_t snapshot_index =
      (v + max_snapshot_dist / 2 + 1) / max_snapshot_dist;
    if (snapshot_index >= snapshots.size ()) {
      snapshot_index = snapshots.size () - 1;
    }
    return snapshot_index;
  }

  void AbstractRollbackDoublyLinkedList::rollforward ()
  {
    record_t& record = records[next_record_index];

    next_record_index++;

    if (next_record_index > max_snapshot_dist * snapshots.size () - 1) {

      if (snapshots.size () == max_no_snapshots - 1) {

        size_t exponent = 2;

        max_snapshot_dist *= exponent;

        vector < pair < size_t,
               ephemeral::DoublyLinkedList* > > new_snapshots;
        size_t index = 0;
        for (size_t i = 0; i < snapshots.size (); ++i) {
          if (i % exponent == 0) {
            new_snapshots.push_back (snapshots[i]);
          }
          else {
            delete snapshots[i].second;
          }
        }
        snapshots = new_snapshots;
      }

      ephemeral_current = new ephemeral::DoublyLinkedList
      (*ephemeral_current);
      snapshots.push_back (std::make_pair (next_record_index,
                                           ephemeral_current));
    }

    ephemeral::Node* node = ephemeral_current->head;

    switch (record.operation) {
    case INSERT: {
      ephemeral::Node* new_node = new ephemeral::Node ();
      new_node->data = record.data;
      ephemeral_current->insert (*new_node, record.index);
      break;
    }
    case REMOVE:
      for (size_t i = 0; i < record.index; ++i) {
        node = node->next;
      }
      record.data = node->data;
      record.old_data = node->data;
      ephemeral_current->remove (*node);
      delete node;
      break;
    case MODIFY:
      for (size_t i = 0; i < record.index; ++i) {
        node = node->next;
      }
      record.old_data = node->data;
      node->data = record.data;
      break;
    }

    snapshots[get_snapshot_index (next_record_index)].first =
      next_record_index;
  }

  void AbstractRollbackDoublyLinkedList::jump_to_snapshot (std::size_t v)
  {
    pair < size_t, ephemeral::DoublyLinkedList* >&snapshot =
      snapshots[get_snapshot_index (v)];
    next_record_index = snapshot.first;
    ephemeral_current = snapshot.second;
  }

  const vector< std::pair< std::size_t, ephemeral::DoublyLinkedList* > >& AbstractRollbackDoublyLinkedList::get_snapshots() const
  {
    return snapshots;
  }
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
