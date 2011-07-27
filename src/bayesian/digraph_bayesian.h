/* digraph_bayesian.h
  
   Extends the 'graph.h' interface by supporting specific to a tree-
   topology.
   
   Copyright 2011.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

digraph_bayesian.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#ifndef DIGRAPH_BAYESIAN_H
#define DIGRAPH_BAYESIAN_H
#include "graph.h"

/* Find out the root vertice for the given Graph.  */
/* It's computational complexity is O(V)           */
/* @Param Graph reference                          */
/* @Return int - The index if exists, -1 otherwise */
int root_index(Graph G);

/* Find the index of parent vertice - O(V)         */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
/* @Return int - The index if exists, -1 otherwise */
int parent_index(Graph G, int node);

/* Find the index of the child vertice - O(V)      */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
/* @Return int - The index if exists, -1 otherwise */
int children_index(Graph G, int node);

/* Returns the link pointing to one of the child   */
/* vertices                                        */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
links child_reference(Graph G, int node);

/* Returns the index of the given link             */
/* @Param link reference                           */
int link_index(links);

/* Returns the next element present in the linked- */
/* list.                                           */
/* @Param link reference                           */
links next_reference(links);
#endif
