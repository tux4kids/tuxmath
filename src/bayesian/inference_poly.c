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
#include "../globals.h"
#define STATE(x) (x == 0)?"TRUE":"FALSE"
#define FILE_DEBUG 0

/* local prototypes */
static void send_lambda_message(Bayesian_Network BN, int node_child, int node_parent);
static void send_pi_message(Bayesian_Network BN, int node_parent, int node_child);

void initial_net (Bayesian_Network BN) {
  int i, j, node_counter;
  int root_in, index;
  links node, root;

  DEBUGMSG(debug_bayesian, "inital_net()\n");
  BN->E->count = 0;

  for (i = 0; i < BN->G->V; i++) {
    for (j = 0; j < NODE_VALUES; j++) {
      BN->P[i]->lambda_value[j] = 1.0; 				// compute 'lambda' values
      DEBUGMSG(debug_bayesian, "ld_v(%d = %s) = %.3lf,   ", i, STATE(j), BN->P[i]->lambda_value[j]);
    }
    DEBUGMSG(debug_bayesian, "\n");
    for (node = parent_reference(BN->G, i), node_counter = 0; node != NULL; node = next_reference(node)) {
      for (j = 0; j < NODE_VALUES; j++) {
        BN->P[i]->lambda_message[node_counter+j] = 1.0;      	// Compute 'lambda' messages
        DEBUGMSG(debug_bayesian, "ld_m_%d(%d = %s) = %.3lf, ", i, link_index(node), STATE(j), BN->P[i]->lambda_message[node_counter+j]);
      }
      DEBUGMSG(debug_bayesian, "\n");
      node_counter += NODE_VALUES;
    }
    DEBUGMSG(debug_bayesian, "\n");
    for (node = child_reference(BN->G, i); node != NULL; node = next_reference(node)) {
      index = link_index(node);
      node_counter = parent_position(BN->G, index, i)*NODE_VALUES;
      for (j = 0; j < NODE_VALUES; j++) {
        BN->P[index]->pi_message[node_counter+j] = 1.0;			// Compute 'pi' messages
        DEBUGMSG(debug_bayesian, "pi_m_%d(%d = %s) = %.3lf, ", index, i, STATE(j), BN->P[index]->pi_message[node_counter+j]);
      }
      DEBUGMSG(debug_bayesian, "\n");
    }
  }

  for (root = root_reference(BN->G); root != NULL; root = next_reference(root)) {
    root_in = link_index(root);
    for (j = 0; j < NODE_VALUES; j++) {
      BN->P[root_in]->pi_value[j] = BN->P[root_in]->post_probabilitiy[j];	// Compute Root node's 'pi' values
      DEBUGMSG(debug_bayesian, "pi_v(%d = %s) = %.3lf,   ", root_in, STATE(j), BN->P[root_in]->pi_value[j]);
    } 
    DEBUGMSG(debug_bayesian, "\n");
    for (node = child_reference(BN->G, root_in); node != NULL; node = next_reference(node)) {
      DEBUGMSG(debug_bayesian, "send_pi_message(%d, %d)\n----------\n", root_in, link_index(node));
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

  for (node = parent_reference(BN->G, node_index); node != NULL; node = next_reference(node)) {
    index = link_index(node);
    if (ismember_Evidence_Set(BN, index) == -1) {
      DEBUGMSG(debug_bayesian, "send_lmda_message(%d, %d)\n", node_index, index);
      send_lambda_message(BN, node_index, index);
    }
  }

  for (node = child_reference(BN->G, node_index); node != NULL; node = next_reference(node)) {
    index = link_index(node);
    DEBUGMSG(debug_bayesian, "send_pi_message(%d, %d)\n", node_index, index);
    send_pi_message(BN, node_index, index);
  }
}


void send_lambda_message(Bayesian_Network BN, int node_child, int node_parent) {
  int i, j, k, num_bits, position_node_parent, index, bit_counter, node_counter;
  int bit[10] = {0};
  links parent, child;
  double temp, temp_, normalizer; 
  position_node_parent = parent_position(BN->G, node_child, node_parent);
  num_bits = parent_number(BN->G, node_child);

  for (i = 0; i < NODE_VALUES; i++) {  		// for each x (values of node_parent)
    BN->P[node_child]->lambda_message[position_node_parent*NODE_VALUES + i] = 0.0;
    for (k = 0; k < NODE_VALUES; k++) { 	// for each y (values of node_child)
      for (j = 0, temp = 0.0, temp_ = 0.0; j < (NODE_VALUES << num_bits); j++) {
        bit_counter = num_bits - 1;
        if ((j % 2) != k) continue;
        if ((1 & (j >> (num_bits - position_node_parent))) != i) continue;
#if FILE_DEBUG
        printf("k = %d, j = %d - ", k, j);
#endif
        while (bit_counter >= 0) {
          bit[bit_counter] = 1 & (j >> (bit_counter + 1));
#if FILE_DEBUG
          printf("%d, ", bit[bit_counter]);
#endif
	  bit_counter--;
        }
#if FILE_DEBUG
        printf("\n");
        printf("1st - %.3lf\n", temp_);
#endif
        temp_ = BN->P[node_child]->probability[j];
        for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
          index = link_index(parent);
          node_counter = parent_position(BN->G, node_child, index)*NODE_VALUES;
	  if (index != node_parent) {
            temp_ *= BN->P[node_child]->pi_message[node_counter + bit[position_node_parent]];
#if FILE_DEBUG
            printf("th[%d + %d]- %.3lf\n", node_counter, bit[position_node_parent], 
                  BN->P[node_child]->pi_message[node_counter + bit[position_node_parent]]);
#endif
          }
        }
        temp += temp_;
      }
#if FILE_DEBUG
      printf("ld_v(%d = %s) = %.3lf\n", node_child, STATE(k), BN->P[node_child]->lambda_value[k]);
#endif
      temp *= BN->P[node_child]->lambda_value[k];
      BN->P[node_child]->lambda_message[position_node_parent*NODE_VALUES + i] += temp;
    }
    DEBUGMSG(debug_bayesian, "ld_m_%d(%d = %s) = %.3lf, ", node_child, node_parent, STATE(i), 
          BN->P[node_child]->lambda_message[position_node_parent*NODE_VALUES + i]);

    BN->P[node_parent]->lambda_value[i] = 1.0;
    for (child = child_reference(BN->G, node_parent); child != NULL; child = next_reference(child)) {
      index = link_index(child);
      BN->P[node_parent]->lambda_value[i] *= BN->P[index]->lambda_message[position_node_parent*NODE_VALUES + i];
    }
    DEBUGMSG(debug_bayesian, "ld_v(%d = %s) = %.3lf\n", node_parent, STATE(i), BN->P[node_parent]->lambda_value[i]);
    BN->P[node_parent]->post_probabilitiy[i] = BN->P[node_parent]->lambda_value[i]*(BN->P[node_parent]->pi_value[i]);
#if FILE_DEBUG
    printf("prob._%d[%d] = %.3lf\n", node_parent, i, BN->P[node_parent]->post_probabilitiy[i]); 
#endif
  }
  DEBUGMSG(debug_bayesian, "\n");

  normalizer = 1.0/(BN->P[node_parent]->post_probabilitiy[0]+BN->P[node_parent]->post_probabilitiy[1]);
  for (i = 0; i < NODE_VALUES; i++) {
    BN->P[node_parent]->post_probabilitiy[i] *= normalizer;
    DEBUGMSG(debug_bayesian, "P(Node_%d = %s) = %.4lf, ", node_parent, STATE(i), BN->P[node_parent]->post_probabilitiy[i]);
  }
  DEBUGMSG(debug_bayesian, "\n");

  for (parent = parent_reference(BN->G, node_parent); parent != NULL; parent = next_reference(parent)) {
    index = link_index(parent);
    if (ismember_Evidence_Set(BN, index) == -1) {
      DEBUGMSG(debug_bayesian, "send_lmda_message(%d, %d)\n", node_parent, index);
      send_lambda_message(BN, node_parent, index);
    }
  }

  for (child = child_reference(BN->G, node_parent); child != NULL; child = next_reference(child)) {
    index = link_index(child);
    if (ismember_Evidence_Set(BN, index) == -1) {
      DEBUGMSG(debug_bayesian, "send_pi_message(%d, %d)\n", node_parent, index);
      send_pi_message(BN, node_parent, index);
    }
  }
}


void send_pi_message(Bayesian_Network BN, int node_parent, int node_child) {
  int i, j, index, bit[8] = {0}, num_bits, bit_counter;
  double temp = 0.0, temp_, normalizer;
  links child, parent;
  int node_counter = parent_position(BN->G, node_child, node_parent)*NODE_VALUES;
  
  for (i = 0; i < NODE_VALUES; i++) {
    temp = BN->P[node_parent]->pi_value[i];
    child = child_reference(BN->G, node_parent);
    for (; child != NULL; child = next_reference(child)) {
      index = link_index(child);
      if (index != node_child)
        temp *= BN->P[index]->lambda_message[i];
    }
    BN->P[node_child]->pi_message[node_counter+i] = temp;        // Compute pi message
    DEBUGMSG(debug_bayesian, "pi_m_%d(%d = %s) = %.3lf, ", node_child, node_parent, STATE(i), 
          BN->P[node_child]->pi_message[node_counter+i]);
  }
  DEBUGMSG(debug_bayesian, "\n");

  if (ismember_Evidence_Set(BN, node_child) == -1) {
    for (i = 0; i < NODE_VALUES; i++) {
      num_bits = parent_number(BN->G, node_child);
      for (j = 0, temp = 0.0, temp_ = 0.0; j < (NODE_VALUES << num_bits); j++) {
        bit_counter = num_bits-1;
	if (j%2 != i) continue;
        printf("bits for %d - ", j);
	while (bit_counter >= 0) {
	  bit[bit_counter] = 1 & (j >> (bit_counter+1));
#if FILE_DEBUG
          printf("%d, ", bit[bit_counter]);
#endif
	  bit_counter--;
	}
        printf("\n");
	temp_ = BN->P[node_child]->probability[j];
#if FILE_DEBUG
        printf("1st - %.3lf\n", temp_);
#endif
	bit_counter = num_bits-1;
        for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
          index = link_index(parent);
          node_counter = parent_position(BN->G, node_child, index)*NODE_VALUES;
          temp_ *= BN->P[node_child]->pi_message[node_counter + bit[bit_counter]];
#if FILE_DEBUG
          printf("th[%d + %d]- %.3lf\n", node_counter, bit[bit_counter], 
                  BN->P[node_child]->pi_message[node_counter + bit[bit_counter]]);
#endif
          bit_counter--;
        }
	temp += temp_;
      }
      BN->P[node_child]->pi_value[i] = temp;
      DEBUGMSG(debug_bayesian, "pi_v(%d = %s) = %.3lf,   ", node_child, STATE(i), BN->P[node_child]->pi_value[i]);
      BN->P[node_child]->post_probabilitiy[i] = BN->P[node_child]->lambda_value[i]*(BN->P[node_child]->pi_value[i]);
    }
    DEBUGMSG(debug_bayesian, "\n");
    normalizer = 1.0/(BN->P[node_child]->post_probabilitiy[0]+BN->P[node_child]->post_probabilitiy[1]);
    for (i = 0; i < NODE_VALUES; i++) {
      BN->P[node_child]->post_probabilitiy[i] *= normalizer;
      DEBUGMSG(debug_bayesian, "P(Node_%d) = %.3lf, ", node_child, BN->P[node_child]->post_probabilitiy[i]);
    }
    DEBUGMSG(debug_bayesian, "\n");
  }
  for (child = child_reference(BN->G, node_child); child != NULL; child = next_reference(child)) {
    DEBUGMSG(debug_bayesian, "send_pi_message(%d, %d)\n", node_child, link_index(child));
    send_pi_message(BN, node_child, link_index(child));
  }

  if (BN->P[node_child]->lambda_value[0] != 1 || BN->P[node_child]->lambda_value[1] != 1) {
    for (parent = parent_reference(BN->G, node_child); parent != NULL; parent = next_reference(parent)) {
      index = link_index(parent);
      if (index != node_parent  && ismember_Evidence_Set(BN, index) == -1) {
        DEBUGMSG(debug_bayesian, "send_lmda_message(%d, %d)\n", node_child, index);
        send_lambda_message(BN, node_child, index);
      }
    }
  }
}
