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
#include <stdlib.h>

#include "base.h"
#include "sanity.h"
#include "set.h"

/**
 * @file base.h
 * @brief Fundamental algorithms on graphs.
 *
 * To construct a graph, use either of the functions gnx_new() or
 * gnx_new_full().  To destroy a graph, use gnx_destroy().
 */

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Whether self-loops are allowed in a graph.
 *
 * @param graph The graph to query.
 * @return Nonzero if the graph allows for self-loops; zero otherwise.
 */
int
gnx_allows_selfloop(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->selfloop & GNX_SELFLOOP;
}

/**
 * @brief Destroys a graph.
 *
 * @param graph The graph to destroy.
 */
void
gnx_destroy(GnxGraph *graph)
{
    if (!graph)
        return;
    gnx_destroy_set(graph->node);
    if (graph->graph) {
        free(graph->graph);
        graph->graph = NULL;
    }
    free(graph);
    graph = NULL;
}

/**
 * @brief Whether a graph contains a node.
 *
 * @param graph The graph to query.
 * @param v A node to query.
 * @return Nonzero if the node is in the graph; zero otherwise.  We also return
 *         zero if the graph is empty.
 */
int
gnx_has_node(const GnxGraph *graph,
             const unsigned int *v)
{
    GnxDict *adj_weighted;
    GnxSet *adj_unweighted;

    gnx_i_check(graph);
    g_return_val_if_fail(v, GNX_FAILURE);

    if (!graph->total_nodes)
        return GNX_FAILURE;

    if (GNX_WEIGHTED & graph->weighted) {
        adj_weighted = (GnxDict *)(graph->graph[*v]);
        if (!adj_weighted)
            return GNX_FAILURE;

        return GNX_SUCCESS;
    }

    adj_unweighted = (GnxSet *)(graph->graph[*v]);
    if (!adj_unweighted)
        return GNX_FAILURE;

    return GNX_FAILURE;
}

/**
 * @brief Whether a graph is directed.
 *
 * @param graph The graph to test for directedness.
 * @return Nonzero if the graph is directed; zero otherwise.
 */
int
gnx_is_directed(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->directed & GNX_DIRECTED;
}

/**
 * @brief Whether a graph is weighted.
 *
 * @param graph The graph to test for weightedness.
 * @return Nonzero if the graph is weighted; zero otherwise.
 */
int
gnx_is_weighted(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->weighted & GNX_WEIGHTED;
}

/**
 * @brief Initializes a new graph with default properties.
 *
 * This is a convenience function for when you require a graph that is
 * undirected, unweighted, and without self-loops.  The graph should be
 * destroyed with gnx_destroy().  If you want to set the properties of a
 * new graph, use the function gnx_new_full() instead.
 *
 * @sa gnx_new_full() to set the properties of a new graph.
 *
 * @return See the return value of gnx_new_full().
 */
GnxGraph*
gnx_new(void)
{
    return gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
}

/**
 * @brief Initializes a new graph with some basic properties.
 *
 * A graph should be destroyed with gnx_destroy().  The function gnx_new_full()
 * allows you to set some basic properties of a new graph.  If you want a graph
 * that is undirected, unweighted, and without self-loops you can instead use
 * the convenience function gnx_new().
 *
 * @sa gnx_new() A convenience function to initialize a graph that is
 *     undirected, unweighted, and without self-loops.
 *
 * @param directed Whether to create a directed graph.  The possible values
 *        are: #GNX_UNDIRECTED or #GNX_DIRECTED.
 * @param selfloop Whether to allow self-loops in a graph.  The possible values
 *        are: #GNX_NO_SELFLOOP or #GNX_SELFLOOP.
 * @param weighted Whether to create a weighted graph.  The possible values
 *        are: #GNX_UNWEIGHTED or #GNX_WEIGHTED.
 * @return An initialized graph.  If we cannot allocate memory for a new graph,
 *         then we set @c errno to @c ENOMEM and return @c NULL.
 */
GnxGraph*
gnx_new_full(const GnxBool directed,
             const GnxBool selfloop,
             const GnxBool weighted)
{
    GnxGraph *graph;
    const unsigned int reserved_nodes = GNX_DEFAULT_ALLOC_SIZE;

    errno = 0;
    gnx_i_check_properties(directed, selfloop, weighted);

    graph = (GnxGraph *)malloc(sizeof(GnxGraph));
    if (!graph)
        goto cleanup;

    graph->node = NULL;
    graph->graph = NULL;

    /* A collection of nodes of the graph.  This collection does not
     * necessarily contain all the nodes of the graph.  Its purpose is to help
     * us determine whether to allocate memory for a node.
     */
    graph->node = gnx_init_set_full(GNX_FREE_ELEMENTS);
    if (!graph->node)
        goto cleanup;

    /* By default, we allocate enough memory for a graph with a specified
     * number of nodes.  As more nodes are added to the graph, we might need
     * to resize this array.
     */
    graph->graph = (gnxptr *)calloc(reserved_nodes, sizeof(gnxptr));
    if (!graph->graph)
        goto cleanup;

    graph->directed = directed;
    graph->selfloop = selfloop;
    graph->weighted = weighted;
    graph->capacity = reserved_nodes;
    graph->total_edges = 0;
    graph->total_nodes = 0;

    return graph;

cleanup:
    errno = ENOMEM;
    gnx_destroy(graph);
    return NULL;
}
