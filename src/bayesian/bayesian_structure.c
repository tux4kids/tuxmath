// {{{ Copyright
/* bayesian_structure.c
 * ⋅⋅⋅
 *     bayesian_structure.c acts as an interface between Tuxmath and the
 *     functions for initializing and inferencing on bayesian networks. The
 *     topology of the belief-net backbone structure is specified in the local
 *     prototypes.
 *     Copyright (C) 2011.
 *     Author(s):  Siddharth Kothari
 *     Project email: <tuxmath-devel@lists.sourceforge.net>
 *     Project website: http://tux4kids.alioth.debian.org
 *
 *     bayesian_structure.c is part of "Tux, of Math Command", a.k.a. "tuxmath".
 *
 *     Tuxmath is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     Tuxmath is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.⋅⋅
 */
// "}}}

// {{{ Header files
#include "bayesian_structure.h"
#include "bayesian_network.h"
#include "inference_poly.h"
#include "../globals.h"
#include "../fileops.h"
#include <math.h>
//  }}}

// {{{ #defines
#define evidence_given_state {0.85, 0.2}
#define backbone_causal {0.8, 0.2}
#define init_prob {0.5}
#define topic_interest {0.2, 0.5}
#define challenge_acceptance {0.8, 0.2}
// }}}

//{{{ Global file variables
Bayesian_Network topic_cluster;
Bayesian_Network number_cluster, addition_cluster, subtraction_cluster,
                 multiplication_cluster, division_cluster;
int nodes_to_lesson[5][100];
int current_sub_topic;
typedef enum{STUDENT_KNOWS, EVIDENCE} node_label;
int *lessons;
int lesson_counter;
int new_profile = 0;
//  }}}

/* {{{ local function prototypes */
static void init_number_cluster(void);
static void init_addition_cluster(void);
static void init_subtraction_cluster(void);
static void init_multiplication_cluster(void);
static void init_division_cluster(void);
static double** add_conditional_links(int backbone_nodes, Bayesian_Network);
static void check_absorbing_states(double *probability);
//  }}}

/* Initialise the bayesian structures for all    */
/* the topic clusters. Must be called once when  */
/* the user runs training academy in Tuxmath     */
//  {{{ void BS_init_beliefnet()
void BS_init_beliefnet() {
    DEBUGMSG(debug_bayesian, "Call read_lesson_proficiency\n");
    new_profile = read_lesson_proficiency();
    DEBUGMSG(debug_bayesian, "Return read_lesson_proficiency-  %d\n", new_profile);
    lessons = malloc(num_lessons*sizeof(int));
    lesson_list_probability = (double *)malloc(num_lessons*sizeof(double));
    memset(nodes_to_lesson, -1, 5*100*sizeof(int));
    init_number_cluster();
    init_addition_cluster();
    init_subtraction_cluster();
    init_multiplication_cluster();
    init_division_cluster();
}
//  }}}

//  {{{ static void init_number_cluster()
static void init_number_cluster() {
    int i, num_nodes = BACKBONE_NUMBER_NODES*LOCAL_NODES;
    double **prob_dist;
    DEBUGMSG(debug_bayesian, "Number cluster initialization - %d\n", num_nodes);
    if (new_profile) {
      number_cluster = BN_init(num_nodes);
      prob_dist = add_conditional_links(BACKBONE_NUMBER_NODES, number_cluster);
      for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(number_cluster, i, prob_dist[i]);
      initial_net(number_cluster);
      BN_display(number_cluster, 0);
    }
    lessons[0] = 0;
    lessons[46] = 4;
    nodes_to_lesson[NUMBER_TYPING][0] = 0;
    nodes_to_lesson[NUMBER_TYPING][4] = 46;
}
//  }}}

