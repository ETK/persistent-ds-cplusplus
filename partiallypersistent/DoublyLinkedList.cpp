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

namespace partiallypersistent {

  DoublyLinkedList::DoublyLinkedList () {
    versions = vector < version_info_t > ();
    version = 0UL;
  } pair < size_t, Node * >DoublyLinkedList::insert (Node & new_node) {
    version_info_t new_version;
    new_version.version = ++version;
    if (versions.size () > 0) {
      version_info_t current_version = versions.back ();
      new_version.size = 1 + current_version.size;
    } else {
      new_version.size = 1;
    }

    if (versions.size () > 0 && head ()) {
      new_node.next_ptr = head ();
      new_node.next_ptr->next_back_ptr = &new_node;
      Node & modified_head = modify_field (*head (), PREV, &new_node);
      if (&modified_head != head ()) {
        versions.back ().head = &modified_head;
      }
      new_node.prev_back_ptr = &modified_head;
    }
    new_version.head = &new_node;
    versions.push_back (new_version);
    return make_pair (version, head ());
  }

  pair < size_t, Node * >DoublyLinkedList::insert (Node & new_node,
                                                   size_t index) {
    version_info_t new_version;
    new_version.version = ++version;

    if (versions.size () > 0 && head ()) {
      // skip through list until correct position is found, or insert at end if index + 1 > size
      Node *insert_before = head ();
      for (size_t i = 0; i < index; ++i) {
        if (insert_before->next ()) {
          insert_before = insert_before->next ();
        } else {
          break;
        }
      }
      Node *insert_after = insert_before->prev ();

      // set pointers between new_node and to-be next
      new_node.next_ptr = insert_before;
      new_node.next_ptr->next_back_ptr = &new_node;
      insert_before = &modify_field (*insert_before, PREV, &new_node);
      new_node.prev_back_ptr = insert_before;

      if (insert_after) {
        // set pointers between new_node and to-be prev
        new_node.prev_ptr = insert_after;
        new_node.prev_ptr->prev_back_ptr = &new_node;
        insert_after = &modify_field (*insert_after, NEXT, &new_node);
        new_node.next_back_ptr = insert_after;
      }
      new_version.size = 1 + versions.back ().size;
      // if effective insertion index was zero, push new head on back of heads vector
      if (insert_before == head ()) {
        new_version.head = &new_node;
        versions.push_back (new_version);
//     } else {
//       new_version.head = head ();
      }
    } else {
      // just set the new head
      new_version.head = &new_node;
      new_version.size = 1;
      versions.push_back (new_version);
    }

    return make_pair (version, head ());
  }

  DoublyLinkedList::version_info_t DoublyLinkedList::remove (Node & to_remove) {
    version_info_t new_version;
    new_version.size = versions.back ().size - 1;
    new_version.version = ++version;

    Node *before = to_remove.prev ();
    Node *after = to_remove.next ();

    if (before) {
      before = &modify_field (*before, NEXT, after);
      if (after) {
        after->next_back_ptr = before;
      }
    }
    if (after) {
      after = &modify_field (*after, PREV, before);
      if (before) {
        before->prev_back_ptr = after;
      }
    }

    if (!to_remove.prev ()) {
      // to_remove has no previous node, so it must be head
      new_version.head = after;
      versions.push_back (new_version);
//   } else {
//     new_version.head = head ();
    }

    return new_version;
  }


  Node & DoublyLinkedList::modify_field (Node & node,
                                         field_name_t field_name,
                                         void *value) {
    if (node.n_mods < MAX_MODS) {
      Node::mod_t mod;
      mod.version = version;
      mod.field_name = field_name;
      mod.value = value;
      node.mods[node.n_mods++] = mod;
    } else {
      Node & n_prime = copy_live_node (node);
      switch (field_name) {
      case DATA:
        n_prime.data_val = (size_t) value;
        break;
      case NEXT:
        n_prime.next_ptr = reinterpret_cast < Node * >(value);
        if (n_prime.next_ptr) {
          n_prime.next_ptr->next_back_ptr = &n_prime;
        }
        break;
      case PREV:
        n_prime.prev_ptr = reinterpret_cast < Node * >(value);
        if (n_prime.prev_ptr) {
          n_prime.prev_ptr->prev_back_ptr = &n_prime;
        }
        break;
      }
      return n_prime;
    }
    return node;
  }

  pair < size_t, Node * >DoublyLinkedList::set_field (Node & node,
                                                      field_name_t field_name,
                                                      void *value) {
    version_info_t new_version;
    new_version.version = ++version;
    new_version.size = versions.back ().size;

    Node & modified_node = modify_field (node, field_name, value);
    if (!modified_node.prev ()) {
      new_version.head = &modified_node;
      versions.push_back (new_version);
//   } else {
//     new_version.head = head ();
    }
  }

  Node & DoublyLinkedList::copy_live_node (Node & node) {
    // copy latest version of each field (data and forward pointers) to the static field section.
    Node & copy = *(new Node ());
    copy.data_val = node.data ();
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

  const vector < DoublyLinkedList::version_info_t >
    &DoublyLinkedList::get_versions () {
    return versions;
  }

  void DoublyLinkedList::print_at_version (size_t v) {
    Node *head = versions.front ().head;
    for (vector < pair < size_t, Node * > >::size_type i = 0;
         i < versions.size (); ++i) {
      if (versions[i].version <= v) {
        head = versions[i].head;
      } else {
        break;
      }
    }
    Node *n = head;
    while (n) {
      cout << n->data_at (v) << " ";
      n = n->next_at (v);
    }
    cout << endl;
  }

  void DoublyLinkedList::print_dot_graph (std::size_t v) {
//   TODO
  }


  Node *DoublyLinkedList::head () const {
    return versions.back ().head;
  } Node *DoublyLinkedList::head_at (std::size_t v) const {
    Node *head = versions.front ().head;
    for (vector < pair < size_t, Node * > >::size_type i = 0;
         i < versions.size (); ++i) {
      if (versions[i].version <= v) {
        head = versions[i].head;
      } else {
        break;
      }
    }
    return head;
  }

}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
