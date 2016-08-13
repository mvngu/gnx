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

#include <assert.h>

#include <glib.h>

#include "base.h"
#include "convert.h"
#include "sanity.h"

/**
 * @file convert.h
 * @brief Convert a graph from one form to another.
 */

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Removes all self-loops in a graph.
 *
 * Convert a graph that allows self-loops to a graph that does not allow
 * self-loops.  All self-loops are removed.  If a node has a self-loop and is
 * not adjacent to any other node, then the node becomes isolated.
 *
 * @param graph Disable self-loops in this graph.  We will modify this graph.
 * @return Nonzero if the conversion was successful; zero otherwise.  We also
 *         return zero if the graph currently does not allow self-loops.
 */
int
gnx_disable_selfloop(GnxGraph *graph)
{
    GnxNodeIter iter;
    unsigned int v;

    gnx_i_check(graph);
    if (!graph->selfloop)
        return GNX_FAILURE;
    if ((!graph->total_nodes) || (!graph->total_edges)) {
        graph->selfloop = FALSE;
        return GNX_SUCCESS;
    }

    g_assert(graph->selfloop);
    g_assert(graph->total_edges);

    gnx_node_iter_init(&iter, graph);
    while (gnx_node_iter_next(&iter, &v)) {
        if (gnx_has_edge(graph, &v, &v))
            assert(gnx_delete_edge(graph, &v, &v));
    }
    graph->selfloop = FALSE;

    return GNX_SUCCESS;
}
