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
}

Node::~Node () {
}

void *
Node::get_field_at_version (field_name_t field_name, size_t v) {
  unsigned int max_version = 0;
  for (unsigned int i = 0; i < n_mods; ++i) {
    if (std::get < 1 > (mods[1]) == field_name
        && std::get < 0 > (mods[i]) <= v) {
      max_version = std::get < 0 > (mods[i]);
    }
  }
  if (max_version == 0) {
    switch (field_name) {
    case DATA:
      return (void *) data;
    case NEXT:
      return next;
    case PREV:
      return prev;
    }
  }
  return std::get < 2 > (mods[max_version]);
}

size_t Node::data_at (size_t v) {
  return (size_t) (get_field_at_version (DATA, v));
}

Node *
Node::next_at (size_t v) {
  return static_cast < Node * >(get_field_at_version (NEXT, v));
}

Node *
Node::prev_at (size_t v) {
  return static_cast < Node * >(get_field_at_version (PREV, v));
}

size_t Node::live_data () {
  return (size_t) (get_field_at_version
                   (DATA, (numeric_limits < size_t >::max ())));
}

Node *
Node::live_next () {
  return static_cast < Node * >(get_field_at_version
                                (NEXT, (numeric_limits < size_t >::max ())));
}

Node *
Node::live_prev () {
  return static_cast < Node * >(get_field_at_version
                                (PREV, (numeric_limits < size_t >::max ())));
}


// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
