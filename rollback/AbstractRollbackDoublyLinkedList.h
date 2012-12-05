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


#ifndef ROLLBACKDOUBLYLINKEDLIST_H
#define ROLLBACKDOUBLYLINKEDLIST_H

#include <vector>

#include "../AbstractDoublyLinkedList.h"

#include "../ephemeral/DoublyLinkedList.h"
#include "../ephemeral/Node.h"

#define INIT_MAX_SNAPSHOT_DIST 65UL
#define MAX_NO_SNAPSHOTS 4000UL

namespace rollback
{
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
    std::size_t size = 0;

    bool operator < (const record_t& other) const {
      return index < other.index;
    }
  };

  class AbstractRollbackDoublyLinkedList : public AbstractDoublyLinkedList
  {
  public:
    AbstractRollbackDoublyLinkedList ();
    AbstractRollbackDoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist);
    
    virtual ~AbstractRollbackDoublyLinkedList () {}

//     void insert (size_t node_data, std::size_t index);
//     void remove (std::size_t index);
//     void modify_data (std::size_t index, std::size_t value);
//     std::size_t print_at_version (std::size_t v);
//     std::size_t num_records ();
    ephemeral::Node* head ();
    ephemeral::Node* head_at (std::size_t v);
//     std::size_t size ();
//     std::size_t size_at (std::size_t v);

    const std::size_t a_access (const std::size_t version,
                                const std::size_t index);
    void a_insert (const std::size_t index, const std::size_t value);
    void a_modify (const std::size_t index, const std::size_t value);
    void a_remove (const std::size_t index);
    const std::size_t a_size ();
    const std::size_t a_size_at (const std::size_t version);
    const std::size_t a_num_versions ();
    void a_print_at (std::size_t version);
    const std::vector < std::pair < std::size_t,
          ephemeral::DoublyLinkedList* >> & get_snapshots() const;
  protected:
    std::size_t max_snapshot_dist;
    std::size_t max_no_snapshots;

    std::size_t next_record_index;
    
    ephemeral::DoublyLinkedList* ephemeral_current;
    std::vector < record_t > records;
    std::vector < std::pair < std::size_t,
        ephemeral::DoublyLinkedList* >> snapshots;

    void jump_to_snapshot (std::size_t v);

    virtual void rollforward ();
    virtual void ensure_version (std::size_t v) = 0;

    std::size_t get_snapshot_index (std::size_t v);
  };
}
#endif // ROLLBACKDOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
