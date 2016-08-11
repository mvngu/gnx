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
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "base.h"
#include "queue.h"
#include "set.h"
#include "visit.h"

/**
 * @file visit.h
 * @brief Traversing a graph.
 */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_bfs(GnxGraph *graph,
                     GnxGraph *g,
                     GnxSet *seen,
                     GnxQueue *queue);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Traverses a graph via the strategy of breadth-first search.
 *
 * @param graph Traverse this graph.
 * @param g The search tree will be stored here.
 * @param seen A set of nodes that we have visited.
 * @param queue A queue of nodes to visit.
 * @return Nonzero if the traversal was successful; zero otherwise.
 */
static int
gnx_i_bfs(GnxGraph *graph,
          GnxGraph *g,
          GnxSet *seen,
          GnxQueue *queue)
{
    GnxNeighborIter iter;
    unsigned int *node, *u, v;

    errno = 0;

    while (queue->size) {
        u = gnx_queue_pop(queue);
        g_assert(u);

        /* Iterate over each (out-)neighbor of u.  Ignore the edge weights. */
        gnx_neighbor_iter_init(&iter, graph, u);
        while (gnx_neighbor_iter_next(&iter, &v, NULL)) {
            if (gnx_set_has(seen, &v))
                continue;

            node = (unsigned int *)malloc(sizeof(unsigned int));
            if (!node)
                goto cleanup;
            *node = v;

            if (!gnx_set_add(seen, node)) {
                free(node);
                node = NULL;
                goto cleanup;
            }
            if (!gnx_queue_append(queue, node))
                goto cleanup;
            if (!gnx_add_edge(g, u, node))
                goto cleanup;
        }
    }

    g_assert(g->total_nodes == seen->size);
    g_assert(g->total_nodes >= 2);
    g_assert(g->total_edges);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    return GNX_FAILURE;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Traverses a graph via the strategy of breadth-first search.
 *
 * If the given graph is directed, then we traverse the graph via out-neighbors
 * of nodes.
 *
 * @param graph Traverse this graph.
 * @param s Start the traversal from this node.  The node is assumed to be in
 *        the graph.
 * @return A breadth-first search tree that is rooted at @f$s@f$.  To destroy
 *         the tree, use the function gnx_destroy().  We return @c NULL if
 *         @a s does not have neighbors.  If we are unable to allocate memory,
 *         then @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxGraph*
gnx_breadth_first_search(GnxGraph *graph,
                         const unsigned int *s)
{
    GnxGraph *g = NULL;
    GnxQueue *queue = NULL;
    GnxSet *seen = NULL;
    int success;
    unsigned int directed;
    unsigned int *node = NULL;

    errno = 0;
    g_return_val_if_fail(gnx_has_node(graph, s), NULL);
    if (graph->directed) {
        if (!gnx_outdegree(graph, s))
            return NULL;
    } else {
        if (!gnx_degree(graph, s))
            return NULL;
    }

    /* Initialize the BFS tree. */
    directed = (graph->directed) ? GNX_DIRECTED : GNX_UNDIRECTED;
    g = gnx_new_full(directed, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    if (!g)
        goto cleanup;

    /* This will store the set of nodes that we have visited. */
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);
    if (!seen)
        goto cleanup;

    /* A queue of nodes to visit. */
    queue = gnx_init_queue();
    if (!queue)
        goto cleanup;

    /* Append the starting node to the queue. */
    node = (unsigned int *)malloc(sizeof(unsigned int));
    if (!node)
        goto cleanup;
    *node = *s;
    if (!gnx_queue_append(queue, node))
        goto cleanup;

    /* Start the traversal. */
    if (!gnx_set_add(seen, node))
        goto cleanup;

    success = gnx_i_bfs(graph, g, seen, queue);

    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);
    if (success)
        return g;

    gnx_destroy(g);
    return NULL;

cleanup:
    errno = ENOMEM;
    if (node) {
        free(node);
        node = NULL;
    }
    gnx_destroy(g);
    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);
    return NULL;
}
