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
#include <iostream>
#include <fstream>
#include <utility>

#include <set>

// #define LOGGING
#undef LOGGING
#ifdef LOGGING
#include <sstream>
#endif

#ifdef EXTRA_ASSERTS
#include <cassert>
#endif

using namespace std;

namespace partiallypersistent
{

  DoublyLinkedList::DoublyLinkedList () : AbstractDoublyLinkedList()
  {
    versions = vector < version_info_t > ();
    version = 0UL;
    version_info_t v;
    v.head = 0x0;
    v.size = 0;
    versions.push_back (v);
  };

  const std::size_t DoublyLinkedList::a_access (const std::size_t version,
      const std::size_t index)
  {
    Node* node = head_at (version);
    for (size_t i = 0; i < index; ++i) {
      node = node->next_at (version);
    }
    return node->data_at (version);
  }

  void DoublyLinkedList::a_insert (const std::size_t index,
                                   const std::size_t value)
  {
    insert (value, index);
  }

  void DoublyLinkedList::a_modify (const std::size_t index,
                                   const std::size_t value)
  {
    Node* node = head();
    for (size_t i = 0; i < index; ++i) {
      node = node->next();
    }
    set_data (node, value);
  }

  void DoublyLinkedList::a_remove (const std::size_t index)
  {
    Node* node = head();
    for (size_t i = 0; i < index; ++i) {
      node = node->next();
    }
    remove (node);
  }

  const std::size_t DoublyLinkedList::a_size ()
  {
    return size();
  }

  const std::size_t DoublyLinkedList::a_size_at (const std::size_t version)
  {
    return size_at (version);
  }

  const std::size_t DoublyLinkedList::a_num_versions ()
  {
    return versions.size();
  }

  void DoublyLinkedList::a_print_at (std::size_t version)
  {
    print_at_version (version);
  }

#ifdef EXTRA_ASSERTS
  void assert_actual_size (Node* head, size_t expected_size)
  {
    size_t actual_size = 0;
    Node* n = head;
    while (n) {
      ++actual_size;
      n = n->next ();
    }
    assert (expected_size == actual_size);
  }
#endif

  pair < size_t, Node* >DoublyLinkedList::insert (size_t data, size_t index)
  {
#ifdef LOGGING
    cout << "v" << version << ": insert (" << data << ", " <<
         index << ")" << endl;
#endif

    version_info_t new_version;
    ++version;

    Node* new_node = new Node ();
#ifdef MEASURE_SPACE
    space += sizeof (*new_node);
#endif
    new_node->data_val = data;

    if (versions.size () > 0 && head ()) {
      Node* new_head = head();
      // skip through list until correct position is found, or insert at end if index > size
      Node* current_head = head ();
      Node* to_be_next = current_head;
      Node* to_be_prev;
      for (size_t i = 0; i < index; ++i) {
        if (to_be_next->next ()) {
          to_be_next = to_be_next->next ();
        }
        else {
          to_be_prev = to_be_next;
          to_be_next = 0x0;
          break;
        }
      }
      if (to_be_next) {
        to_be_prev = to_be_next->prev ();
      }
      // if effective insertion index was zero, push new head on back of heads vector
      if (to_be_next == current_head) {
        new_version.head = new_node;
      }
      else {
        new_version.head = current_head;
      }

      if (to_be_next) {
        // set pointers between new_node and to-be next
        to_be_next = modify_field (to_be_next, PREV, new_node, new_head);
        to_be_next->next_back_ptr = new_node;
        new_node->prev_back_ptr = to_be_next;
        new_node->next_ptr = to_be_next;
//         to_be_next->next_back_ptr = new_node;
      }

      if (to_be_prev) {
        // set pointers between new_node and to-be prev
        bool to_be_prev_is_head = to_be_prev == current_head;
        to_be_prev = modify_field (to_be_prev, NEXT, new_node, new_head);
        to_be_prev->prev_back_ptr = new_node;
        new_node->next_back_ptr = to_be_prev;
        new_node->prev_ptr = to_be_prev;
        if (to_be_prev_is_head) {
          new_version.head = to_be_prev;
        }
        else
          if (new_version.head != new_head) {
            new_version.head = new_head;
          }
//         to_be_prev->prev_back_ptr = new_node;
      }
      new_version.size = 1 + versions.back ().size;
//       if (new_version.head == new_node) {
      versions.push_back (new_version);
//       }
#ifdef EXTRA_ASSERTS
      assert_actual_size (new_version.head, new_version.size);
#endif
    }
    else {
      // just set the new head
      new_version.head = new_node;
      new_version.size = 1;
      versions.push_back (new_version);
#ifdef EXTRA_ASSERTS
      assert_actual_size (head (), new_version.size);
#endif
    }

    return make_pair (version, head ());
  }

