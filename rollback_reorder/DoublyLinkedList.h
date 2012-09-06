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


#ifndef ROLLBACK_REORDER_DOUBLYLINKEDLIST_H
#define ROLLBACK_REORDER_DOUBLYLINKEDLIST_H

#include <cstdlib>
#include <vector>

#include "../ephemeral/DoublyLinkedList.h"
#include "../ephemeral/Node.h"

#define INIT_MAX_SNAPSHOT_DIST 100UL
#define MAX_NO_SNAPSHOTS 1200UL

namespace rollback_reorder {
  enum operation_t {
    INSERT,
    MODIFY,
    REMOVE,
  };

  enum action_t {
    APPLY,
    UNAPPLY
  };

  struct record_t {
    operation_t operation;
      std::size_t index;
      std::size_t old_data;
      std::size_t data;
      std::size_t size;

      bool operator < (const record_t& other) const {
        return index < other.index;
      }
  };

  class DoublyLinkedList {

  public:
    DoublyLinkedList ();
    DoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist);

    void i(std::size_t data, size_t index);
    void r(size_t index);
    void m(std::size_t data, size_t index);
    void test_reorder();

    void insert (size_t node_data, std::size_t index);

    /// removes the specified node from the next version
    void remove (std::size_t index);

    void modify_data (std::size_t index, std::size_t value);

      std::size_t print_at_version (std::size_t v);

      std::size_t num_records ();
      ephemeral::Node * head ();
      ephemeral::Node * head_at (std::size_t v);
      std::size_t size ();
      std::size_t size_at (std::size_t v);

  private:
      ephemeral::DoublyLinkedList ephemeral_current;

      std::vector < record_t > records;
      std::vector < std::pair < std::size_t,
      ephemeral::DoublyLinkedList > >snapshots;
      std::size_t max_snapshot_dist;
      std::size_t max_no_snapshots;

    void rollback ();
    void rollforward ();
    void jump_to_snapshot (size_t v);

      std::size_t next_record_index;
  };
}
#endif                          // ROLLBACK_REORDER_DOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
