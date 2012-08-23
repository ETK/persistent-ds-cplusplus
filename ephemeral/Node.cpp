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


#include "Node.h"

namespace ephemeral {
  Node::Node () {
    data = 0;
    prev = 0;
    next = 0;
  }

  Node::~Node () {
//     Node* tmp = 0x0;
    if (next) {
      delete next;
//       tmp = next;
//       next = 0x0;
//       delete tmp;
    }
//     if (prev) {
//       tmp = prev;
//       prev = 0x0;
//       delete tmp;
//     }
  }
}