  DoublyLinkedList::
  version_info_t DoublyLinkedList::remove (partiallypersistent::Node *
      to_remove)
  {
#ifdef LOGGING

    cout << "v" << version +
         1 << ": remove (" << to_remove->data () << ")" << endl;
#endif

    version_info_t new_version;
    new_version.size = versions.back ().size - 1;
    ++version;

    Node* before = to_remove->prev ();
    Node* after = to_remove->next ();
    Node* current_head;
    Node* new_head;
    current_head = head();
    new_head = current_head;

    if (before) {
      before->prev_back_ptr = after;
    }
    if (after) {
      after->next_back_ptr = before;
    }

    if (before) {
      before = modify_field (before, NEXT, after, new_head);
      if (after) {
        after->next_back_ptr = before;
      }
    }
    if (after) {
      after = modify_field (after, PREV, before, new_head);
      if (before) {
        before->prev_back_ptr = after;
      }
    }

    if (!before) {
      // to_remove has no previous node, so it must be head
      new_version.head = after;
    }
    else
      if (current_head != new_head) {
        new_version.head = new_head;
      }
      else {
        new_version.head = head ();
      }
    versions.push_back (new_version);

    return new_version;
  }

#ifdef LOGGING
  string node_to_string (Node* node)
  {
    stringstream ss;
    if (node->prev ()) {
      ss << node->prev ()->data ();
    }
    else {
      ss << "/";
    }
    ss << " < " << node->data () << " > ";
    if (node->next ()) {
      ss << node->next ()->data ();
    }
    else {
      ss << "/";
    }
    return ss.str ();
  }
#endif

  Node* DoublyLinkedList::modify_field (Node* node,
                                        field_name_t field_name,
                                        Node* value)
  {
    Node* empty;
    return modify_field (node, field_name, value, empty);
  }

  Node* DoublyLinkedList::modify_field (Node* node,
                                        field_name_t field_name,
                                        Node* value,
                                        Node*& head)
  {
#ifdef LOGGING
    string field_name_s =
      field_name == DATA ? "DATA" : field_name == PREV ? "PREV" : "NEXT";
    stringstream ss;
    if (field_name == DATA) {
      ss << ( (size_t) value);
    }
    else
      if (value) {
        ss << node_to_string (value);
      }
      else {
        ss << "/";
      }
    string value_s = ss.str ();
    string node_s = node_to_string (node);
    cout << "modify_field (" << node_s << ", " <<
         field_name_s << ", " << value_s << ")" << endl;
#endif

    if (node->n_mods < MAX_MODS) {
      Node::mod_t mod;
      mod.version = version;
      mod.field_name = field_name;
      mod.value = value;
      node->mods[node->n_mods++] = mod;
    }
    else {
      Node* copy = new Node ();
#ifdef MEASURE_SPACE
      space += sizeof (*copy);
#endif
      copy_live_node (node, copy, field_name, value, head);
      if (copy->prev() == 0x0) {
        head = copy;
      }
#ifdef LOGGING
      cout << " +> " << node_to_string (copy) << endl;
#endif
      return copy;
    }
#ifdef LOGGING
    cout << " => " << node_to_string (node) << endl;
#endif
    return node;
  }

