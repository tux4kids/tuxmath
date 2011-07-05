#include "graph.h"
#ifndef DIGRAPH_BAYESIAN_H
#define DIGRAPH_BAYESIAN_H

/* Returns the reference to the parents_node linked*/
/* list                                            */ 
/* @Param node - node index                        */
int parent_index(Graph G, int node);

/* Returns the reference to the children_node      */
/* linked-list                                     */ 
/* @Param node - node index                        */
int children_index(Graph G, int node);

link child_reference(Graph G, int node);

int link_index(link);

int root_index(Graph G);
link next_reference(link);
#endif
