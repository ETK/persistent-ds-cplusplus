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
#include <iostream>
#include <algorithm>

using namespace std;

namespace rollback
{
  namespace eliminate_reorder
  {
    bool remove_insert_index (const pair < record_t, int64_t > &a,
                              const pair < record_t, int64_t > &b)
    {
      if (a.first.operation > b.first.operation) {
        if (a.first.operation == MODIFY && b.first.operation == INSERT) {
          return a.first.index < b.first.index;
        }
        return true;
      }
      else
        if (a.first.operation == b.first.operation) {
          if (a.first.operation == REMOVE) {
            return a.first.index > b.first.index;
          }
          else {
            return a.first.index < b.first.index;
          }
        }
        else
          if (b.first.operation == MODIFY && a.first.operation == INSERT) {
            return a.first.index < b.first.index;
          }
          else {
            return false;
          }
    }

    record_t reverse_record (record_t record)
    {
      switch (record.operation) {
      case INSERT:
        record.operation = REMOVE;
        record.old_data = record.data;
        break;
      case REMOVE:
        record.operation = INSERT;
        record.data = record.old_data;
        break;
      case MODIFY:
        size_t temp = record.old_data;
        record.old_data = record.data;
        record.data = temp;
        break;
      }
      return record;
    }

    bool unsorted (const vector < record_t > &v)
    {
      size_t last_index = 0;
      for (vector < record_t >::const_iterator c_iter = v.cbegin ();
           c_iter != v.cend (); ++c_iter) {
        size_t index = (*c_iter).index;
        if (index < last_index) {
          return true;
        }
        last_index = index;
      }
      return false;
    }

    vector < record_t > reorder (vector < record_t > v)
    {
      vector < record_t > result;
      while (v.size () > 0) {

        // Pick out record with lowest index
        size_t min_i;
        size_t min_index = -1;
        for (size_t i = 0; i < v.size (); ++i) {
          if (v[i].index < min_index) {
            min_i = i;
            min_index = v[i].index;
          }
        }
        record_t ri = v[min_i];
        v.erase (v.begin () + min_i);

        // Compensate where relevant
        switch (ri.operation) {
        case INSERT:
          for (int64_t i = min_i - 1; i >= 0; --i) {
            if (v[i].index > ri.index) {
              v[i].index++;
            }
          }
          break;
        case MODIFY:
          break;
        case REMOVE:
          for (int64_t i = min_i - 1; i >= 0; --i) {
            if (v[i].index > ri.index) {
              v[i].index--;
            }
          }
          break;
        }
        result.push_back (ri);
      }
      return result;
    }


#ifdef DEBUG_ELIMINATE_OPS
    void print_operations_batch (const vector < record_t > &recs)
    {
      for (size_t i = 0; i < recs.size (); ++i) {
        const record_t& r = recs[i];
        cout << i << ": ";
        switch (r.operation) {
        case INSERT:
          cout << "insert(" << r.data << "," << r.index << ")" << endl;
          break;
        case MODIFY:
          cout << "modify(" << r.data << "," << r.index << ")" << endl;
          break;
        case REMOVE:
          cout << "remove(" << r.index << ")" << endl;
          break;
        }
      }
    }
#endif
    