  pair < size_t, Node* >DoublyLinkedList::set_data (Node* node,
      size_t value)
  {
#ifdef LOGGING
    cout << "v" << version << ": set_data (" << node_to_string (node) << ", "
         << value << ")" << endl;
#endif

    version_info_t new_version;
    ++version;
    new_version.size = versions.back ().size;
    Node* current_head = head ();

    Node* modified_node =
      modify_field (node, DATA, reinterpret_cast < Node* > (value), current_head);
    if (!modified_node->prev ()) {
      new_version.head = modified_node;
    }
    else {
      new_version.head = current_head;
    }
    versions.push_back (new_version);
  }

  void DoublyLinkedList::copy_live_node (partiallypersistent::Node* node,
                                         Node* copy,
                                         field_name_t field_name,
                                         Node* value, Node *& head)
  {
#ifdef LOGGING
    cout << "  COPY (";
#endif

    // copy latest version of each field (data and forward pointers) to the static field section.
    copy->data_val = node->data ();
    copy->prev_ptr = node->prev ();
    copy->next_ptr = node->next ();

    // also copy back pointers to n'
    copy->prev_back_ptr = node->prev_back_ptr;
    copy->next_back_ptr = node->next_back_ptr;

    switch (field_name) {
    case DATA:
      copy->data_val = (size_t) value;
      break;
    case NEXT:
      copy->next_ptr = reinterpret_cast < Node* > (value);
      copy->prev_back_ptr = value;
      if (copy->next_ptr) {
        copy->next_ptr->next_back_ptr = copy;
      }
      break;
    case PREV:
      copy->prev_ptr = reinterpret_cast < Node* > (value);
      copy->next_back_ptr = value;
      if (copy->prev_ptr) {
        copy->prev_ptr->prev_back_ptr = copy;
      }
      break;
    }

    // for every node x such that n points to x, redirect its back pointers to n' (using our pointers to get to them) (at most d of them)
    if (copy->prev ()) {
      copy->prev ()->prev_back_ptr = copy;
    }
    if (copy->next ()) {
      copy->next ()->next_back_ptr = copy;
    }
    // for every node x such that x points to n, call write(x.p, n') recursively (at most p recursive calls)
    if (copy->prev_back_ptr) {
      if (field_name == NEXT && copy->prev_back_ptr == value) {
        copy->prev_back_ptr->prev_ptr = copy;
      }
      else {
        copy->prev_back_ptr = modify_field (copy->prev_back_ptr, PREV, copy, head);
        copy = copy->prev_back_ptr->prev ();
      }
      copy->prev_back_ptr->next_back_ptr = copy;
    }
    if (copy->next_back_ptr) {
      if (field_name == PREV && copy->next_back_ptr == value) {
        bool found_in_mods = false;
        for (size_t i = copy->next_back_ptr->n_mods - 1; i != -1; --i) {
          if (copy->next_back_ptr->mods[i].field_name == NEXT) {
            copy->next_back_ptr->mods[i].value = copy;
            found_in_mods = true;
            break;
          }
        }
        if (!found_in_mods) {
          copy->next_back_ptr->next_ptr = copy;
        }
      }
      else {
        copy->next_back_ptr = modify_field (copy->next_back_ptr, NEXT, copy, head);
        copy = copy->next_back_ptr->next ();
      }
      copy->next_back_ptr->prev_back_ptr = copy;
    }
#ifdef LOGGING
    cout << ")" << endl;
#endif
  }

  const vector < DoublyLinkedList::version_info_t >
  &DoublyLinkedList::get_versions ()
  {
    return versions;
  }

  size_t DoublyLinkedList::print_at_version (size_t v)
  {
    size_t printed_size = 0;
    Node* head = head_at (v);
    Node* n = head;
    while (n) {
      ++printed_size;
      cout << n->data_at (v) << " ";
      cout.flush ();
      n = n->next_at (v);
    }
    cout << endl;

    return printed_size;
  }

