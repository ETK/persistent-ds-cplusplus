/*
 *    <one line to give the program's name and a brief idea of what it does.>
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

#include <limits>

#include "Node.h"

using namespace std;

Node::Node () {
  n_mods = 0UL;
  next_ptr = 0;
  prev_ptr = 0;
  next_back_ptr = 0;
  prev_back_ptr = 0;
}

Node::~Node () {
}

void *Node::get_field_at_version (field_name_t field_name, size_t v) {
  size_t max_version_i = 0;
  bool in_mods = false;
  for (size_t i = 0; i < n_mods; ++i) {
    if (mods[i].field_name == field_name) {
      if (mods[i].version <= v) {
        max_version_i = i;
        in_mods = true;
      } else {
        break;
      }
    }
  }
  if (!in_mods) {
    switch (field_name) {
    case DATA:
      return (void *) data;
    case NEXT:
      return next_ptr;
    case PREV:
      return prev_ptr;
    }
  }
  return mods[max_version_i].value;
}

size_t Node::data_at (size_t v) {
  return (size_t) (get_field_at_version (DATA, v));
}

Node *Node::next_at (size_t v) {
  return reinterpret_cast < Node * >(get_field_at_version (NEXT, v));
}

Node *Node::prev_at (size_t v) {
  return reinterpret_cast < Node * >(get_field_at_version (PREV, v));
}

size_t Node::live_data () {
  return (size_t) (get_field_at_version
                   (DATA, (numeric_limits < size_t >::max ())));
}

Node *Node::next () {
  return reinterpret_cast < Node * >(get_field_at_version
                                     (NEXT,
                                      (numeric_limits < size_t >::max ())));
}

Node *Node::prev () {
  return reinterpret_cast < Node * >(get_field_at_version
                                     (PREV,
                                      (numeric_limits < size_t >::max ())));
}


// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
