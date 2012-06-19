/*
 *    <one line to give the program's name and a brief idea of what it
 * does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NODE_H
#define NODE_H

#define MAX_MODS 4

#include <cstddef>

enum field_name_t {
  DATA,
  PREV,
  NEXT
};

class Node {

public:
  struct mod_t {
    std::size_t version;
    field_name_t field_name;
    void *value;
  };

    std::size_t data;
  Node *prev_ptr;
  Node *next_ptr;

  Node *prev_back_ptr;
  Node *next_back_ptr;

    std::size_t n_mods;
  mod_t mods[MAX_MODS];

    Node ();
    virtual ~ Node ();

  void *get_field_at_version (field_name_t field_name, std::size_t v);

  Node *prev_at (std::size_t v);
  Node *next_at (std::size_t v);
    std::size_t data_at (std::size_t v);

  Node *prev ();
  Node *next ();
    std::size_t live_data ();
};

#endif // NODE_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
