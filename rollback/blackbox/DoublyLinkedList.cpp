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

namespace rollback
{
  namespace blackbox
  {
    void DoublyLinkedList::rollforward ()
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

    void DoublyLinkedList::rollback ()
    {
      record_t& record = records[--next_record_index];

      ephemeral::Node* node = ephemeral_current->head;

      switch (record.operation) {
      case INSERT:
        for (size_t i = 0; i < record.index; ++i) {
          node = node->next;
        }
        ephemeral_current->remove (*node);
        delete node;
        break;
      case REMOVE: {
        ephemeral::Node* new_node = new ephemeral::Node ();
        new_node->data = record.data;
        ephemeral_current->insert (*new_node, record.index);
        break;
      }
      case MODIFY:
        for (size_t i = 0; i < record.index; ++i) {
          node = node->next;
        }
        node->data = record.old_data;
        break;
      }
    }

    void DoublyLinkedList::ensure_version (std::size_t v)
    {
      if (next_record_index == v || v == -1) {
        return;
      }

      jump_to_snapshot (v);

      if (next_record_index != v) {
        vector < record_t > recs;
        while (next_record_index < v) {
          rollforward ();
        }

        while (v < next_record_index) {
          rollback ();
        }
      }

      snapshots[get_snapshot_index (next_record_index)].first =
        next_record_index;
    }
  }
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
