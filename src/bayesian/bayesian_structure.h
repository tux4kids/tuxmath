#ifndef BAYESIAN_STRUCTURE_H
#define BAYESIAN_STRUCTURE_H

typedef enum {
    TRUE,
    FALSE
} node_state;

void BN_init_cluster(int topic_index);

void BN_update_cluster(node_state value);

void BN_update_backbone(void);
#endif
