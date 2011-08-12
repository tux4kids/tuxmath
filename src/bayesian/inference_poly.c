/* inference_poly.c

   Inference is done by Pearl's message-passing algorithm
   for DAG with a topology of poly-tree. 

   Copyright 2011.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

inference_poly.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.⋅⋅
*/

#include "inference_poly.h"
#define STATE(x) (x == 0)?"TRUE":"FALSE"

/* local prototypes */
static void send_lambda_message(Bayesian_Network BN, int node_child, int node_parent);
static void send_pi_message(Bayesian_Network BN, int node_parent, int node_child);

void initial_net (Bayesian_Network BN) {
  int i, j, node_counter;
  int parent_index, root_in, index;
  links node, root;

  DEBUGMSG(debug_bayesian, "inital_net()\n");

  for (i = 0; i < BN->G->V; i++) {
    for (j = 0; j < NODE_VALUES; j++) {
      BN->P[i]->lambda_value[j] = 1.0; 				// compute 'lambda' values
      DEBUGMSG(debug_bayesian, "l_v__%d(%s) = %.2lf, ", i, STATE(j), BN->P[i]->lambda_value[j]);
    }
    DEBUGMSG(debug_bayesian, "\n");
    for (node = parent_reference(BN->G, i), node_counter = 0; node != NULL; node = next_reference(node)) {
      for (j = 0; j < NODE_VALUES; j++) {
        BN->P[i]->lambda_message[node_counter+j] = 1.0;      	// Compute 'lambda' messages
        DEBUGMSG(debug_bayesian, "l_m__%d(%s) = %.2lf, ", i, STATE(j), BN->P[i]->lambda_message[node_counter+j]);
      }
      node_counter += NODE_VALUES;
    }
    DEBUGMSG(debug_bayesian, "\n");
    for (node = child_reference(BN->G, i); node != NULL; node = next_reference(node)) {
      index = link_index(node);
      for (j = 0; j < NODE_VALUES; j++) {
        BN->P[index]->pi_message[j] = 1.0;			// Compute 'pi' messages
        DEBUGMSG(debug_bayesian, "pi_m__%d(%s) = %.2lf, ", i, STATE(j), BN->P[index]->[j]);
      }
      DEBUGMSG(debug_bayesian, "\n");
    }
  }

  for (root = root_reference(BN->G); root != NULL; root = next_reference(root)) {
    root_in = link_index(root);
    for (j = 0; j < NODE_VALUES; j++) {
      BN->P[root_in]->pi_value[j] = BN->P[root_in]->post_probabilitiy[j];	// Compute Root node's 'pi' values
      DEBUGMSG(debug_bayesian, "pi_v__%d(%s) = %.2lf, ", root_in, STATE(j), BN->P[root_in]->pi_value[j]);
    }
    for (node = child_reference(BN->G, root_in); node != NULL; node = next_reference(node)) {
      send_pi_message(BN, root_in, link_index(node));
    }
  }
}

void update_net(Bayesian_Network BN, int node_index, int value) {
  int i, index;
  links node;
  DEBUGMSG(debug_bayesian, "update_net() init\n");
  if (ismember_Evidence_Set(BN, node_index) == -1) {
    BN->E->index[BN->E->count] = node_index;
    BN->E->count++;
    DEBUGMSG(debug_bayesian, "Add new evidence node with index-> %d, Total count now: %d\n", node_index, BN->E->count);
  }

  for (i = 0; i < NODE_VALUES; i++) {
    if (i == value) {
      BN->P[node_index]->lambda_value[i] = 1.0;
      BN->P[node_index]->pi_value[i] = 1.0;
      BN->P[node_index]->post_probabilitiy[i] = 1.0;
    }
    else {
      BN->P[node_index]->lambda_value[i] = 0.0;
      BN->P[node_index]->pi_value[i] = 0.0;
      BN->P[node_index]->post_probabilitiy[i] = 0.0;
    }
  }

  for (node = parent_reference(BN->G, node_index); node != NULL; node = next_reference(BN->G, node)) {
    index = link_index(node);
    if (ismember_Evidence_Set(index) == -1) {
      send_lambda_message(node_index, index);
    }
  }

  for (node = child_reference(BN->G, node_index); node != NULL; node = next_reference(BN->G, node)) {
    index = link_index(node);
    if (ismember_Evidence_Set(index) == -1) {
      send_pi_message(node_index, index);
    }
  }
}

