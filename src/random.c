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

#include <glib.h>
#include <gmp.h>

#include "base.h"
#include "random.h"
#include "sanity.h"

/**
 * @file random.h
 * @brief Random nodes, edges, and graphs.
 */

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Randomly choose a node from a graph.
 *
 * @param graph A graph that has at least one node.
 * @return A node that has been chosen uniformly at random from the graph.
 */
unsigned int
gnx_random_node(const GnxGraph *graph)
{
    gmp_randstate_t state;
    mpz_t high, n;
    unsigned int v;

    gnx_i_check(graph);
    g_return_val_if_fail(graph->total_nodes, GNX_FAILURE);

    gmp_randinit_mt(state);
    mpz_init(n);
    mpz_init_set_ui(high, graph->capacity);

    do {
        /* Random integer in the range [0, high - 1]. */
        mpz_urandomm(n, state, high);
        v = (unsigned int)mpz_get_ui(n);
    } while (!gnx_has_node(graph, &v));

    gmp_randclear(state);
    mpz_clears(high, n, NULL);

    return v;
}
