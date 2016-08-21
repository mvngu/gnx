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

#include <errno.h>

#include <glib.h>

#include "base.h"
#include "query.h"
#include "queue.h"
#include "sanity.h"
#include "set.h"
#include "util.h"

/**
 * @file query.h
 * @brief Tests for particular properties of graphs.
 */

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Determines whether an undirected graph is connected.
 *
 * An undirected graph @f$G@f$ is connected if there is a path between each
 * pair of distinct nodes.  If we are given a node @f$u@f$ in @f$G@f$,
 * then we should be able to reach another node @f$v@f$ in the same graph.
 * Note that if @f$G@f$ has one node then @f$G@f$ is connected.  If @f$G@f$
 * has zero nodes, then @f$G@f$ is not connected.
 *
 * @param graph Determine whether this undirected graph is connected.
 * @return Nonzero if the graph is connected; zero otherwise.  If we are unable
 *         to allocate memory, then @c errno is set to @c ENOMEM and we return
 *         zero.
 */
int
gnx_is_connected(GnxGraph *graph)
{
    GnxNeighborIter iter;
    GnxQueue *queue = NULL;
    GnxSet *seen = NULL;
    gnxptr vptr;
    int connected;
    unsigned int start, *u, *v;

    errno = 0;
    gnx_i_check(graph);
    g_return_val_if_fail(!graph->directed, GNX_FAILURE);

    /* A graph with zero nodes is not connected. */
    if (!graph->total_nodes)
        return GNX_FAILURE;

    /* A graph with one node is trivially connected. */
    if (1 == graph->total_nodes)
        return GNX_SUCCESS;

    start = gnx_any_node(graph);

    /* This will store the set of nodes that we have visited. */
    seen = gnx_init_set();
    if (!seen)
        goto cleanup;
    if (!gnx_set_add(seen, &start))
        goto cleanup;

    /* A queue of nodes to visit. */
    queue = gnx_init_queue();
    if (!queue)
        goto cleanup;
    if (!gnx_queue_append(queue, &start))
        goto cleanup;

    /* Use breadth-first search to determine connectivity. */
    while (queue->size) {
        u = gnx_queue_pop(queue);
        g_assert(u);

        /* Iterate over each (out-)neighbor of u.  Ignore the edge weights. */
        gnx_neighbor_iter_init(&iter, graph, u);
        while (gnx_neighbor_iter_next(&iter, &vptr, NULL)) {
            v = (unsigned int *)vptr;
            g_assert(v);

            /* This should take care of nodes that we have seen.  Furthermore,
             * it should take care of the case where u has a self-loop.
             */
            if (gnx_set_has(seen, v))
                continue;

            g_assert(*u != *v);
            if (!gnx_set_add(seen, v))
                goto cleanup;
            if (!gnx_queue_append(queue, v))
                goto cleanup;
        }
    }

    connected = GNX_FAILURE;
    if (seen->size == graph->total_nodes)
        connected = GNX_SUCCESS;

    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);

    return connected;

cleanup:
    errno = ENOMEM;
    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);
    return GNX_FAILURE;
}

/**
 * @brief Determines whether an undirected graph is a tree.
 *
 * Let @f$T = (V, E)@f$ be an undirected graph with @f$n = |V|@f$ nodes.  If
 * @f$T@f$ is connected and has @f$n - 1@f$ edges, then @f$T@f$ is a tree.
 * Thus by definition, a graph with only one node is a tree, whereas a graph
 * with zero nodes is not a tree.
 *
 * @param graph We want to check whether this undirected graph is a tree.
 * @return Nonzero if the graph is a tree; zero otherwise.
 */
int
gnx_is_tree(GnxGraph *graph)
{
    unsigned int nedge, nnode;

    gnx_i_check(graph);
    g_return_val_if_fail(!graph->directed, GNX_FAILURE);

    nnode = graph->total_nodes;
    nedge = graph->total_edges;

    if (!nnode)
        return GNX_FAILURE;
    if (1 == nnode)
        return GNX_SUCCESS;

    g_assert(nnode >= 2);

    if (!nedge)
        return GNX_FAILURE;

    if (nedge != (nnode - 1))
        return GNX_FAILURE;

    g_assert(nedge == (nnode - 1));

    return gnx_is_connected(graph);
}
