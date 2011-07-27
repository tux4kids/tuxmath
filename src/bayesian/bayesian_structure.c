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

Bayesian_Network topic_cluster;
Bayesian_Network number_cluster, addition_cluster, subtraction_cluster, 
                 multiplication_cluster, division_cluster;

int current_sub_topic;
typedef enum{STUDENT_KNOWS, EVIDENCE} node_label;
int *lessons;
int lesson_counter;
#define evidence_given_state {0.85, 0.2}
#define backbone_causal {0.8, 0.2}
#define init_prob {0.5}


/* local function prototypes */
static void init_number_cluster(void);
static void init_addition_cluster(void);
static void init_subtraction_cluster(void);
static void init_multiplication_cluster(void);
static void init_division_cluster(void);
static void update_cluster(const int type, int index, node_state state);
static void check_absorbing_states(double *probability);

void BS_init_beliefnet() {
    lessons = malloc(num_lessons*sizeof(int));
    init_number_cluster();
    init_addition_cluster(); 
    init_subtraction_cluster();
    init_multiplication_cluster();
    init_division_cluster();
}    



static void init_number_cluster() {
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state};
    int i, num_nodes;
    printf("Number cluster initialization - %d\n", num_nodes);
    num_nodes = BACKBONE_NUMBER_NODES*(LOCAL_NODES+1);
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
}

static void init_addition_cluster() {
    int i, num_nodes;
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state, 
            backbone_causal, evidence_given_state, backbone_causal, evidence_given_state, backbone_causal,
            evidence_given_state, backbone_causal, evidence_given_state, backbone_causal, evidence_given_state};
    num_nodes = BACKBONE_ADDITION_NODES*(LOCAL_NODES+1);
    printf("Addition cluster initialization - %d\n", num_nodes);
    addition_cluster = BN_init(num_nodes);
    for (i = 0; i < num_nodes-2; i+=2) {
        BN_add_link(addition_cluster, i, i+1); 
        BN_add_link(addition_cluster, i, i+2); 
    }
    BN_add_link(addition_cluster, num_nodes-2, num_nodes-1);
    for (i = 0; i < num_nodes; i++)
        BN_nodeprobability(addition_cluster, i, prob_dist[i]);
    for (i = 1; i < 8; i++)
        lessons[i] = 2*(i-1);
    initial_tree(addition_cluster);
}


static void init_subtraction_cluster() {
    int i, num_nodes;
    double prob_dist[][2] = {init_prob, evidence_given_state, backbone_causal, evidence_given_state, 
                             backbone_causal,evidence_given_state};
    num_nodes = BACKBONE_SUBTRACTION_NODES*(LOCAL_NODES+1);
    printf("Subtraction cluster initialization - %d\n", num_nodes);
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
    initial_tree(subtraction_cluster);
    BN_display(subtraction_cluster, 1);
}


static void init_multiplication_cluster() {
    int i, num_nodes = BACKBONE_MULTIPLICATION_NODES*(LOCAL_NODES+1);
    double prob_dist[num_nodes][2];
    double evid_state[] = evidence_given_state;
    double back_causal[] = backbone_causal;
    double init_probab[] = init_prob;
    multiplication_cluster = BN_init(num_nodes);
    printf("Multiplication cluster initialization - %d\n", num_nodes);
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
    for (i = 0; i < BACKBONE_MULTIPLICATION_NODES; i++)
        lessons[i+12] =  (LOCAL_NODES+1)*i;
    initial_tree(multiplication_cluster);
    BN_display(multiplication_cluster, 1);
}


static void init_division_cluster() {
    int i, num_nodes = BACKBONE_DIVISION_NODES*(LOCAL_NODES+1);
    double prob_dist[num_nodes][2];
    double evid_state[] = evidence_given_state;
    double back_causal[] = backbone_causal;
    double init_probab[] = init_prob;
    division_cluster = BN_init(num_nodes);
    printf("Division cluster initialization - %d\n", num_nodes);
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
    for (i = 0; i < BACKBONE_DIVISION_NODES; i++)
        lessons[i+30] =  (LOCAL_NODES+1)*i;
    initial_tree(division_cluster);    
    BN_display(division_cluster, 1);
}


void BS_set_topic(int sub_topic_index) {
    current_sub_topic = sub_topic_index;
}


void BS_update_cluster(node_state value) {
    update_cluster(lesson_list_topics[current_sub_topic], lessons[current_sub_topic]+1, value);
}


static void check_absorbing_states(double *probability) {
    if (fabs(probability[0] - 1.00) < 0.01) {
        probability[0] = 0.99;
        probability[1] = 0.01;
        return;
    }
    if (fabs(probability[0] - 0.00) < 0.01) { 
        probability[0] = 0.01;
        probability[1] = 0.99;
    }
}


static void update_cluster(const int type, int cluster_index, node_state state) {
    int i, j;
    switch (type) {
        case NUMBER_TYPING:
            update_tree(number_cluster, cluster_index, state);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1) {
                check_absorbing_states((number_cluster->P[i]->post_probabilitiy));
                for (j = 0; j < NODE_VALUES; j++)
                    number_cluster->P[i]->probability[j] = number_cluster->P[i]->post_probabilitiy[j];
            }
            initial_tree(number_cluster);
            BN_display(number_cluster, 0);
            break;
        case ADDITION:
            update_tree(addition_cluster, cluster_index, state);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1) {
                check_absorbing_states((addition_cluster->P[i]->post_probabilitiy));
                for (j = 0; j < NODE_VALUES; j++) 
                    addition_cluster->P[i]->probability[j] = addition_cluster->P[i]->post_probabilitiy[j];
            }
            initial_tree(addition_cluster);
            BN_display(addition_cluster, 0);
            break;
        case SUBTRACTION:
            update_tree(subtraction_cluster, cluster_index, state);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1) {
                check_absorbing_states((subtraction_cluster->P[i]->post_probabilitiy));
                for (j = 0; j < NODE_VALUES; j++) 
                    subtraction_cluster->P[i]->probability[j] = subtraction_cluster->P[i]->post_probabilitiy[j];
            }
            initial_tree(subtraction_cluster);
            BN_display(subtraction_cluster, 0);
            break;
        case MULTIPLICATION:
            update_tree(multiplication_cluster, cluster_index, state);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1) {
                check_absorbing_states((multiplication_cluster->P[i]->post_probabilitiy));
                for (j = 0; j < NODE_VALUES; j++) 
                    multiplication_cluster->P[i]->probability[j] = multiplication_cluster->P[i]->post_probabilitiy[j];
            }
            initial_tree(multiplication_cluster);
            BN_display(multiplication_cluster, 0);
            break;
        case DIVISION:
            update_tree(division_cluster, cluster_index, state);
            for (i = 0; i < BACKBONE_NUMBER_NODES*(LOCAL_NODES+1); i += LOCAL_NODES+1) {
                check_absorbing_states((division_cluster->P[i]->post_probabilitiy));
                for (j = 0; j < NODE_VALUES; j++) 
                    division_cluster->P[i]->probability[j] = division_cluster->P[i]->post_probabilitiy[j];
            }
            initial_tree(division_cluster);
            BN_display(division_cluster, 0);
            break; 
        default:
            printf("Lesson does not match any topics\n");
            break;
    }
    printf("Current node-index - %d\n", cluster_index);
}
