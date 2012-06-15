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
#include <limits>

using namespace std;

DoublyLinkedList::DoublyLinkedList () {
  heads = vector < pair < size_t, Node * >>();
  version = numeric_limits < size_t >::min ();
}


DoublyLinkedList::~DoublyLinkedList () {
}


pair < size_t, Node * >DoublyLinkedList::insert (Node & new_node) {
  ++version;
  if (heads.size () > 0 && heads.back ().second) {
    new_node.next_ptr = heads.back ().second;
    new_node.next_ptr->next_back_ptr = &new_node;
    modify_field (*heads.back ().second, PREV, &new_node);
    new_node.prev_back_ptr = heads.back ().second;
  }
  heads.push_back (make_pair (version, &new_node));
  return make_pair (version, heads.back ().second);
}

pair < size_t,
  Node * >DoublyLinkedList::modify_field (Node & node,
                                          field_name_t field_name,
                                          void *value) {
  Node *
    new_head = heads.back ().second;
  if (node.n_mods < MAX_MODS) {
    node.mods[node.n_mods++] = make_tuple (version, field_name, value);
  } else {
    Node & n_prime = copy_live_node (node);
    switch (field_name) {
    case DATA:
      n_prime.data = (size_t) value;
      break;
    case NEXT:
      n_prime.next_ptr = reinterpret_cast < Node * >(value);
      n_prime.next_ptr->next_back_ptr = &n_prime;
      break;
    case PREV:
      n_prime.prev_ptr = reinterpret_cast < Node * >(value);
      n_prime.prev_ptr->prev_back_ptr = &n_prime;
      break;
    }
    if (heads.back ().second == &node) {
      new_head = &n_prime;
    }
  }
  return make_pair (version, new_head);
}

pair < size_t, Node * >DoublyLinkedList::set_field (Node & node,
                                                    field_name_t field_name,
                                                    void *value) {
  ++version;
  heads.push_back (modify_field (node, field_name, value));
}

Node & DoublyLinkedList::copy_live_node (Node & node) {
  // copy latest version of each field (data and forward pointers) to the static field section.
  Node & copy = *(new Node ());
  copy.data = node.live_data ();
  copy.prev_ptr = node.prev ();
  copy.next_ptr = node.next ();

  // also copy back pointers to n'
  copy.prev_back_ptr = node.prev_back_ptr;
  copy.next_back_ptr = node.next_back_ptr;

  // for every node x such that n points to x, redirect its back pointers to n' (using our pointers to get to them) (at most d of them)
  if (node.next ()) {
    node.next ()->next_back_ptr = &copy;
  }
  if (node.prev ()) {
    node.prev ()->prev_back_ptr = &copy;
  }
  // for every node x such that x points to n, call write(x.p, n') recursively (at most p recursive calls)
  if (node.next_back_ptr) {
    modify_field (*node.next_back_ptr, NEXT, &copy);
  }
  if (node.prev_back_ptr) {
    modify_field (*node.prev_back_ptr, PREV, &copy);
  }

  return copy;
}

const
  vector <
  std::pair <
size_t, Node * >>&
DoublyLinkedList::get_heads () {
  return heads;
}

void
  DoublyLinkedList::print_at_version (size_t v) {
  Node *
    head = heads.front ().second;
  for (vector < pair < size_t, Node * >>::size_type i = 0; i < heads.size ();
       ++i) {
    if (heads[i].first <= v) {
      head = heads[i].second;
    } else {
      break;
    }
  }
  Node *
    n = head;
  while (n) {
    cout << n->data_at (v) << " ";
    n = n->next_at (v);
  }
  cout << endl;
}

void
  DoublyLinkedList::print_dot_graph (std::size_t v) {
//   TODO
}


const Node *
DoublyLinkedList::head () {
  return heads.back ().second;
}


// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
