#ifndef BAYESIAN_STRUCTURE_H
#define BAYESIAN_STRUCTURE_H

typedef enum {
    TRUE,
    FALSE
} node_state;

void BS_init_beliefnet();

void BS_set_topic(int topic_index);

void BS_update_cluster(node_state value);
#endif
