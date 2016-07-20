/* gnx -- algorithms for graphs and networks
 * Copyright (C) 2016 Minh Van Nguyen <mvngu.name AT gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GNX_ROUTINE_H
#define GNX_ROUTINE_H

#include <assert.h>

#include <glib.h>
#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

void random_edge(unsigned int *u,
                 unsigned int *v);
unsigned int random_node_id(void);

/**************************************************************************
 * helper functions
 *************************************************************************/

/**
 * @brief A random edge that is not a self-loop.
 *
 * This function generates a random node ID for each end point of the edge.
 *
 * @param u This will hold one end point of an edge.
 * @param v This will hold the other end point of the edge.
 */
void
random_edge(unsigned int *u,
            unsigned int *v)
{
    assert(u);
    assert(v);

    *u = (unsigned int)g_random_int_range(0, (int)GNX_MAXIMUM_NODE_ID);
    do {
        *v = (unsigned int)g_random_int_range(0, (int)GNX_MAXIMUM_NODE_ID);
    } while (*u == *v);
}

/**
 * @brief Generates a random node ID.
 *
 * @return A random node ID.
 */
unsigned int
random_node_id(void)
{
    return (unsigned int)g_random_int_range(0, (int)GNX_MAXIMUM_NODE_ID);
}

#endif  /* GNX_ROUTINE_H */
