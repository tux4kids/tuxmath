#include "bayesian_structure.h"
#include "bayesian_network.h"
#include "inference.h"

#define LOCAL_NODES 2

Bayesian_Network cluster;

int current_sub_topic;
typedef enum{STUDENT_KNOWS, EVIDENCE} node_label;

void BN_init_cluster(int sub_topic_index) {
    
    current_sub_topic = sub_topic_index;

    cluster = BN_init(LOCAL_NODES);
    BN_add_link(cluster, 0, 1);

    // n1[0] == p(student_knows = True) = 0.5
    double n1[] = {0.5};
    // n2[0] = p(correct| SK=True) = 0.95; n2[1] = p(correct| SK=False)=0.2;
    double n2[] = {0.95, 0.2};
    // Sets the initial probabilities for SK (student_knows) node
    BN_nodeprobability(cluster, STUDENT_KNOWS, n1);
    // Sets the initial probabilities for evidence node
    BN_nodeprobability(cluster, EVIDENCE, n2);
    initial_tree(cluster);
}

void BN_update_cluster(node_state value) {
    int i;
    if (value == TRUE) {
        update_tree(cluster, EVIDENCE, TRUE);
    } else {
        update_tree(cluster, EVIDENCE, FALSE);
    }
    // change the initial_prob. to post_prob.
    for (i = 0; i < NODE_VALUES; i++) {
      cluster->P[STUDENT_KNOWS]->probability[i] = cluster->P[STUDENT_KNOWS]->post_probabilitiy[i];
    }
    initial_tree(cluster);
}

void BN_update_backbone() {
    BN_display(cluster, 0);
}
