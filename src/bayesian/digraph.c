/* digraph.c
  
   Graph is represented in the form of adjaceny-lists;
   and the implementation is specific to directed graphs.
   
   Copyright 2005, 2008, 2009, 2010.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

digraph.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include "graph.h"

typedef struct node *link;

struct node {
  int v;
  link next;
};

struct graph {
  int V;
  int E;
  link *adj;
};

/* Local function prototypes */
link add(int, link);
link remove(int, link, int *);


/* Initialize graph with a fixed number of     */
/* vertices                                    */
/* @Param int - The number of vertices         */
/* @Return Graph                               */
Graph graph_init(int V) {
  int v;
  Graph G = malloc(sizeof *G);
  G->V = V;
  G->E = 0;
  G->adj = malloc(V*sizeof(link));
  for (v = 0; v < V; v++)
    G->adj[v] = NULL;
  return G;
}

/* Insert a new edge in the Graph. Insertion   */
/* is O(1) since we don't check for duplicates */
/* @Param Graph, Edge                          */
void graph_insert_edge(Graph G, Edge e) {
  int vertice_out = e.out;
  int vertice_in = e.in;
  G->adj[vertice_out] = add(vertice_in, G->adj[vertice_out]);
  G->E++;
}

/* Remove an edge from the graph. Cost of      */
/* deletion is proportional to O(V) and the    */
/* implementation also removes the duplicates  */
void graph_remove_edge(Graph G, Edge e) {
  int vertice_out = e.out;
  int vertice_in = e.in;
  G->adj[vertice_out] = remove(vertice_in, G->adj[vertice_out], &G->E);
}


link add(int v, link list) {
  link new = malloc(sizeof *new);
  new->v = v;
  list->next = new;
  return list;
}

link remove(int v, link start, int *edge_counter) {

  link temp, current, previous;
  // check the start link
  if (start->v == v) {
    temp = start;
    start=start->next;
    free(temp);
    return start;
  }
  current = start->next, previous = start;

  while (current != NULL) {
    if (current->v == v) {
      temp = current;
      previous->next = current->next;
      (*edge_counter)--;
      // de-allocate memory
      free(temp);
    } 
    else previous = previous->next;
    current = current->next;
  }
  return start;
}
