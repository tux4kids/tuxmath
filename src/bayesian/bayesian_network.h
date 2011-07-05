/* bayesian_network.h
  
   Interface for all the Bayesian network functions. At this point, one
   can work only on a single instance of Bayesian network since it keeps 
   the Bayesian-network instance to itself. [needs a design-change]
   
   Copyright 2005, 2008, 2009, 2010.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

bayesian_network.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#include "digraph_bayesian.h"
#ifndef BAYESIAN_NETWORK_H_
#define BAYESIAN_NETWORK_H_
#define NODE_VALUES 2

typedef struct joint_probability *Joint_Probability;

struct joint_probability {
  int number;
  double post_probabilitiy[NODE_VALUES];
  double lambda_value[NODE_VALUES];
  double lambda_message[NODE_VALUES];
  double pi_value[NODE_VALUES];
  double pi_message[NODE_VALUES];
  double *probability;
};

typedef struct evidence_set *Evidence_set;

struct evidence_set {
  int *index;
  int count;
};

struct bayesian_network {
  Graph G;
  Evidence_set E;
  Joint_Probability *P;
};

typedef struct bayesian_network *Bayesian_Network;

/* Initializes a Bayesian network with a specified */
/* number of nodes.                                */
/* @Param int - Number of nodes in the Bayesian    */
/* network                                         */
/* @Return - The initialized Bayesian_Network var. */
Bayesian_Network BN_init(int);

/* Adds a link from a node to the other. The nodes */
/* are identified only by indices                  */
/* @Param Bayesian_Network instance                */
/* @Param int - The 'from' node index              */
/* @Param int - The 'to' node index                */
void BN_add_link(Bayesian_Network, int from, int to);

/* Removes an existing link between two nodes. The */
/* direction of the link is identified by the posi-*/
/* tion of the arguments.                          */
/* @Param Bayesian_Network instance                */
/* @Param int - The 'from' node index              */
/* @Param int - The 'to' node index                */
void BN_remove_link(Bayesian_Network, int from, int to);

/* Specify the initial probabilites for each node  */
/* @Param Bayesian_Network instance                */
/* @Param int - the node index                     */
/* @Param double[] - the probability distribution  */
/* specified as an array(since the number of prob- */
/* abilities required depends on the number of     */
/* incoming links)                                 */
void BN_nodeprobability(Bayesian_Network, int, double[]);

/* Prints on the console the relations among nodes */
/* and the probability distribution                */
/* @Param Bayesian_Network instance                */
void BN_display(Bayesian_Network);

int BN_parent_index(Bayesian_Network, int);

int ismember_Evidence_Set(Bayesian_Network, int);

void debug_probability(Bayesian_Network);
#endif
