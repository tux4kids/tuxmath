/* bayesian_network.c
  
   Bayesian network functions for creating and initializing the 
   network. The topology of the DAG (directed acyclic graph) 
   should be "linear" or "tree", as inferencing is supported 
   for only these two type of topologies.
   
   Copyright 2011.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

bayesian_network.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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
#include "bayesian_network.h"
#include "inference.h"
#include "../globals.h"

/* Initializes a Bayesian network with a specified */
/* number of nodes.                                */
/* @Param int - Number of nodes in the Bayesian    */
/* network                                         */
Bayesian_Network BN_init(int total_nodes) {
  int i = 0;
  Bayesian_Network BN = malloc(sizeof *BN);
  BN->G = graph_init(total_nodes);
  BN->E = malloc(sizeof *(BN->E));
  BN->E->index = malloc(total_nodes*sizeof(Evidence_set));
  BN->P = malloc(total_nodes*sizeof(Joint_Probability));
  for (; i < total_nodes; i++)
    BN->P[i] = NULL;
  return BN;
}

/* Adds a link from a node to the other. The nodes */
/* are identified only by indices                  */
/* @Param Bayesian_Network instance                */
/* @Param int - The 'from' node index              */
/* @Param int - The 'to' node index                */
void BN_add_link(Bayesian_Network BN, int node1, int node2) {
  graph_insert_edge(BN->G, EDGE(node1, node2));
}

/* Removes an existing link between two nodes. The */
/* direction of the link is identified by the posi-*/
/* tion of the arguments.                          */
/* @Param Bayesian_Network instance                */
/* @Param int - The 'from' node index              */
/* @Param int - The 'to' node index                */
void BN_remove_link(Bayesian_Network BN, int node1, int node2) {
  graph_remove_edge(BN->G, EDGE(node1, node2));
}

/* Specify the initial probabilites for each node  */
/* @Param Bayesian_Network instance                */
/* @Param int - the node index                     */
/* @Param double[] - the probability distribution  */
/* specified as an array(since the number of prob- */
/* abilities required depends on the number of     */
/* incoming links)                                 */
void BN_nodeprobability(Bayesian_Network BN, int node, double probability[]) {
  int num = parent_index(BN->G, node);
  int i,j;
  num = (num == -1)?2:4; // 'num' stores the number of prob. values required
  
  BN->P[node] = malloc(sizeof(*(BN->P[node])));
  BN->P[node]->number = num;
  BN->P[node]->probability = (double *)malloc(num*sizeof(double));  
  for (i=0,j=0; i < num; i += 2, j++) {
    BN->P[node]->probability[i] = probability[j];
    BN->P[node]->probability[i+1] = 1.0-probability[j];
  }
  // Initialize the posterior-probability in case of a root node
  if (num == 2) {
    for (i = 0; i < NODE_VALUES; i++)
      BN->P[node]->post_probabilitiy[i] = BN->P[node]->probability[i];
  }
}

/* Returns the parent node's index                 */
/* @Param Bayesian_Network reference               */
/* @Param int - The index of the node whose parent */
/* is to be found                                  */
int BN_parent_index(Bayesian_Network BN, int index) {
  return parent_index(BN->G, index);
}

/* Find out whether a node is a member of the evid */
/* ence set.                                       */
/* @Param Bayesian_Network reference               */
/* @Param int - index of the node                  */
int ismember_Evidence_Set(Bayesian_Network BN, int index) {
  int i;
  for (i = 0; i < BN->E->count; i++) {
    if (index == BN->E->index[i])
      return 1;
  }
  return -1;    // not found
}

// For debug purposes - not needed for the user 
void debug_probability(Bayesian_Network BN) {
  int i,j,node;
  for (node = 0; node < BN->G->V; node++) {
    DEBUGMSG(debug_bayesian, "node %d (", node);
    for (i=0,j=0; i < BN->P[node]->number; i += 2, j++) {
      DEBUGMSG(debug_bayesian, "%.4lf, %.4lf, ",BN->P[node]->probability[i], BN->P[node]->probability[i+1]);
    }
    DEBUGMSG(debug_bayesian, ")\n");
  }
}

/* Prints on the console the relations among nodes */
/* and the probability distribution                */
/* @Param Bayesian_Network instance                */
/* @Param int - 0 for additional debug info.       */
/*          any positive value for normal output   */
void BN_display(Bayesian_Network BN, int quantity) {
  DEBUGMSG(debug_bayesian, "Structure\n");
  graph_display(BN->G);
  DEBUGMSG(debug_bayesian, "Joint probability distribution\n");
  int v = 0, i;
  Joint_Probability p;
  Graph g = BN->G;
  for (; v < g->V; v++) {
    DEBUGMSG(debug_bayesian, "Node #%d: ", v);
    for (i = 0; i < BN->P[v]->number; i++)
      DEBUGMSG(debug_bayesian, "%.2lf, ", BN->P[v]->probability[i]);
    DEBUGMSG(debug_bayesian, "\n");
    if (quantity == 0) {
      DEBUGMSG(debug_bayesian, "lmda values: (%lf, %lf)\n", BN->P[v]->lambda_value[0], BN->P[v]->lambda_value[1]);
      DEBUGMSG(debug_bayesian, "lmda message: (%lf, %lf)\n", BN->P[v]->lambda_message[0], BN->P[v]->lambda_message[1]);
      DEBUGMSG(debug_bayesian, "pi message: (%lf, %lf)\n", BN->P[v]->pi_message[0], BN->P[v]->pi_message[1]);
      DEBUGMSG(debug_bayesian, "pi values: (%lf, %lf)\n", BN->P[v]->pi_value[0], BN->P[v]->pi_value[1]);
    }
    DEBUGMSG(debug_bayesian, "posterior:  (%lf, %lf)\n\n", BN->P[v]->post_probabilitiy[0], BN->P[v]->post_probabilitiy[1]);
    DEBUGMSG(debug_bayesian, "-------------------\n");
  }
  DEBUGMSG(debug_bayesian, "------------------------------------------------------------\n\n");
}