  Node* DoublyLinkedList::head () const
  {
    return versions.back ().head;
  };

  Node* DoublyLinkedList::head_at (std::size_t v) const
  {
    return versions[v].head;
  }

  size_t DoublyLinkedList::size_at (size_t v) const
  {
    return versions[v].size;
  };

  size_t DoublyLinkedList::size () const
  {
    if (versions.size() > 0) {
      return versions.back ().size;
    }
    else {
      return 0;
    }
  };

  void print_dot_graph_helper (Node* n, size_t v,
                               set < Node* >&printed,
                               const set < Node* >&live_set,
                               ofstream& out)
  {
    if (printed.find (n) != printed.end ()) {
      return;
    }

    out << "    // node " << n << " with data " << n->data_at (v) << endl;

    if (printed.empty ()) {
      out << "    struct" << n <<
          "[style=filled,fillcolor=cyan,label=\"<id> " << n << "|{<data> " <<
          n->data_val << "|<prev> prev|<next> next}";
    }
    else
      if (live_set.find (n) != live_set.end ()) {
        out << "    struct" << n <<
            "[style=filled,fillcolor=cyan2,label=\"<id> " << n << "|{<data> " <<
            n->data_val << "|<prev> prev|<next> next}";
      }
      else {
        out << "    struct" << n <<
            "[style=filled,fillcolor=gray,label=\"<id> " << n << "|{<data> " <<
            n->data_val << "|<prev> prev|<next> next}";
      }
    printed.insert (n);

    for (size_t i = 0; i < MAX_MODS; ++i) {
      if (i < n->n_mods && n->mods[i].version <= v) {
        switch (n->mods[i].field_name) {
        case PREV:
          if (n->mods[i].value) {
            out << "|<prev" << i << "> prev (v" << n->mods[i].version << ")";
          }
          else {
            out << "|<prev" << i << "> prev := 0x0 (v" << n->
                mods[i].version << ")";
          }
          break;
        case NEXT:
          if (n->mods[i].value) {
            out << "|<next" << i << "> next (v" << n->mods[i].version << ")";
          }
          else {
            out << "|<prev" << i << "> next := 0x0 (v" << n->
                mods[i].version << ")";
          }
          break;
        case DATA:
          out << "|<data" << i << "> data := " << (size_t) n->mods[i].
              value << " (v" << n->mods[i].version << ")";
          break;
        }
      }
      else {
        out << "|- -";
      }
    }

    out << "\"];" << endl;

    vector < Node* >to_print = vector < Node* >();

    Node* prev = n->prev_at (v);
    Node* next = n->next_at (v);

    if (n->prev_ptr) {
      to_print.push_back (n->prev_ptr);
      out << "    struct" << n << ":prev -> struct" << n->prev_ptr <<
          ":id [color=blue, ";
      if (live_set.find (n) == live_set.end() || n->prev_ptr != prev) {
        out << "style=dashed";
      }
      out << "];" << endl;
    }
    if (n->next_ptr) {
      to_print.push_back (n->next_ptr);
      out << "    struct" << n << ":next -> struct" << n->next_ptr <<
          ":id [color=green, ";
      if (live_set.find (n) == live_set.end() || next != 0x0 && n->next_ptr != next) {
        out << "style=dashed";
      }
      out << "];" << endl;
    }

    for (size_t i = 0; i < n->n_mods; ++i) {
      if (n->mods[i].value) {
        if (n->mods[i].version > v) {
          continue;
        }
        switch (n->mods[i].field_name) {
        case PREV:
          to_print.push_back (n->mods[i].value);
          out << "    struct" << n << ":prev" << i << " -> struct" <<
              n->mods[i].value << ":id [color=blue, ";
          if (live_set.find (n) == live_set.end() || prev != 0x0 && n->mods[i].value != prev) {
            out << "style=dashed";
          }
          out << "];" << endl;
          break;
        case NEXT:
          to_print.push_back (n->mods[i].value);
          out << "    struct" << n << ":next" << i << " -> struct" <<
              n->mods[i].value << ":id [color=green, ";
          if (live_set.find (n) == live_set.end() || n->mods[i].value != next) {
            out << "style=dashed";
          }
          out << "];" << endl;
          break;
        default:
          break;
        }
      }
    }
    out << endl;

    if (next != 0x0) {
      print_dot_graph_helper (next, v, printed, live_set, out);
    }
    if (prev != 0x0) {
      print_dot_graph_helper (prev, v, printed, live_set, out);
    }

    for (size_t i = 0; i < to_print.size (); ++i) {
      print_dot_graph_helper (to_print[i], v, printed, live_set, out);
    }

//     set < Node * >same = set < Node * >();
//     if (n->next_ptr) {
//       same.insert (n->next_ptr);
//     }
//     for (size_t i = 0; i < n->n_mods; ++i) {
//       if (n->mods[i].field_name == NEXT && n->mods[i].value) {
//         same.insert (n->mods[i].value);
//       }
//     }
//     if (same.size () >= 2) {
//       out << "    { rank = same";
//       for (set < Node * >::iterator iter = same.begin (); iter != same.end ();
//            ++iter) {
//         out << "; " << "struct" << *iter;
//       }
//       out << " };" << endl;
//     }
  }

