/* bayesian_structure.h
 *
 *
 *   Copyright (C) 2011.
 *   Author(s):  Siddharth Kothari
 *   Project email: <tuxmath-devel@lists.sourceforge.net>
 *   Project website: http://tux4kids.alioth.debian.org
 *
 *   bayesian_structure.h is part of "Tux, of Math Command", a.k.a. "tuxmath".
 *
 *   Tuxmath is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tuxmath is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.⋅⋅
 */

#ifndef BAYESIAN_STRUCTURE_H
#define BAYESIAN_STRUCTURE_H

#define LOCAL_NODES 4
#define GLOBAL_NODES 1

#define BACKBONE_NUMBER_NODES 2
#define BACKBONE_ADDITION_NODES 7
#define BACKBONE_SUBTRACTION_NODES 3
#define BACKBONE_MULTIPLICATION_NODES 17
#define BACKBONE_DIVISION_NODES 15

typedef enum {
    TRUE,
    FALSE
} node_state;

typedef struct bayesian_node *Bayesian_node;

struct bayesian_node {
    int node_index;
    int parents[2];
    double probability[4];
};

/* Initialise the bayesian structures for all    */
/* the topic clusters. Must be called once when  */
/* the user runs training academy in Tuxmath     */
void BS_init_beliefnet();

/* Sets the current topic in the corresponding   */
/* bayesian topic cluster. Must be set every time*/
/* a user starts a lesson based game             */
/* @param int - The lesson's index               */
void BS_set_topic(int topic_index);

/* The call to this function must be made from   */
/* within the game, with either TRUE or FALSE    */
/* based on whether the user answers the question*/
/* correctly or incorrectly.                     */
/* @param node_state - TRUE, or FALSE            */
void BS_update_cluster(node_state value);

/* Return the next lesson in the topic to which  */
/* the 'lesson' parameter belongs.               */
/* @Param int lesson - the lesson index          */
/* @Param const int type - topic to which lesson */
/*   belongs                                     */
/* @Return int - If exists, index of next lesson,*/
/* -1 otherwise                                  */
int BS_next_lesson(int lesson, const int type);

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
int BS_read(Bayesian_node *cluster_node, const int type, int back_nodes);

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
Bayesian_node* BS_write(const int type);

int BS_is_new_profile(void);
#endif
