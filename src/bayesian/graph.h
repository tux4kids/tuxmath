/*
   graph.h: Interface for graph. We define two data-types:
   'Edge' and 'Graph'. The interface is independent from the
   representation used in implementation.

   Copyright 2007, 2008, 2009, 2010.
Authors: Siddharth Kothari
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


digraph.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

typedef struct {
  int out;
  int in;
} Edge;

/* Create an edge from 1st vertice to second */
Edge EDGE(int, int);

typedef struct graph *Graph;

/* Initialize graph                          */
/* @Param int - The number of vertices       */
/* @Return Graph                             */
Graph graph_init(int);

/* Insert a new edge in the Graph            */
/* @Param Graph, Edge                        */
void graph_insert_edge(Graph, Edge);

/* Remove an existing edge from the graph    */
/* @Param Graph, Edge                        */
void graph_remove_edge(Graph, Edge);
