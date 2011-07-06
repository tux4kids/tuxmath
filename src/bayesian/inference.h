/* inference.h
  
   Interface for the inference algorithms.
   
   Copyright 2011.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

inference.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

#ifndef INFERENCE_H
#define INFERENCE_H
#include "bayesian_network.h"

/* One-time initialization call before using the   */
/* update_tree function. Initializes the Evidence- */
/* set, and posterior probability is set to prior  */
/* probability, intialize lambda, pi message       */
/* @Param Bayesian_Network reference               */
void initial_tree(Bayesian_Network);

/* Each time a (variable) Node is updated for a    */
/* 0/1, update_tree is called for finding out the  */
/* posterior probability of all the nodes.         */
/* @Param Bayesian_Network reference               */
/* @Param int node_index - The index of the insta  */
/* ntiated node.                                   */
/* @Param int node_value - The value(either 0 or 1)*/
void update_tree(Bayesian_Network, int node_index, int node_value);
#endif
