/* digraph_bayesian.c
  
   Extends 'digraph.c' by providing additional functionality
   specific to bayesian networks
   
   Copyright 2011.
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
along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

#include <stdio.h>
#include <stdlib.h>
#include "digraph_bayesian.h"
//#include "../globals.h"

struct node {
  int v;
  links next;
};

/* Local function prototypes */
links add(int, links);
links remov_(int, links, int *, int);


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
  G->child = malloc(V*sizeof(links));
  G->parent = malloc(V*sizeof(links));
  G->root = NULL;
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
  links t;
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
  printf("Root(s): ");;
  for (t = G->root; t != NULL; t = t->next) {
    printf("%d, ", t->v);
  }
  printf("\n");
}

/* Find out the root vertice for the given Graph.  */
/* It's computational complexity is O(V)           */
/* @Param Graph reference                          */
/* @Return int - The index if exists, -1 otherwise */
int root_index(Graph G) {
  int i;
  for (i = 0; i < G->V; i++) {
    if (parent_index(G, i) == -1)
      return i;
  }
  return -1;  // Error case
}

/* This function needs to be called whenever before*/
/* calling root_reference                          */
/* @Param Graph reference                          */
void calc_root_nodes(Graph G) {
  int i;
  for (i = 0; i < G->V; i++) 
  if (parent_index(G, i) == -1) 
    G->root = add(i, G->root);
}

/* Returns the link pointing to one of the root    */
/* vertices                                        */
/* @Param Graph reference                          */
/* @Return links reference                         */
links root_reference(Graph G) {
  return G->root;
}


/* Depcrecated: Only applicable for single-parent topology
  Find the index of parent vertice                 */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
/* @Return int - The index if exists, -1 otherwise */
int parent_index(Graph G, int node) {
  if (G->parent[node] == NULL) // in case of root
    return -1;
  return G->parent[node]->v;
}

/* Find out the number of parent nodes for the     */
/* given node                                      */
/* @Param Graph reference                          */
/* @Param int node - node index                    */
/* @Return int - The number of parent nodes        */
int parent_number(Graph G, int node) {
  int total = 0;
  links t = G->parent[node];
  for (; t != NULL; t = t->next)
    total++;
  return total;
}

/* Returns the link pointing to one of the parent  */
/* vertices                                        */
/* @Param Graph reference                          */
/* @Param node - child vertice index               */
/* @Return links reference                         */
links parent_reference(Graph G, int node) {
  return G->parent[node];
}

/* Find the index of the child vertice - O(V)      */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
/* @Return int - The index if exists, -1 otherwise */
int children_index(Graph G, int node) {
  if (G->child[node] == NULL) // in case of leaf
    return -1; 
  return G->child[node]->v;
}

/* Returns the link pointing to one of the child   */
/* vertices                                        */
/* @Param Graph reference                          */
/* @Param node - vertice index                     */
/* @Return links reference                         */
links child_reference(Graph G, int node) {
  return G->child[node];
}

/* Returns the index of the given link             */
/* @Param link reference                           */
int link_index(links t) {
  if (t != NULL)
    return t->v;
  return -1;   // Error case
}

/* Find position of the node in the linked list    */
/* @Param Graph reference                          */
/* @Param child_node - child_node's vertex number  */
/* @Param parent_node -parent node's vertex number */
/* @Return int - Position of the parent node       */
int parent_position(Graph G, int child_node, int parent_node) {
  links start;
  int position = 0;
  for (start = G->parent[child_node]; start != NULL && start->v < parent_node; 
          start = start->next, position++);
  return position;
}


/* Returns the next element present in the linked- */
/* list.                                           */
/* @Param link reference                           */
links next_reference(links t) {
  return (t == NULL)?NULL:t->next;
}


// Adds a new element in the linked-list such that
// the elements remain sorted in ascending order
links add(int v, links list) {
  links t, new = malloc(sizeof *new);
  new->v = v;
  if (list == NULL || list->v > v) {
    new->next = list;
    return new;
  }
  for (t = list; t != NULL; t = t->next) {
    if (t->next == NULL || t->next->v > v) {
      new->next = t->next;
      t->next = new;
      return list;
    }
  }
  // Control should not reach here - // just in case
  return NULL;
}


links remov_(int v, links start, int *edge_counter, int condition) {

  links temp, current, previous;
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