//  {{{ static void init_addition_cluster()
static void init_addition_cluster() {
    int i, num_nodes = BACKBONE_ADDITION_NODES*LOCAL_NODES;
    double **prob_dist;

    DEBUGMSG(debug_bayesian, "Addition cluster initialization - %d\n", num_nodes);
    if (new_profile) {
      addition_cluster = BN_init(num_nodes);
      prob_dist = add_conditional_links(BACKBONE_ADDITION_NODES, addition_cluster);
      for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(addition_cluster, i, prob_dist[i]);
      initial_net(addition_cluster);
      BN_display(addition_cluster, 1);
    }
    for (i = 1; i < 8; i++) {
      lessons[i] = LOCAL_NODES*(i-1);
      nodes_to_lesson[ADDITION][LOCAL_NODES*(i-1)] = i;
    }
}
//  }}}

//  {{{ static void init_subtraction_cluster()
static void init_subtraction_cluster() {
    int i, num_nodes = BACKBONE_SUBTRACTION_NODES*LOCAL_NODES;
    double **prob_dist;

    DEBUGMSG(debug_bayesian, "Subtraction cluster initialization - %d\n", num_nodes);
    if (new_profile) {
      subtraction_cluster = BN_init(num_nodes);
      prob_dist = add_conditional_links(BACKBONE_SUBTRACTION_NODES, subtraction_cluster);
      for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(subtraction_cluster, i, prob_dist[i]);
      initial_net(subtraction_cluster);
      BN_display(subtraction_cluster, 1);
    }

    for (i = 8; i <= 10; i++) {
      lessons[i] = NODE_VALUES*(i-8);
      nodes_to_lesson[SUBTRACTION][NODE_VALUES*(i-8)] = i;
    }
    lessons[47] = NODE_VALUES*4;
    lessons[50] = NODE_VALUES*5;
    lessons[51] = NODE_VALUES*6;
    lessons[53] = NODE_VALUES*7;
    nodes_to_lesson[SUBTRACTION][NODE_VALUES*4] = 47;
    nodes_to_lesson[SUBTRACTION][NODE_VALUES*5] = 50;
    nodes_to_lesson[SUBTRACTION][NODE_VALUES*6] = 51;
    nodes_to_lesson[SUBTRACTION][NODE_VALUES*7] = 53;

}
//  }}}

//  {{{ static void init_multiplication_cluster()
static void init_multiplication_cluster() {
    int i, num_nodes = BACKBONE_MULTIPLICATION_NODES*LOCAL_NODES;
    double **prob_dist;
    DEBUGMSG(debug_bayesian, "Multiplication cluster initialization - %d\n", num_nodes);
    if (new_profile) {
      multiplication_cluster = BN_init(num_nodes);
      prob_dist = add_conditional_links(BACKBONE_MULTIPLICATION_NODES, multiplication_cluster);
      for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(multiplication_cluster, i, prob_dist[i]);
      initial_net(multiplication_cluster);
      BN_display(multiplication_cluster, 1);
    }
    for (i = 0; i < BACKBONE_MULTIPLICATION_NODES; i++) {
      lessons[i+12] =  LOCAL_NODES*i;
      nodes_to_lesson[MULTIPLICATION][LOCAL_NODES*i] = i+12;
    }
}
//  }}}

//  {{{ static void init_division_cluster()
static void init_division_cluster() {
    int i, num_nodes = BACKBONE_DIVISION_NODES*LOCAL_NODES;
    double **prob_dist;
    DEBUGMSG(debug_bayesian, "Division cluster initialization - %d\n", num_nodes);
    if (new_profile) {
      division_cluster = BN_init(num_nodes);
      prob_dist = add_conditional_links(BACKBONE_DIVISION_NODES, division_cluster);
      for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(division_cluster, i, prob_dist[i]);
      initial_net(division_cluster);
      BN_display(division_cluster, 1);
    }
    for (i = 0; i < BACKBONE_DIVISION_NODES; i++) {
      lessons[i+30] =  LOCAL_NODES*i;
      nodes_to_lesson[DIVISION][LOCAL_NODES*i] = i+30;
    }
}
//  }}}

