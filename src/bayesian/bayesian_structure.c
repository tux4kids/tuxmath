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
#include "bayesian_structure.h"
#include "bayesian_network.h"
#include "inference.h"
#include "../globals.h"
#include <math.h>

#define LOCAL_NODES 1
#define GLOBAL_NODES 0

#define BACKBONE_NUMBER_NODES 2
#define BACKBONE_ADDITION_NODES 7
#define BACKBONE_SUBTRACTION_NODES 3
#define BACKBONE_MULTIPLICATION_NODES 17
#define BACKBONE_DIVISION_NODES 15

#define evidence_given_state {0.85, 0.2}
#define backbone_causal {0.8, 0.2}
#define init_prob {0.5}

Bayesian_Network topic_cluster;
Bayesian_Network number_cluster, addition_cluster, subtraction_cluster, 
                 multiplication_cluster, division_cluster;

int nodes_to_lesson[5][50];
int current_sub_topic;
typedef enum{STUDENT_KNOWS, EVIDENCE} node_label;
int *lessons;
int lesson_counter;


/* local function prototypes */
static void init_number_cluster(void);
static void init_addition_cluster(void);
static void init_subtraction_cluster(void);
static void init_multiplication_cluster(void);
static void init_division_cluster(void);
static void check_absorbing_states(double *probability);


/* Initialise the bayesian structures for all    */
/* the topic clusters. Must be called once when  */
/* the user runs training academy in Tuxmath     */
void BS_init_beliefnet() {
    lessons = malloc(num_lessons*sizeof(int));
    lesson_list_probability = (double *)malloc(num_lessons*sizeof(double));
    memset(nodes_to_lesson, -1, 5*50*sizeof(int));
    init_number_cluster();
    init_addition_cluster();
    init_subtraction_cluster();
    init_multiplication_cluster();
    init_division_cluster();
}

static void init_number_cluster() {
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state};
    int i, num_nodes;
    num_nodes = BACKBONE_NUMBER_NODES*(LOCAL_NODES+1);
    DEBUGMSG(debug_bayesian, "Number cluster initialization - %d\n", num_nodes);
    number_cluster = BN_init(BACKBONE_NUMBER_NODES*(LOCAL_NODES+1));
    BN_add_link(number_cluster, 0, 1);
    BN_add_link(number_cluster, 0, 2);
    BN_add_link(number_cluster, 2, 3);
    for (i = 0; i < num_nodes; i++) {
        BN_nodeprobability(number_cluster, i, prob_dist[i]);
    }
    initial_tree(number_cluster);
    lessons[0] = 0;
    lessons[46] = 2;
    nodes_to_lesson[NUMBER_TYPING][0] = 0;
    nodes_to_lesson[NUMBER_TYPING][2] = 46;
    BN_display(number_cluster, 0);
}

static void init_addition_cluster() {
    int i, num_nodes;
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state, 
            backbone_causal, evidence_given_state, backbone_causal, evidence_given_state, backbone_causal,
            evidence_given_state, backbone_causal, evidence_given_state, backbone_causal, evidence_given_state};
    num_nodes = BACKBONE_ADDITION_NODES*(LOCAL_NODES+1);
    DEBUGMSG(debug_bayesian, "Addition cluster initialization - %d\n", num_nodes);
    addition_cluster = BN_init(num_nodes);
    for (i = 0; i < num_nodes-2; i+=2) {
        BN_add_link(addition_cluster, i, i+1); 
        BN_add_link(addition_cluster, i, i+2); 
    }
    BN_add_link(addition_cluster, num_nodes-2, num_nodes-1);
    for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(addition_cluster, i, prob_dist[i]);
    for (i = 1; i < 8; i++) {
        lessons[i] = 2*(i-1);
	nodes_to_lesson[ADDITION][2*(i-1)] = i;
    }
    initial_tree(addition_cluster);
    //BN_display(addition_cluster, 1);
}

static void init_subtraction_cluster() {
    int i, num_nodes;
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state, 
                             backbone_causal,evidence_given_state};
    num_nodes = BACKBONE_SUBTRACTION_NODES*(LOCAL_NODES+1);
    DEBUGMSG(debug_bayesian, "Subtraction cluster initialization - %d\n", num_nodes);
    subtraction_cluster = BN_init(num_nodes);
    for (i = 0; i < num_nodes; i += 2)
        BN_add_link(subtraction_cluster, i, i+1);
    BN_add_link(subtraction_cluster, 0, 2);
    BN_add_link(subtraction_cluster, 2, 4);
    for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(subtraction_cluster, i, prob_dist[i]);
    lessons[8] = 0;
    lessons[9] = 2;
    lessons[10] = 4;
    nodes_to_lesson[SUBTRACTION][0] = 8;
    nodes_to_lesson[SUBTRACTION][2] = 9;
    nodes_to_lesson[SUBTRACTION][4] = 10;

    initial_tree(subtraction_cluster);
    //BN_display(subtraction_cluster, 1);
}

