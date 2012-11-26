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


#ifndef ROLLBACK_LAZY_DOUBLYLINKEDLIST_H
#define ROLLBACK_LAZY_DOUBLYLINKEDLIST_H

#include <cstdlib>
#include <vector>

#include "../AbstractRollbackDoublyLinkedList.h"

namespace rollback
{
  namespace blackbox
  {
    class DoublyLinkedList: public AbstractRollbackDoublyLinkedList
    {
    public:
      DoublyLinkedList () : AbstractRollbackDoublyLinkedList() {}
      DoublyLinkedList (size_t max_no_snapshots, size_t max_snapshot_dist) : AbstractRollbackDoublyLinkedList (max_no_snapshots, max_snapshot_dist) {};
      virtual ~DoublyLinkedList() {}
    private:
      virtual void rollforward();
      void ensure_version (std::size_t v);
      void rollback ();
    };
  }
}
#endif                          // ROLLBACK_LAZY_DOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;