//  {{{ static double** add_conditional_links(int, Bayesian_Network)
static double** add_conditional_links(int backbone_nodes, Bayesian_Network cluster) {
  int i, node_val;
  double evidence_state[] = evidence_given_state;
  double back_causal[] = backbone_causal;
  double topic_int[] = topic_interest;
  double challenge_level[] = challenge_acceptance;
  double init_probab[] = init_prob;
  double **probability_distribution;
  probability_distribution = (double **)malloc(backbone_nodes*LOCAL_NODES*sizeof(double*));
  for (i = 0; i < backbone_nodes*LOCAL_NODES; i++)
    probability_distribution[i] = (double *)malloc(2*sizeof(double));

  for (i = 0; i < backbone_nodes; i++) {
    node_val = i*LOCAL_NODES;
    // specify conditional_probabilities for each node
    if (node_val == 0)
      memcpy(&probability_distribution[node_val][0], init_probab, sizeof(double));
    else
      memcpy(&probability_distribution[node_val][0], back_causal, 2*sizeof(double));
    memcpy(&probability_distribution[node_val+1][0], evidence_state, 2*sizeof(double));
    memcpy(&probability_distribution[node_val+2][0], topic_int, 2*sizeof(double));
    memcpy(&probability_distribution[node_val+3][0], challenge_level, 2*sizeof(double));

    // specify the link directions
    BN_add_link(cluster, node_val, node_val+1);
    BN_add_link(cluster, node_val, node_val+2);
    BN_add_link(cluster,node_val +2, node_val+3);
    if (i != backbone_nodes-1)
      BN_add_link(cluster, node_val, node_val+LOCAL_NODES);
  }
  return probability_distribution;
}
//  }}}

// Do not allow a node to an absolute state of 1 or 0
//  {{{ static void check_absorbing_states(double *)
static void check_absorbing_states(double *probability) {
    if (fabs(probability[0] - 1.00) < 0.00001) {
        probability[0] = 0.99999;
        probability[1] = 0.00001;
        return;
    }
    if (fabs(probability[0] - 0.00) < 0.00001) {
        probability[0] = 0.00001;
        probability[1] = 0.99999;
    }
}
//  }}}

/* Sets the current topic in the corresponding   */
/* bayesian topic cluster. Must be set every time*/
/* a user starts a lesson based game             */
/* @param int - The lesson's index               */
//  {{{ void BS_set_topic(int)
void BS_set_topic(int sub_topic_index) {
    current_sub_topic = sub_topic_index;
}
//  }}}


int BS_is_new_profile(void) {
    return new_profile;
}

