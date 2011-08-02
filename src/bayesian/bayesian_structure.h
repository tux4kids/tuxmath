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

typedef enum {
    TRUE,
    FALSE
} node_state;

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
#endif