static void init_multiplication_cluster() {
    int i, num_nodes = BACKBONE_MULTIPLICATION_NODES*(LOCAL_NODES+1);
    double prob_dist[num_nodes][2];
    double evid_state[] = evidence_given_state;
    double back_causal[] = backbone_causal;
    double init_probab[] = init_prob;
    multiplication_cluster = BN_init(num_nodes);
    DEBUGMSG(debug_bayesian, "Multiplication cluster initialization - %d\n", num_nodes);
    memcpy(&prob_dist[0], init_probab, NODE_VALUES*sizeof(double));
    for (i = 1; i < num_nodes; i += 2) {
        memcpy(&prob_dist[i], evid_state, NODE_VALUES*sizeof(double));
    }
    for (i = 2; i < num_nodes; i += 2) {
        memcpy(&prob_dist[i], back_causal, NODE_VALUES*sizeof(double));
        BN_add_link(multiplication_cluster, i-2, i-1);
        BN_add_link(multiplication_cluster, i-2, i);
    }
    BN_add_link(multiplication_cluster, num_nodes-2, num_nodes-1);
    for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(multiplication_cluster, i, prob_dist[i]);
    for (i = 0; i < BACKBONE_MULTIPLICATION_NODES; i++) {
        lessons[i+12] =  (LOCAL_NODES+1)*i;
	nodes_to_lesson[MULTIPLICATION][(LOCAL_NODES+1)*i] = i+12;
    }
    initial_tree(multiplication_cluster);
    //BN_display(multiplication_cluster, 1);
}

static void init_division_cluster() {
    int i, num_nodes = BACKBONE_DIVISION_NODES*(LOCAL_NODES+1);
    double prob_dist[num_nodes][2];
    double evid_state[] = evidence_given_state;
    double back_causal[] = backbone_causal;
    double init_probab[] = init_prob;
    division_cluster = BN_init(num_nodes);
    DEBUGMSG(debug_bayesian, "Division cluster initialization - %d\n", num_nodes);
    memcpy(&prob_dist[0], init_probab, NODE_VALUES*sizeof(double));
    for (i = 1; i < num_nodes; i += 2) {
        memcpy(&prob_dist[i], evid_state, NODE_VALUES*sizeof(double));
    }
    for (i = 2; i < num_nodes; i += 2) {
        memcpy(&prob_dist[i], back_causal, NODE_VALUES*sizeof(double));
        BN_add_link(division_cluster, i-2, i-1);
        BN_add_link(division_cluster, i-2, i);
    }
    BN_add_link(division_cluster, num_nodes-2, num_nodes-1);
    for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(division_cluster, i, prob_dist[i]);
    for (i = 0; i < BACKBONE_DIVISION_NODES; i++) {
        lessons[i+30] =  (LOCAL_NODES+1)*i;
	nodes_to_lesson[DIVISION][(LOCAL_NODES+1)*i] = i+30;
    }
    initial_tree(division_cluster);    
    //BN_display(division_cluster, 1);
}

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

/* Sets the current topic in the corresponding   */
/* bayesian topic cluster. Must be set every time*/
/* a user starts a lesson based game             */
/* @param int - The lesson's index               */
void BS_set_topic(int sub_topic_index) {
    current_sub_topic = sub_topic_index;
}

/* The call to this function must be made from   */
/* within the game, with either TRUE or FALSE    */
/* based on whether the user answers the question*/
/* correctly or incorrectly.                     */
void BS_update_cluster(node_state value) {
    const int type = lesson_list_topics[current_sub_topic];
    int cluster_index = lessons[current_sub_topic]+1;
    int i;
    switch (type) {
        case NUMBER_TYPING:
            update_tree(number_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1)
                check_absorbing_states((number_cluster->P[i]->post_probabilitiy));
            initial_tree(number_cluster);
            lesson_list_probability[current_sub_topic] = number_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(number_cluster, 0);
            break;
        case ADDITION:
            update_tree(addition_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1)
                check_absorbing_states((addition_cluster->P[i]->post_probabilitiy));
            initial_tree(addition_cluster);
	    DEBUGMSG(debug_bayesian, "cluster_index = %d, sub_topic = %d\n", cluster_index, current_sub_topic);
            lesson_list_probability[current_sub_topic] = addition_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(addition_cluster, 0);
            break;
        case SUBTRACTION:
            update_tree(subtraction_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1)
                check_absorbing_states((subtraction_cluster->P[i]->post_probabilitiy));
            initial_tree(subtraction_cluster);
            lesson_list_probability[current_sub_topic] = subtraction_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(subtraction_cluster, 0);
            break;
        case MULTIPLICATION:
            update_tree(multiplication_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1)
                check_absorbing_states((multiplication_cluster->P[i]->post_probabilitiy));
            initial_tree(multiplication_cluster);
            lesson_list_probability[current_sub_topic] = multiplication_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(multiplication_cluster, 0);
            break;
        case DIVISION:
            update_tree(division_cluster, cluster_index, value);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1)
                check_absorbing_states((division_cluster->P[i]->post_probabilitiy));
            initial_tree(division_cluster);
            lesson_list_probability[current_sub_topic] = division_cluster->P[cluster_index-1]->post_probabilitiy[0];
            BN_display(division_cluster, 0);
            break; 
        default:
            DEBUGMSG(debug_bayesian, "Lesson does not match any topics\n");
            break;
    }
    DEBUGMSG(debug_bayesian, "Current node-index - %d\n", cluster_index);
    DEBUGMSG(debug_bayesian, "Lesson[%d] probability - %.2lf\n", current_sub_topic, lesson_list_probability[current_sub_topic]);
}

/* Return the next lesson in the topic to which  */
/* the 'lesson' parameter belongs.               */
/* @Param int lesson - the lesson index          */
/* @Param const int type - topic to which lesson */
/*   belongs                                     */
/* @Return int - If exists, index of next lesson,*/
/* -1 otherwise                                  */
int BS_next_lesson(int lesson, const int type) {
  int node = lessons[lesson]+2;
  return nodes_to_lesson[type][node];
}
