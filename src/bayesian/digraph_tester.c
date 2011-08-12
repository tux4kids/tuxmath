/* digraph_tester.c
  
   Simple test program to check validity of digraph.c functions
   
   Copyright 2011.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

digraph_tester.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

#include <stdio.h>
#include "graph.h"
#include "digraph_bayesian.h"

int main(void) {
  Graph G = graph_init(3);
  graph_insert_edge(G, EDGE(0, 2));
  graph_insert_edge(G, EDGE(1, 2));
/*  graph_insert_edge(G, EDGE(2, 3));
  graph_insert_edge(G, EDGE(4, 1));
  graph_insert_edge(G, EDGE(5, 3)); */
  calc_root_nodes(G);
  graph_display(G);
  return 0;
}