  void prepare_live_set_helper (Node* n, set < Node* >&live_set, size_t v,
                                set < Node* >&visited)
  {
    if (visited.find (n) != visited.end ()) {
      return;
    }

    bool is_live = live_set.find (n) != live_set.end();

    Node* prev = n->prev_at (v);
    Node* next = n->next_at (v);
    visited.insert (n);

    for (size_t i = n->n_mods - 1; i != -1; --i) {
      switch (n->mods[i].field_name) {
      case PREV:
        if (is_live && n->mods[i].value == prev) {
          live_set.insert (prev);
        }
        break;
      case NEXT:
        if (is_live && n->mods[i].value == next) {
          live_set.insert (next);
        }
        break;
      default:
        continue;
      }
    }
    if (n->prev_ptr) {
      if (is_live && n->prev_ptr == prev) {
        live_set.insert (n->prev_ptr);
      }
    }
    if (n->next_ptr) {
      if (is_live && n->next_ptr == next) {
        live_set.insert (n->next_ptr);
      }
    }

    for (size_t i = n->n_mods - 1; i != -1; --i) {
      if (n->mods[i].version > v) {
        continue;
      }
      switch (n->mods[i].field_name) {
      case PREV:
        prepare_live_set_helper (n->mods[i].value, live_set, v, visited);
        break;
      case NEXT:
        prepare_live_set_helper (n->mods[i].value, live_set, v, visited);
        break;
      default:
        continue;
      }
    }
    if (n->prev_ptr) {
      prepare_live_set_helper (n->prev_ptr, live_set, v, visited);
    }
    if (n->next_ptr) {
      prepare_live_set_helper (n->next_ptr, live_set, v, visited);
    }
  }

  void prepare_live_set (Node* n, set < Node* >&live_set, size_t v)
  {
    set < Node* >visited = set < Node* >();
    prepare_live_set_helper (n, live_set, v, visited);
  }

  void DoublyLinkedList::print_dot_graph (size_t v, ofstream& out)
  {
    set < Node* >printed = set < Node* >();
    set < Node* >live_set = set < Node* >();
    Node* head = head_at (v);
    if (head == 0x0) {
      return;
    }

    out << "digraph G {" << endl;
    out << "    label=\"List at version " << v << "\";" << endl;
    out << "    rankdir=LR;" << endl;
    out << "    node [shape=record];" << endl;
    out << endl;
    live_set.insert (head);
    prepare_live_set (head, live_set, v);
    print_dot_graph_helper (head, v, printed, live_set, out);
    out << "    structhead [label=\"<a> head\"];" << endl;
    out << "    structhead:a -> struct" << head << ":id;" << endl;
    out << "}" << endl;
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
