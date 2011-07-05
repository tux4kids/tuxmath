/* digraph_bayesian.c
  
   Extends 'digraph.c' by providing additional functionality
   specific to bayesian networks
   
   Copyright 2005, 2008, 2009, 2010.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

digraph_bayesian.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#include <stdio.h>
#include <stdlib.h>
#include "digraph_bayesian.h"


struct node {
  int v;
  link next;
};


/* Local function prototypes */
link add(int, link);
link remov_(int, link, int *, int);


/* Create an edge from 1st vertice to second   */
/* @Param v - Origin vertice                   */
/* @Param w - Destination vertice              */
Edge EDGE(int v, int w) {
  Edge e;
  e.out = v;
  e.in = w;
  return e;
}

/* Initialize graph with a fixed number of     */
/* vertices                                    */
/* @Param int - The number of vertices         */
/* @Return Graph                               */
Graph graph_init(int V) {
  int v;
  Graph G = malloc(sizeof *G);
  G->V = V;
  G->E = 0;
  G->child = malloc(V*sizeof(link));
  G->parent = malloc(V*sizeof(link));
  for (v = 0; v < V; v++) {
    G->child[v] = NULL;
    G->parent[v] = NULL;
  }
  return G;
}

/* Insert a new edge in the Graph. Insertion   */
/* is O(1) since we don't check for duplicates */
/* @Param Graph, Edge                          */
void graph_insert_edge(Graph G, Edge e) {
  int vertice_out = e.out;
  int vertice_in = e.in;
  G->child[vertice_out] = add(vertice_in, G->child[vertice_out]);
  G->parent[vertice_in] = add(vertice_out, G->parent[vertice_in]);
  G->E++;
}

/* Remove an edge from the graph. Cost of      */
/* deletion is proportional to O(V) and the    */
/* implementation also removes the duplicates  */
void graph_remove_edge(Graph G, Edge e) {
  int vertice_out = e.out;
  int vertice_in = e.in;
  G->child[vertice_out] = remov_(vertice_in, G->child[vertice_out], &G->E, 0);
  G->parent[vertice_in] = remov_(vertice_out, G->parent[vertice_in], &G->E, 1);
}

/* Prints the graph on the console             */
/* @Param Graph                                */
void graph_display(Graph G) {
  int v;
  link t;
  for(v = 0; v < G->V; v++) {
    printf("%d (child )-> ", v);
    for(t = G->child[v]; t != NULL; t = t->next) {
      printf("%d, ", t->v);
    }
    printf("\n%d (parent)-> ", v);
    for(t = G->parent[v]; t != NULL; t = t->next) {
      printf("%d, ", t->v);
    }
    printf("\n");
  }
}

int root_index(Graph G) {
  int i;
  for (i = 0; i < G->V; i++) {
    if (parent_index(G, i) == -1)
      return i;
  }
  return -1;  // Error case
}

int parent_index(Graph G, int node) {
  if (G->parent[node] == NULL) // in case of root
    return -1;
  return G->parent[node]->v;
}

int children_index(Graph G, int node) {
  if (G->child[node] == NULL) // in case of leaf
    return -1; 
  return G->child[node]->v;
}

link child_reference(Graph G, int node) {
  return G->child[node];
}

int link_index(link t) {
  if (t != NULL)
    return t->v;
  return -1;   // Error case
}

link next_reference(link t) {
  return (t == NULL)?NULL:t->next;
}

link add(int v, link list) {
  link new = malloc(sizeof *new);
  new->v = v;
  new->next = list;
  return new;
}


link remov_(int v, link start, int *edge_counter, int condition) {

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
      if (condition)
        (*edge_counter)--;
      // de-allocate memory
      free(temp);
    } 
    else previous = previous->next;
    current = current->next;
  }
  return start;
}