/* The call to this function must be made from   */
/* within the game, with either TRUE or FALSE    */
/* based on whether the user answers the question*/
/* correctly or incorrectly.                     */
//  {{{ void BS_update_cluster(node_state)
void BS_update_cluster(node_state value) {
    const int type = lesson_list_topics[current_sub_topic];
    int cluster_index = lessons[current_sub_topic]+1;
    DEBUGMSG(debug_bayesian, "%d, %d\n", current_sub_topic, cluster_index);
    int i;
    switch (type) {
        case NUMBER_TYPING:
            update_net(number_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*LOCAL_NODES; i += LOCAL_NODES)
                check_absorbing_states((number_cluster->P[i]->post_probabilitiy));
            initial_net(number_cluster);
            lesson_list_probability[current_sub_topic] = number_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(number_cluster, 0);
            break;
        case ADDITION:
            update_net(addition_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_ADDITION_NODES*LOCAL_NODES; i += LOCAL_NODES)
                check_absorbing_states((addition_cluster->P[i]->post_probabilitiy));
            initial_net(addition_cluster);
	    DEBUGMSG(debug_bayesian, "cluster_index = %d, sub_topic = %d\n", cluster_index, current_sub_topic);
            lesson_list_probability[current_sub_topic] = addition_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(addition_cluster, 0);
            break;
        case SUBTRACTION:
            update_net(subtraction_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_SUBTRACTION_NODES*LOCAL_NODES; i += LOCAL_NODES)
                check_absorbing_states((subtraction_cluster->P[i]->post_probabilitiy));
            initial_net(subtraction_cluster);
            lesson_list_probability[current_sub_topic] = subtraction_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(subtraction_cluster, 0);
            break;
        case MULTIPLICATION:
            update_net(multiplication_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_MULTIPLICATION_NODES*LOCAL_NODES; i += LOCAL_NODES)
                check_absorbing_states((multiplication_cluster->P[i]->post_probabilitiy));
            initial_net(multiplication_cluster);
            lesson_list_probability[current_sub_topic] = multiplication_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(multiplication_cluster, 0);
            break;
        case DIVISION:
            update_net(division_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_DIVISION_NODES*LOCAL_NODES; i += LOCAL_NODES)
                check_absorbing_states((division_cluster->P[i]->post_probabilitiy));
            initial_net(division_cluster);
            lesson_list_probability[current_sub_topic] = division_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(division_cluster, 0);
            break;
        default:
            DEBUGMSG(debug_bayesian, "Lesson does not match any topics\n");
            break;
    }
    DEBUGMSG(debug_bayesian, "Current node-index - %d\n", cluster_index-1);
    DEBUGMSG(debug_bayesian, "Lesson[%d] probability - %.4lf\n", current_sub_topic, lesson_list_probability[current_sub_topic]);
    lesson_list_probability[current_sub_topic] /= pow(0.8, (cluster_index-1)/LOCAL_NODES);
    if (lesson_list_probability[current_sub_topic] >= 1.00000)
       lesson_list_probability[current_sub_topic] = 0.99999;
    DEBUGMSG(debug_bayesian, "Lesson[%d] probability - %.4lf\n", current_sub_topic, lesson_list_probability[current_sub_topic]);
}
//  }}}

/* Return the next lesson in the topic to which  */
/* the 'lesson' parameter belongs.               */
/* @Param int lesson - the lesson index          */
/* @Param const int type - topic to which lesson */
/*   belongs                                     */
/* @Return int - If exists, index of next lesson,*/
/* -1 otherwise                                  */
//  {{{ int BS_next_lesson(int, const int)
int BS_next_lesson(int lesson, const int type) {
  int node = lessons[lesson]+LOCAL_NODES;
  if (nodes_to_lesson[type][node] == -1 && type <= MULTIPLICATION) {
    return nodes_to_lesson[type+1][0];
  }
  return nodes_to_lesson[type][node];
}
//  }}}


/* BS_read() parses out the structure of the Bay-*
 * esian network from the file and initializes it*
 * It maintains Bayesian_Network model of a stud-*
 * ent between consecutive sessions.             *
 * @Param - Bayesian_node * => Array of struct b-*
 * ayesian_node retrieved from lesson-proficiency*
 * file.
 * @Param - const int => Type of the structure   *
 * (Number, Addition, Sub., Mult. or Division  ) *
 * @Param - int => Backbone nodes in the BBN     *
 * @Return - int => 0 for success, -1 for failure*/
