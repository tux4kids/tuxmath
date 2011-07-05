/* bayes_test.c
  
   'bayes_test' - This is used for testing, debugging 
   and showing an example usage of 'bayesian_network.c'. 
   
   Copyright 2005, 2008, 2009, 2010.
   Authors:  Siddharth Kothari
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

bayes_test.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#include <stdio.h>
#include "bayesian_network.h"
#include "inference.h"

int main(void) {
  double d[] = {0.2};
  double d1[] = {0.25, 0.05};
  double d2[] = {0.003, 0.00005};
  double d3[] = {0.6, 0.02};
  Bayesian_Network BN = BN_init(4);
  BN_add_link(BN, 0,1);
  BN_add_link(BN, 0,2);
  BN_add_link(BN, 2,3);
  BN_nodeprobability(BN, 0, d);
  BN_nodeprobability(BN, 1, d1);
  BN_nodeprobability(BN, 2, d2);
  BN_nodeprobability(BN, 3, d3);

  initial_tree(BN);
  BN_display(BN);

  update_tree(BN, 1, 0);
  BN_display(BN);
}