void send_lambda_message(Bayesian_Network BN, int node_child, int node_parent) {
  int i, j, num_bits, position_node_parent, index, bit_counter;
  links parent, child;
  double temp, temp_; 

  for (i = 0; i < NODE_VALUES; i++) {
    num_bits = parent_number(BN->G, node_child);
    // Find out node_parent's index in the list
    position_node_parent = 0;
    for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
       index = link_index(parent);
       if (index == node_parent)
         break;
       position_node_parent++;
    }
    for (j = 0, temp = 0.0, temp_ = 0.0, bit_counter = num_bits-1; j < (NODE_VALUES << num_bits); j++) {
      if ((j >> position_node_parent) % 2 != i) continue;
      while (bit_counter >= 0) {
	if (position_node_parent != bit_counter)
          bit[bit_counter] = 1 & (j >> bit_counter);
	bit_counter--;
      }
      temp_ = BN->P[node_child]->probability[j];
      bit_counter = num_bit-1;
      for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
        index = link_index(parent);
	if (index != node_parent)
          temp_ *= BN->P[index]->pi_message(bit[bit_counter]);
      }
      temp += temp_;
    }
    BN->P[node_parent]->lambda_message[i] = temp;

    BN->P[node_parent]->lambda_value[i] = 1.0;
    for (child = child_reference(BN->G, node_parent); child != NULL; child = next_reference(child)) {
      index = link_index(BN->G, child);
      BN->P[node_parent]->lambda_value[i] *= BN->P[index]->lambda_message[i];
    }

    BN->P[node_parent]->post_probabilitiy[i] = BN->P[node_parent]->lambda_value[i]*(BN->P[node_parent]->pi_value[i]);
  }

  normalizer = 1.0/(BN->P[node_parent]->post_probabilitiy[0]+BN->P[node_parent]->post_probabilitiy[1]);
  for (i = 0; i < NODE_VALUES; i++) {
    BN->P[node_parent]->post_probabilitiy[i] *= normalizer;
    DEBUGMSG(debug_bayesian, "P(Node_%d) = %.2lf\n", node_parent, BN->P[node_parent]->post_probabilitiy[i]);
  }

  for (parent = parent_reference(BN->G, node_parent); parent != NULL; parent = next_reference(parent)) {
    index = link_index(parent);
    if (ismember_Evidence_Set(BN, index) == -1)
      send_lambda_message(node_parent, index);
  }

  for (child = child_reference(BN->G, node_parent); child != NULL; child = next_reference(child)) {
    index = link_index(child);
    if (ismember_Evidence_Set(BN, index) == -1)
      send_pi_message(node_parent, index);
  }
}

void send_pi_message(Bayesian_Network BN, int node_parent, int node_child) {
  int i, j, index, bit[8] = {0}, num_bits, bit_counter;
  double temp = 0.0, normalizer;
  links child, parent;
  for (i = 0; i < NODE_VALUES; i++) {
    temp = BN->P[node_parent]->pi_value[i];
    child = child_reference(BN->G, node_parent);
    for (; child != NULL; child = next_reference(child)) {
      index = link_index(child);
      if (index != node_child)
        temp *= BN->P[index]->lambda_message[i];
    }
    BN->P[node_child]->pi_message[i] = temp;
    DEBUGMSG(debug_bayesian, "pi_m_%d(%s) = %.2lf\n", node_child, STATE(i), BN->P[node_child]->pi_message[i]);
  }

  if (ismember_Evidence_Set(BN, node_child) == -1) {
    for (i = 0; i < NODE_VALUES; i++) {
      num_bits = parent_number(BN->G, node_child);
      for (j = 0, temp = 0.0, temp_ = 0.0, bit_counter = num_bits-1; j < (NODE_VALUES << num_bits); j++) {
	if (j%2 != i) continue;
	while (bit_counter >= 0) {
	  bit[bit_counter] = 1 & (j >> bit_counter);
	  bit_counter--;
	}
	temp_ = BN->P[node_child]->probability[j];
	bit_counter = num_bit-1;
	for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
	  index = link_index(parent);
          temp_ *= BN->P[index]->pi_message(bit[bit_counter]);
	}
	temp += temp_;
      }
      BN->P[node_child]->pi_value[i] = temp;
      BN->P[node_child]->post_probabilitiy[i] = BN->P[node_child]->lambda_value[i]*(BN->P[node_child]->pi_value[i]);
    }
    normalizer = 1.0/(BN->P[node_child]->post_probabilitiy[0]+BN->P[node_child]->post_probabilitiy[1]);
    for (i = 0; i < NODE_VALUES; i++) {
      BN->P[node_child]->post_probabilitiy[i] *= normalizer;
      DEBUGMSG(debug_bayesian, "P(Node_%d) = %.2lf\n", node_child, BN->P[node_child]->post_probabilitiy[i]);
    }
  }
  for (child = child_reference(BN->G, node_child); child != NULL; child = next_reference(child)) {
    send_pi_message(BN, node_child, link_index(child));
  }

  if (BN->P[node_child]->lambda_value[0] != 1 || BN->P[node_child]->lambda_value[1] != 1) {
    for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
      index = link_index(parent);
      if (index != node_parent  && ismember_Evidence_Set(BN, index) == -1)
        send_lambda_message(BN, node_child, index);
    }
  }
}
