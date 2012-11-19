/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Sune Keller <email>

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


#ifndef ABSTRACTDOUBLYLINKEDLIST_H
#define ABSTRACTDOUBLYLINKEDLIST_H

#include <cstddef>

class AbstractDoublyLinkedList {
public:
  virtual void a_insert(const std::size_t index, const std::size_t value) = 0;
  virtual void a_modify(const std::size_t index, const std::size_t value) = 0;
  virtual void a_remove(const std::size_t index) = 0;
  virtual const std::size_t a_access(const std::size_t version, const std::size_t index) = 0;
  virtual const std::size_t a_size() = 0;
  virtual const std::size_t a_size_at(const std::size_t version) = 0;
  virtual const std::size_t a_num_versions() = 0;
  virtual void a_print_at(std::size_t version) = 0;
};

#endif // ABSTRACTDOUBLYLINKEDLIST_H