    void DoublyLinkedList::ensure_version (std::size_t v)
    {
      if (next_record_index == v || v == -1) {
        return;
      }

      jump_to_snapshot (v);

      if (next_record_index != v) {
        // 1. Work on copy of records from current to v
        vector < record_t > recs;
        if (next_record_index < v) {
          for (size_t i = next_record_index; i < v; ++i) {
            record_t r = records[i];
            recs.push_back (r);
          }
        }
        else {
          for (size_t i = next_record_index - 1; i >= v && i != -1; --i) {
            record_t r = reverse_record (records[i]);
            recs.push_back (r);
          }
        }
#ifdef DEBUG_ELIMINATE_OPS
        cout << "Original sequence:" << endl;
        print_operations_batch (recs);
        cout << endl;
#endif

        if (recs.size () > 0) {
          for (size_t i = recs.size () - 1; i != -1; --i) {
            switch (recs[i].operation) {
            case INSERT:
            case MODIFY:
              size_t c = recs[i].index;
              for (size_t j = i + 1; j < recs.size (); ++j) {
                record_t rj = recs[j];

                if (rj.operation == INSERT) {
                  if (rj.index <= c) {
                    ++c;
                  }
                }
                else
                  if (rj.operation == MODIFY) {
                    if (rj.index == c) {
#ifdef DEBUG_ELIMINATE_OPS
                      cout << "Updating " << i << ", then removing " << j << ":"
                           << endl;
#endif
                      recs[i].data = rj.data;
                      recs.erase (recs.begin () + j);
                      --j;
#ifdef DEBUG_ELIMINATE_OPS
                      print_operations_batch (recs);
                      cout << endl;
#endif
                    }
                  }
                  else
                    if (rj.operation == REMOVE) {
                      if (rj.index < c) {
                        --c;
                      }
                      else
                        if (rj.index == c) {
                          c = recs[i].index;
                          switch (recs[i].operation) {
                          case INSERT:
                            for (size_t k = i + 1; k < j; ++k) {
                              if (recs[k].index > c) {
                                --recs[k].index;
                              }
                              else {
                                switch (recs[k].operation) {
                                case INSERT:
                                  if (recs[k].index <= c) {
                                    ++c;
                                  }
                                  break;
                                case REMOVE:
                                  if (recs[k].index <= c) {
                                    --c;
                                  }
                                }
                              }
                            }
#ifdef DEBUG_ELIMINATE_OPS
                            cout << "Removing " << j << ", then " << i << ":" << endl;
#endif
                            recs.erase (recs.begin () + j);
                            recs.erase (recs.begin () + i);
                            break;
                          case MODIFY:
#ifdef DEBUG_ELIMINATE_OPS
                            cout << "Removing " << i << ":" << endl;
#endif
                            recs.erase (recs.begin () + i);
                            break;
                          }
#ifdef DEBUG_ELIMINATE_OPS
                          print_operations_batch (recs);
                          cout << endl;
#endif
                          break;
                        }
                    }
              }
              break;
            }
          }
        }
#ifdef DEBUG_ELIMINATE_OPS
        cout << "Final sequence:" << endl;
        print_operations_batch (recs);
#endif

        // 3. Sort by index until nice and clean
        while (unsorted (recs)) {
          recs = reorder (recs);
#ifdef DEBUG_REORDER_OPS
          cout << endl;
          cout << "After reorder:" << endl;
          print_operations_batch (recs);
#endif
        }

        ephemeral::Node* node = ephemeral_current->head;
        size_t index = 0;

        for (size_t i = 0; i < recs.size (); ++i) {
          record_t ri = recs[i];
          if (ri.operation == REMOVE) {
            while (index < ri.index) {
              node = node->next;
              ++index;
            }
            if (node->prev) {
              node->prev->next = node->next;
            }
            if (node->next) {
              node->next->prev = node->prev;
            }
            if (index == 0) {
              ephemeral_current->head = node->next;
            }
            ephemeral::Node* tmp = node;
            if (node->next) {
              node = node->next;
            }
            else {
              node = node->prev;
              if (index > 0) {
                --index;
              }
            }

            tmp->next = 0x0;
            tmp->prev = 0x0;
            delete tmp;

            --ephemeral_current->size;
          }
          else
            if (ri.operation == INSERT) {
              ephemeral::Node* new_node = new ephemeral::Node ();
              new_node->data = ri.data;
              bool already_done = false;
              while (index < recs[i].index) {
                ++index;
                if (node->next) {
                  node = node->next;
                }
                else {
                  node->next = new_node;
                  new_node->prev = node;
                  node = new_node;
                  ++ephemeral_current->size;
                  already_done = true;
                  break;
                }
              }
              if (already_done) {
                continue;
              }

              if (node && node->prev) {
                node->prev->next = new_node;
                new_node->prev = node->prev;
              }
              else {
                ephemeral_current->head = new_node;
              }
              if (node) {
                node->prev = new_node;
              }
              new_node->next = node;

              node = new_node;
              ++ephemeral_current->size;
            }
            else
              if (ri.operation == MODIFY) {
                while (index < recs[i].index) {
                  ++index;
                  if (node->next) {
                    node = node->next;
                  }
                  else {
                    break;
                  }
                }
                node->data = recs[i].data;
              }
        }

        next_record_index = v;
        snapshots[get_snapshot_index (next_record_index)].first =
          next_record_index;
      }
    }
  }
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;


