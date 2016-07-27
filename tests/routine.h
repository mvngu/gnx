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

void add_edges(GnxGraph *graph,
               const unsigned int tail[],
               const unsigned int head[],
               const unsigned int *size);
void add_edges_weighted(GnxGraph *graph,
                        const unsigned int tail[],
                        const unsigned int head[],
                        const double weight[],
                        const unsigned int *size);
void is_empty_graph(const GnxGraph *graph);
void random_edge(const int *low,
                 const int *high,
                 unsigned int *u,
                 unsigned int *v);
unsigned int random_node_id(void);
void test_properties(const GnxGraph *graph,
                     const GnxBool directed,
                     const GnxBool selfloop,
                     const GnxBool weighted);

/**************************************************************************
 * helper functions
 *************************************************************************/

/**
 * @brief Insert a bunch of edges into an unweighted graph.
 *
 * @param graph Add edges to this unweighted graph.
 * @param tail An array of tail nodes.
 * @param head An array of corresponding head nodes. An edge is
 *        (tail[i], head[i]).
 * @param size How many edges to insert.
 */
void
add_edges(GnxGraph *graph,
          const unsigned int tail[],
          const unsigned int head[],
          const unsigned int *size)
{
    unsigned int i;

    assert(size);
    assert(*size > 0);

    for (i = 0; i < *size; i++)
        assert(gnx_add_edge(graph, &(tail[i]), &(head[i])));
}

/**
 * @brief Insert a bunch of edges into an unweighted graph.
 *
 * @param graph Add edges to this unweighted graph.
 * @param tail An array of tail nodes.
 * @param head An array of corresponding head nodes. An edge is
 *        (tail[i], head[i]).
 * @param size How many edges to insert.
 */
void
add_edges_weighted(GnxGraph *graph,
                   const unsigned int tail[],
                   const unsigned int head[],
                   const double weight[],
                   const unsigned int *size)
{
    unsigned int i;

    assert(size);
    assert(*size > 0);

    for (i = 0; i < *size; i++)
        assert(gnx_add_edgew(graph, &(tail[i]), &(head[i]), &(weight[i])));
}

/**
 * @brief Test that a graph is empty.
 *
 * A graph is empty if it has zero nodes and zero edges.
 *
 * @param graph Test this graph.
 */
void
is_empty_graph(const GnxGraph *graph)
{
    assert(0 == graph->total_nodes);
    assert(0 == graph->total_edges);
}

/**
 * @brief A random edge that is not a self-loop.
 *
 * This function generates a random node ID for each end point of the edge.
 *
 * @param low The minimum node ID.
 * @param high Bound on the maximum node ID.  Note that the maximum node ID
 *        that can be generated is high - 1.
 * @param u This will hold one end point of an edge.
 * @param v This will hold the other end point of the edge.
 */
void
random_edge(const int *low,
            const int *high,
            unsigned int *u,
            unsigned int *v)
{
    assert(low);
    assert(*low >= 0);
    assert(high);
    assert(*high > *low);
    assert(u);
    assert(v);

    *u = (unsigned int)g_random_int_range(*low, *high);
    do {
        *v = (unsigned int)g_random_int_range(*low, *high);
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

/**
 * @brief Test a graph for a specified list of properties.
 *
 * @param graph Test this graph.
 * @param directed Whether the graph is directed or undirected.
 * @param selfloop Whether the graph allows for self-loops.
 * @param weighted Whether the graph is weighted or unweighted.
 */
void
test_properties(const GnxGraph *graph,
                const GnxBool directed,
                const GnxBool selfloop,
                const GnxBool weighted)
{
    assert(graph);
    assert((directed == GNX_DIRECTED) || (directed == GNX_UNDIRECTED));
    assert((selfloop == GNX_SELFLOOP) || (selfloop == GNX_NO_SELFLOOP));
    assert((weighted == GNX_WEIGHTED) || (weighted == GNX_UNWEIGHTED));

    if (directed == GNX_DIRECTED)
        assert(gnx_is_directed(graph));
    else
        assert(!gnx_is_directed(graph));

    if (selfloop == GNX_SELFLOOP)
        assert(gnx_allows_selfloop(graph));
    else
        assert(!gnx_allows_selfloop(graph));

    if (weighted == GNX_WEIGHTED)
        assert(gnx_is_weighted(graph));
    else
        assert(!gnx_is_weighted(graph));
}

#endif  /* GNX_ROUTINE_H */
