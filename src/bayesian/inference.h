#ifndef INFERENCE_H
#define INFERENCE_H
#include "bayesian_network.h"

void initial_tree(Bayesian_Network);

void update_tree(Bayesian_Network, int, int);
#endif