// {{{ int BS_read(Bayesian_node *cluster_node, const int type, int back_nodes)
int BS_read(Bayesian_node *cluster, const int type, int back_nodes) {
  int i, j, num_nodes = back_nodes*LOCAL_NODES;
  Bayesian_Network BN;
  double **probability_distribution;
  static int k = 0;
  probability_distribution = (double **)malloc(num_nodes*sizeof(double*));
  for (i = 0; i < num_nodes; i++)
    probability_distribution[i] = (double *)malloc(2*sizeof(double));

  BN = BN_init(num_nodes);
  for (i = 0; i < num_nodes; i++) {
    for (j = 0; j < 2; j++) {
      if (cluster[i]->parents[j] >= -1 && cluster[i]->parents[j] < num_nodes) { // To prevent garbage values
        if (cluster[i]->parents[j] != -1) {   // -1 => not present
          DEBUGMSG(debug_bayesian, "link => (%d, %d), ", cluster[i]->parents[j], cluster[i]->node_index);
          BN_add_link(BN, cluster[i]->parents[j], cluster[i]->node_index);
        }
      }
      else
         return -1;
    }
    DEBUGMSG(debug_bayesian, "\n");
    for (j = 0; j < 4; j++) {
       if (cluster[i]->probability[j] >= 0.00) {
         DEBUGMSG(debug_bayesian, "P[%d][%d] = %.3lf, ", i, j, cluster[i]->probability[j]);
         probability_distribution[cluster[i]->node_index][j] = cluster[i]->probability[j];
       }
       else if (cluster[i]->probability[j] < -1 || cluster[i]->probability[j] > 1)
          return -1;  // Garbage values
       else
          break;
    }
    DEBUGMSG(debug_bayesian, "\n");
  }
  for (i = 0; i < num_nodes; i++)
    BN_nodeprobability(BN, i, probability_distribution[i]);
  initial_net(BN);
  BN_display(BN, 1);
  if (type == NUMBER_TYPING)
     number_cluster = BN;
  else if (type == ADDITION)
     addition_cluster = BN;
  else if (type == MULTIPLICATION)
     multiplication_cluster = BN;
  else if (type == SUBTRACTION)
     subtraction_cluster = BN;
  else if (type == DIVISION)
     division_cluster = BN;
  // Free the memory now
  for (i = 0; i < num_nodes; i++) {
     free(probability_distribution[i]);
     probability_distribution[i] = NULL;
  }
  return 0;
}
// }}}

/* BS_write writes out the structure of the BBN  *
 * in a format compatible with BS_read. Informa- *
 * tion about a particular node is written in the*
 * following format: %d %d %d |%lf %lf %lf %lf   *
 * 1st number is the index of the node, 2nd and  *
 * 3rd numbers are indices of it's parent nodes, *
 * -1 in case there are less than 2 parent nodes.*
 * | acts as a delimiter. The last 4 values are  *
 * the conditional probability values of the node*
 * The number of values needed is 1 << num_par.  *
 * The redundent values are filled with -1.000000*
 * @Param - const int => Type of BBN structure   *
 * @Return - Bayesian_node* => Array of struct b-*
 * ayesian_node, each array member containing    *
 * information related to a node in the BBN.     */
//  {{{ int BS_Write(const int)
Bayesian_node* BS_write(const int type) {
  Bayesian_node *cluster_node = NULL;
  Bayesian_Network BN = NULL;
  int num_nodes = 0, i = 0, j, index;
  links node;
  switch(type) {
    case NUMBER_TYPING:
      BN = number_cluster;
      num_nodes = BACKBONE_NUMBER_NODES;
      break;
    case ADDITION:
      BN = addition_cluster;
      num_nodes = BACKBONE_ADDITION_NODES;
      break;
    case SUBTRACTION:
      BN = subtraction_cluster;
      num_nodes = BACKBONE_SUBTRACTION_NODES;
      break;
    case MULTIPLICATION:
      BN = multiplication_cluster;
      num_nodes = BACKBONE_MULTIPLICATION_NODES;
      break;
    case DIVISION:
      BN = division_cluster;
      num_nodes = BACKBONE_DIVISION_NODES;
      break;
  }
  num_nodes *= LOCAL_NODES;
  cluster_node = malloc(num_nodes*sizeof(Bayesian_node));
  DEBUGMSG(debug_bayesian, "num_nodes = %d\n", num_nodes);
  for (i = 0; i < num_nodes; i++) {
    cluster_node[i] = malloc(sizeof(*(cluster_node[i])));
    cluster_node[i]->node_index = i;
    if (BN != NULL) {
      for (node = parent_reference(BN->G, i), j = 0; node != NULL; node = next_reference(node)) {
        index = link_index(node);
        cluster_node[i]->parents[j++] = index;
      }
      for (; j < 2; j++)
        cluster_node[i]->parents[j] = -1;
      for (j = 0; j < BN->P[i]->number; j += 2)
        cluster_node[i]->probability[j/2] = BN->P[i]->probability[j];
      for (; j < 8; j+=2)
        cluster_node[i]->probability[j/2] = -1.0;
    }
  }
  return cluster_node;
}
//  }}}
