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

int main(void) {
  double d[] = {0.0};
  double d1[] = {0.1, 0.9, 0.3, 0.7};
  BN_init(3);
  BN_add_link(0,2);
  BN_add_link(1,2);
  BN_nodeprobability(0, d);
  BN_nodeprobability(1, d);
  BN_nodeprobability(2, d1);
  BN_display();
}
