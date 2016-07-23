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

/* Test the functions in the module src/base.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <gnx.h>

#include "constant.h"
#include "routine.h"

/**************************************************************************
 * prototypes of helper functions
 *************************************************************************/

static void is_empty_graph(const GnxGraph *graph);
static void test_properties(const GnxGraph *graph,
                            const GnxBool directed,
                            const GnxBool selfloop,
                            const GnxBool weighted);

/* add edge: unweighted edge */
static void add_edge_no_memory(void);
static void add_edge_no_selfloop(void);
static void add_edge_one_directed(void);
static void add_edge_one_undirected(void);
static void add_edge_random_directed(void);
static void add_edge_random_undirected(void);
static void add_edge_selfloop_directed_unweighted(void);
static void add_edge_selfloop_undirected_unweighted(void);

/* add edge: weighted edge */
static void add_edge_weighted_no_memory(void);
static void add_edge_weighted_no_selfloop(void);
static void add_edge_weighted_one_directed(void);
static void add_edge_weighted_one_undirected(void);
static void add_edge_weighted_random_directed(void);
static void add_edge_weighted_random_undirected(void);
static void add_edge_weighted_selfloop_directed(void);
static void add_edge_weighted_selfloop_undirected(void);

/* add node */
static void add_node_no_memory(void);
static void add_node_one_unweighted(void);
static void add_node_one_weighted(void);
static void add_node_random_unweighted(void);
static void add_node_random_weighted(void);
static void add_node_resize(void);

/* delete node */
static void delete_node_empty(void);
static void delete_node_one_directed_unweighted(void);
static void delete_node_one_directed_weighted(void);
static void delete_node_one_undirected_unweighted(void);
static void delete_node_one_undirected_weighted(void);
static void delete_node_random_directed_unweighted(void);
static void delete_node_random_directed_weighted(void);
static void delete_node_random_undirected_unweighted(void);
static void delete_node_random_undirected_weighted(void);
static void delete_node_selfloop_directed_unweighted(void);
static void delete_node_selfloop_directed_weighted(void);
static void delete_node_selfloop_undirected_unweighted(void);
static void delete_node_selfloop_undirected_weighted(void);

/* has edge */
static void has_edge_empty(void);
static void has_edge_one_node(void);
static void has_edge_two_nodes_directed_weighted(void);
static void has_edge_two_nodes_directed_unweighted(void);
static void has_edge_two_nodes_no_selfloop(void);
static void has_edge_two_nodes_undirected_weighted(void);
static void has_edge_two_nodes_undirected_unweighted(void);

/* has node */
static void has_node_empty(void);

/* new: create and destroy */
static void new_empty(void);
static void new_no_memory(void);
static void new_undirected_no_selfloop_unweighted(void);
static void new_undirected_no_selfloop_weighted(void);
static void new_undirected_selfloop_unweighted(void);
static void new_undirected_selfloop_weighted(void);
static void new_directed_no_selfloop_unweighted(void);
static void new_directed_no_selfloop_weighted(void);
static void new_directed_selfloop_unweighted(void);
static void new_directed_selfloop_weighted(void);

/**************************************************************************
 * helper functions
 *************************************************************************/

/* Test that a graph is empty.  A graph is empty if it has zero nodes and
 * zero edges.
 */
static void
is_empty_graph(const GnxGraph *graph)
{
    assert(0 == graph->total_nodes);
    assert(0 == graph->total_edges);
}

/* Test a graph for a specified list of properties.
 */
static void
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

/**************************************************************************
 * add edge: unweighted edge
 *************************************************************************/

static void
add_edge(void)
{
    add_edge_no_memory();
    add_edge_no_selfloop();
    add_edge_one_directed();
    add_edge_one_undirected();
    add_edge_random_directed();
    add_edge_random_undirected();
    add_edge_selfloop_directed_unweighted();
    add_edge_selfloop_undirected_unweighted();
}

/* Test the function gnx_add_edge() under low-memory scenarios.
 */
static void
add_edge_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    int alloc_size;
    unsigned int u, v;
    const int high = 35;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    random_edge(&low, &high, &u, &v);
    is_empty_graph(graph);

    /* Cannot allocate memory for the tail node. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_add_edge(graph, &u, &v));
    assert(ENOMEM == errno);
    is_empty_graph(graph);

    /* Cannot add a node to the set graph->node. */
    alloc_size = (2 * GNX_ALLOC_NODE_DIRECTED_UNWEIGHTED_SIZE) + 1;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_add_edge(graph, &u, &v));
    assert(ENOMEM == errno);
    is_empty_graph(graph);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
}

/* Insert a self-loop into a graph that does not allow self-loops.
 */
static void
add_edge_no_selfloop(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 65;
    const int low = 0;

    graph = gnx_new();
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);

    /* Insert a self-loop into an empty graph. */
    u = (unsigned int)g_random_int_range(low, high);
    assert(!gnx_has_node(graph, &u));
    assert(!gnx_add_edge(graph, &u, &u));
    is_empty_graph(graph);

    random_edge(&low, &high, &u, &v);
    assert(u != v);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    /* A node that is in the graph.  Use this node as a self-loop and try to
     * insert the self-loop into the graph.
     */
    assert(gnx_has_node(graph, &u));
    assert(!gnx_add_edge(graph, &u, &u));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    /* A node that is not in the graph.  Use this node as a self-loop and try
     * to insert the self-loop into the graph.
     */
    do {
        u = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &u));
    assert(!gnx_has_node(graph, &u));
    assert(!gnx_add_edge(graph, &u, &u));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert one edge into an unweighted digraph.
 */
static void
add_edge_one_directed(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 65;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(u != v);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert one edge into an unweighted graph that is undirected.
 */
static void
add_edge_one_undirected(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 65;
    const int low = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(u != v);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert a random number of edges into a digraph that is unweighted.
 */
static void
add_edge_random_directed(void)
{
    GnxGraph *graph;
    unsigned int i, nnode, u, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 43);

    nnode = 0;
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    /* Add a bunch of unique edges to the graph. */
    for (i = 0; i < size; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u))
            nnode++;
        if (!gnx_has_node(graph, &v))
            nnode++;

        assert(gnx_add_edge(graph, &u, &v));
    }
    assert(size == graph->total_edges);
    assert(nnode == graph->total_nodes);

    gnx_destroy(graph);
}

/* Insert a random number of edges into an undirected graph that is unweighted.
 */
static void
add_edge_random_undirected(void)
{
    GnxGraph *graph;
    unsigned int i, nnode, u, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 43);

    nnode = 0;
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    /* Add a bunch of unique edges to the graph. */
    for (i = 0; i < size; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u))
            nnode++;
        if (!gnx_has_node(graph, &v))
            nnode++;

        assert(gnx_add_edge(graph, &u, &v));
    }
    assert(size == graph->total_edges);
    assert(nnode == graph->total_nodes);

    gnx_destroy(graph);
}

/* Insert a self-loop into a graph that allows for self-loops.  The graph is
 * directed and unweighted.
 */
static void
add_edge_selfloop_directed_unweighted(void)
{
    GnxGraph *graph;
    unsigned int i, *node, size, u, v;
    const int high = 43;
    const int low = 0;

    node = (unsigned int *)malloc(sizeof(unsigned int) * 10);
    size = 0;

    /* Insert a self-loop into an empty graph. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    u = (unsigned int)g_random_int_range(low, high);
    assert(!gnx_has_node(graph, &u));
    assert(gnx_add_edge(graph, &u, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    node[size] = u;
    size++;

    /* Insert a random edge. */
    do {
        random_edge(&low, &high, &u, &v);
    } while (gnx_has_edge(graph, &u, &v));
    assert(u != v);
    if (!gnx_has_node(graph, &u)) {
        node[size] = u;
        size++;
    }
    if (!gnx_has_node(graph, &v)) {
        node[size] = v;
        size++;
    }
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_edges);

    /* Choose a node v that is in the graph.  If (v,v) is not a self-loop of
     * the graph, then add the self-loop to the graph.
     */
    do {
        i = (unsigned int)g_random_int_range(low, (int)size);
        v = node[i];
    } while (gnx_has_edge(graph, &v, &v));
    assert(gnx_has_node(graph, &v));
    assert(gnx_add_edge(graph, &v, &v));
    assert(3 == graph->total_edges);

    /* Choose a node v that is not in the graph.  Add the self-loop (v,v) to
     * the graph.
     */
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    assert(gnx_add_edge(graph, &v, &v));
    assert(4 == graph->total_edges);

    free(node);
    gnx_destroy(graph);
}

/* Insert a self-loop into a graph that allows for self-loops.  The graph is
 * undirected and unweighted.
 */
static void
add_edge_selfloop_undirected_unweighted(void)
{
    GnxGraph *graph;
    unsigned int i, *node, size, u, v;
    const int high = 43;
    const int low = 0;

    node = (unsigned int *)malloc(sizeof(unsigned int) * 10);
    size = 0;

    /* Insert a self-loop into an empty graph. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    u = (unsigned int)g_random_int_range(low, high);
    assert(!gnx_has_node(graph, &u));
    assert(gnx_add_edge(graph, &u, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    node[size] = u;
    size++;

    /* Insert a random edge. */
    do {
        random_edge(&low, &high, &u, &v);
    } while (gnx_has_edge(graph, &u, &v));
    assert(u != v);
    if (!gnx_has_node(graph, &u)) {
        node[size] = u;
        size++;
    }
    if (!gnx_has_node(graph, &v)) {
        node[size] = v;
        size++;
    }
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_edges);

    /* Choose a node v that is in the graph.  If (v,v) is not a self-loop of
     * the graph, then add the self-loop to the graph.
     */
    do {
        i = (unsigned int)g_random_int_range(low, (int)size);
        v = node[i];
    } while (gnx_has_edge(graph, &v, &v));
    assert(gnx_has_node(graph, &v));
    assert(gnx_add_edge(graph, &v, &v));
    assert(3 == graph->total_edges);

    /* Choose a node v that is not in the graph.  Add the self-loop (v,v) to
     * the graph.
     */
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    assert(gnx_add_edge(graph, &v, &v));
    assert(4 == graph->total_edges);

    free(node);
    gnx_destroy(graph);
}

/**************************************************************************
 * add edge: weighted edge
 *************************************************************************/

static void
add_edge_weighted(void)
{
    add_edge_weighted_no_memory();
    add_edge_weighted_no_selfloop();
    add_edge_weighted_one_directed();
    add_edge_weighted_one_undirected();
    add_edge_weighted_random_directed();
    add_edge_weighted_random_undirected();
    add_edge_weighted_selfloop_directed();
    add_edge_weighted_selfloop_undirected();
}

/* Test the function gnx_add_edgew() under low-memory scenarios.
 */
static void
add_edge_weighted_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    int alloc_size;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int high = 35;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    random_edge(&low, &high, &u, &v);
    is_empty_graph(graph);

    /* Cannot allocate memory for the tail node. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_add_edgew(graph, &u, &v, &weight));
    assert(ENOMEM == errno);
    is_empty_graph(graph);

    /* Cannot add a node to the set graph->node. */
    alloc_size = (2 * GNX_ALLOC_NODE_DIRECTED_WEIGHTED_SIZE) + 1;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_add_edgew(graph, &u, &v, &weight));
    assert(ENOMEM == errno);
    is_empty_graph(graph);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
}

/* Insert a self-loop into a weighted graph that does not allow self-loops.
 */
static void
add_edge_weighted_no_selfloop(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 65;
    const int low = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);

    /* Insert a self-loop into an empty graph. */
    u = (unsigned int)g_random_int_range(low, high);
    weight = (double)g_random_double();
    assert(!gnx_has_node(graph, &u));
    assert(!gnx_add_edgew(graph, &u, &u, &weight));
    is_empty_graph(graph);

    random_edge(&low, &high, &u, &v);
    assert(u != v);
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    /* A node that is in the graph.  Use this node as a self-loop and try to
     * insert the self-loop into the graph.
     */
    assert(gnx_has_node(graph, &u));
    weight = (double)g_random_double();
    assert(!gnx_add_edgew(graph, &u, &u, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    /* A node that is not in the graph.  Use this node as a self-loop and try
     * to insert the self-loop into the graph.
     */
    do {
        u = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &u));
    assert(!gnx_has_node(graph, &u));
    weight = (double)g_random_double();
    assert(!gnx_add_edgew(graph, &u, &u, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert one edge into a weighted digraph.
 */
static void
add_edge_weighted_one_directed(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int high = 65;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(u != v);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert one edge into a weighted graph that is undirected.
 */
static void
add_edge_weighted_one_undirected(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int high = 65;
    const int low = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(u != v);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);

    gnx_destroy(graph);
}

/* Insert a random number of edges into a weighted digraph.
 */
static void
add_edge_weighted_random_directed(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, nnode, u, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 43);

    nnode = 0;
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    /* Add a bunch of unique edges to the graph. */
    for (i = 0; i < size; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();

        if (!gnx_has_node(graph, &u))
            nnode++;
        if (!gnx_has_node(graph, &v))
            nnode++;

        assert(gnx_add_edgew(graph, &u, &v, &weight));
    }
    assert(size == graph->total_edges);
    assert(nnode == graph->total_nodes);

    gnx_destroy(graph);
}

/* Insert a random number of edges into an undirected graph that is weighted.
 */
static void
add_edge_weighted_random_undirected(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, nnode, u, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 43);

    nnode = 0;
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    /* Add a bunch of unique edges to the graph. */
    for (i = 0; i < size; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();

        if (!gnx_has_node(graph, &u))
            nnode++;
        if (!gnx_has_node(graph, &v))
            nnode++;

        assert(gnx_add_edgew(graph, &u, &v, &weight));
    }
    assert(size == graph->total_edges);
    assert(nnode == graph->total_nodes);

    gnx_destroy(graph);
}

/* Insert a self-loop into a weighted digraph that allows for self-loops.
 */
static void
add_edge_weighted_selfloop_directed(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, *node, size, u, v;
    const int high = 43;
    const int low = 0;

    node = (unsigned int *)malloc(sizeof(unsigned int) * 10);
    size = 0;

    /* Insert a self-loop into an empty graph. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    u = (unsigned int)g_random_int_range(low, high);
    weight = (double)g_random_double();
    assert(!gnx_has_node(graph, &u));
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    node[size] = u;
    size++;

    /* Insert a random edge. */
    do {
        random_edge(&low, &high, &u, &v);
    } while (gnx_has_edge(graph, &u, &v));
    assert(u != v);
    if (!gnx_has_node(graph, &u)) {
        node[size] = u;
        size++;
    }
    if (!gnx_has_node(graph, &v)) {
        node[size] = v;
        size++;
    }
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_edges);

    /* Choose a node v that is in the graph.  If (v,v) is not a self-loop of
     * the graph, then add the self-loop to the graph.
     */
    do {
        i = (unsigned int)g_random_int_range(low, (int)size);
        v = node[i];
    } while (gnx_has_edge(graph, &v, &v));
    assert(gnx_has_node(graph, &v));
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(3 == graph->total_edges);

    /* Choose a node v that is not in the graph.  Add the self-loop (v,v) to
     * the graph.
     */
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(4 == graph->total_edges);

    free(node);
    gnx_destroy(graph);
}

/* Insert a self-loop into a weighted graph that allows for self-loops.  The
 * graph is undirected.
 */
static void
add_edge_weighted_selfloop_undirected(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, *node, size, u, v;
    const int high = 43;
    const int low = 0;

    node = (unsigned int *)malloc(sizeof(unsigned int) * 10);
    size = 0;

    /* Insert a self-loop into an empty graph. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    u = (unsigned int)g_random_int_range(low, high);
    weight = (double)g_random_double();
    assert(!gnx_has_node(graph, &u));
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    node[size] = u;
    size++;

    /* Insert a random edge. */
    do {
        random_edge(&low, &high, &u, &v);
    } while (gnx_has_edge(graph, &u, &v));
    assert(u != v);
    if (!gnx_has_node(graph, &u)) {
        node[size] = u;
        size++;
    }
    if (!gnx_has_node(graph, &v)) {
        node[size] = v;
        size++;
    }
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_edges);

    /* Choose a node v that is in the graph.  If (v,v) is not a self-loop of
     * the graph, then add the self-loop to the graph.
     */
    do {
        i = (unsigned int)g_random_int_range(low, (int)size);
        v = node[i];
    } while (gnx_has_edge(graph, &v, &v));
    assert(gnx_has_node(graph, &v));
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(3 == graph->total_edges);

    /* Choose a node v that is not in the graph.  Add the self-loop (v,v) to
     * the graph.
     */
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (gnx_has_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    weight = (double)g_random_double();
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(4 == graph->total_edges);

    free(node);
    gnx_destroy(graph);
}

/**************************************************************************
 * add node
 *************************************************************************/

static void
add_node(void)
{
    add_node_no_memory();
    add_node_one_unweighted();
    add_node_one_weighted();
    add_node_random_unweighted();
    add_node_random_weighted();
    add_node_resize();
}

/* Test the function gnx_add_node() under low-memory scenarios.
 */
static void
add_node_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    const int alloc_size = 0;
    const int high = 65;
    const int low = 0;
    const unsigned int v = (unsigned int)g_random_int_range(low, high);

    graph = gnx_new();
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_add_node(graph, &v));
    assert(ENOMEM == errno);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
}

/* Add one node to an empty graph that is unweighted.
 */
static void
add_node_one_unweighted(void)
{
    GnxGraph *graph;
    const unsigned int size = 1;
    const unsigned int v = 0;

    graph = gnx_new();
    is_empty_graph(graph);
    assert(!gnx_is_weighted(graph));

    assert(gnx_add_node(graph, &v));
    assert(size == graph->total_nodes);
    assert(gnx_has_node(graph, &v));

    gnx_destroy(graph);
}

/* Add one node to an empty graph that is weighted.
 */
static void
add_node_one_weighted(void)
{
    GnxGraph *graph;
    const unsigned int size = 1;
    const unsigned int v = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(gnx_is_weighted(graph));

    assert(gnx_add_node(graph, &v));
    assert(size == graph->total_nodes);
    assert(gnx_has_node(graph, &v));

    gnx_destroy(graph);
}

/* Add to an unweighted graph a random number of nodes.
 */
static void
add_node_random_unweighted(void)
{
    GnxGraph *graph;
    unsigned int i, *node, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 33);

    node = (unsigned int *)malloc(sizeof(unsigned int) * size);
    graph = gnx_new();
    assert(!gnx_is_weighted(graph));

    /* Insert a bunch of nodes into the graph. */
    for (i = 0; i < size; i++) {
        do {
            v = (unsigned int)g_random_int_range(low, high);
        } while (gnx_has_node(graph, &v));

        node[i] = v;
        assert(gnx_add_node(graph, &(node[i])));
    }
    assert(size == graph->total_nodes);

    /* Check that our list of nodes are indeed nodes of the graph. */
    for (i = 0; i < size; i++)
        assert(gnx_has_node(graph, &(node[i])));
    assert(size == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Add to a weighted graph a random number of nodes.
 */
static void
add_node_random_weighted(void)
{
    GnxGraph *graph;
    unsigned int i, *node, v;
    const int high = 124;
    const int low = 0;
    const unsigned int size = (unsigned int)g_random_int_range(2, 33);

    node = (unsigned int *)malloc(sizeof(unsigned int) * size);
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_is_weighted(graph));

    /* Insert a bunch of nodes into the graph. */
    for (i = 0; i < size; i++) {
        do {
            v = (unsigned int)g_random_int_range(low, high);
        } while (gnx_has_node(graph, &v));

        node[i] = v;
        assert(gnx_add_node(graph, &(node[i])));
    }
    assert(size == graph->total_nodes);

    /* Check that our list of nodes are indeed nodes of the graph. */
    for (i = 0; i < size; i++)
        assert(gnx_has_node(graph, &(node[i])));
    assert(size == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Add enough nodes to trigger a resize of a graph.
 */
static void
add_node_resize(void)
{
    GnxGraph *graph;
    unsigned int i;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    graph = gnx_new();
    assert(size == graph->capacity);

    /* Insert as many nodes as possible without triggering a resize. */
    for (i = 0; i < size; i++)
        assert(gnx_add_node(graph, &i));
    assert(size == graph->total_nodes);
    assert(size == graph->capacity);

    /* Inserting an extra node will cause the graph to resize. */
    i = size;
    assert(!gnx_has_node(graph, &i));
    assert(gnx_add_node(graph, &i));
    assert((size + 1) == graph->total_nodes);
    assert((size << 1) == graph->capacity);

    gnx_destroy(graph);
}

/**************************************************************************
 * delete node
 *************************************************************************/

static void
delete_node(void)
{
    delete_node_empty();
    delete_node_one_directed_unweighted();
    delete_node_one_directed_weighted();
    delete_node_one_undirected_unweighted();
    delete_node_one_undirected_weighted();
    delete_node_random_directed_unweighted();
    delete_node_random_directed_weighted();
    delete_node_random_undirected_unweighted();
    delete_node_random_undirected_weighted();
    delete_node_selfloop_directed_unweighted();
    delete_node_selfloop_directed_weighted();
    delete_node_selfloop_undirected_unweighted();
    delete_node_selfloop_undirected_weighted();
}

/* Remove a node from an empty graph.
 */
static void
delete_node_empty(void)
{
    GnxGraph *graph;
    const unsigned int v = random_node_id();

    /* Directed and weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_delete_node(graph, &v));
    gnx_destroy(graph);

    /* Directed and unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_delete_node(graph, &v));
    gnx_destroy(graph);

    /* Undirected and weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_delete_node(graph, &v));
    gnx_destroy(graph);

    /* Undirected and unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_delete_node(graph, &v));
    gnx_destroy(graph);

    /* Allows self-loops. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_delete_node(graph, &v));
    gnx_destroy(graph);
}

/* Remove a node from an unweighted digraph that has exactly one node.
 */
static void
delete_node_one_directed_unweighted(void)
{
    GnxGraph *graph;
    const int high = 65;
    const int low = 0;
    const unsigned int v = (unsigned int)g_random_int_range(low, high);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &v));
    assert(gnx_delete_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/* Remove a node from a weighted digraph that has exactly one node.
 */
static void
delete_node_one_directed_weighted(void)
{
    GnxGraph *graph;
    const int high = 65;
    const int low = 0;
    const unsigned int v = (unsigned int)g_random_int_range(low, high);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &v));
    assert(gnx_delete_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/* Remove a node from a graph that is undirected, unweighted, and has one node.
 */
static void
delete_node_one_undirected_unweighted(void)
{
    GnxGraph *graph;
    const int high = 65;
    const int low = 0;
    const unsigned int v = (unsigned int)g_random_int_range(low, high);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &v));
    assert(gnx_delete_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/* Remove a node from a graph that is undirected, weighted, and has one node.
 */
static void
delete_node_one_undirected_weighted(void)
{
    GnxGraph *graph;
    const int high = 65;
    const int low = 0;
    const unsigned int v = (unsigned int)g_random_int_range(low, high);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &v));
    assert(gnx_delete_node(graph, &v));
    assert(!gnx_has_node(graph, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/* Add a random number of edges to an unweighted digraph.  We then remove a
 * random number of nodes from the graph.
 */
static void
delete_node_random_directed_unweighted(void)
{
    GnxGraph *graph;
    unsigned int i, j, nnode, *node, size, u, v;
    const int high = 20;
    const int low = 0;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 33);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    node = (unsigned int *)malloc(sizeof(unsigned int) * (2 * nedge));
    size = 0;

    /* Insert a random number of edges to the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u)) {
            node[size] = u;
            size++;
        }
        if (!gnx_has_node(graph, &v)) {
            node[size] = v;
            size++;
        }

        assert(gnx_add_edge(graph, &u, &v));
    }
    assert(nedge == graph->total_edges);
    assert(size == graph->total_nodes);

    /* Remove a random number of nodes from the graph. */
    nnode = (unsigned int)g_random_int_range(1, (int)size + 1);
    for (i = 0; i < nnode; i++) {
        do {
            j = (unsigned int)g_random_int_range(0, (int)size);
            v = node[j];
        } while (!gnx_has_node(graph, &v));

        assert(gnx_delete_node(graph, &v));
    }
    assert((size - nnode) == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Add a random number of edges to a weighted digraph.  We then remove a
 * random number of nodes from the graph.
 */
static void
delete_node_random_directed_weighted(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, j, nnode, *node, size, u, v;
    const int high = 20;
    const int low = 0;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 33);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    node = (unsigned int *)malloc(sizeof(unsigned int) * (2 * nedge));
    size = 0;

    /* Insert a random number of edges to the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u)) {
            node[size] = u;
            size++;
        }
        if (!gnx_has_node(graph, &v)) {
            node[size] = v;
            size++;
        }

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));
    }
    assert(nedge == graph->total_edges);
    assert(size == graph->total_nodes);

    /* Remove a random number of nodes from the graph. */
    nnode = (unsigned int)g_random_int_range(1, (int)size + 1);
    for (i = 0; i < nnode; i++) {
        do {
            j = (unsigned int)g_random_int_range(0, (int)size);
            v = node[j];
        } while (!gnx_has_node(graph, &v));

        assert(gnx_delete_node(graph, &v));
    }
    assert((size - nnode) == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Add a random number of edges to a graph that is undirected and unweighted.
 * We then remove a random number of nodes from the graph.
 */
static void
delete_node_random_undirected_unweighted(void)
{
    GnxGraph *graph;
    unsigned int i, j, nnode, *node, size, u, v;
    const int high = 20;
    const int low = 0;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 33);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    node = (unsigned int *)malloc(sizeof(unsigned int) * (2 * nedge));
    size = 0;

    /* Insert a random number of edges to the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u)) {
            node[size] = u;
            size++;
        }
        if (!gnx_has_node(graph, &v)) {
            node[size] = v;
            size++;
        }

        assert(gnx_add_edge(graph, &u, &v));
    }
    assert(nedge == graph->total_edges);
    assert(size == graph->total_nodes);

    /* Remove a random number of nodes from the graph. */
    nnode = (unsigned int)g_random_int_range(1, (int)size + 1);
    for (i = 0; i < nnode; i++) {
        do {
            j = (unsigned int)g_random_int_range(0, (int)size);
            v = node[j];
        } while (!gnx_has_node(graph, &v));

        assert(gnx_delete_node(graph, &v));
    }
    assert((size - nnode) == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Add a random number of edges to a graph that is undirected and weighted.
 * We then remove a random number of nodes from the graph.
 */
static void
delete_node_random_undirected_weighted(void)
{
    double weight;
    GnxGraph *graph;
    unsigned int i, j, nnode, *node, size, u, v;
    const int high = 20;
    const int low = 0;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 33);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    node = (unsigned int *)malloc(sizeof(unsigned int) * (2 * nedge));
    size = 0;

    /* Insert a random number of edges to the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        if (!gnx_has_node(graph, &u)) {
            node[size] = u;
            size++;
        }
        if (!gnx_has_node(graph, &v)) {
            node[size] = v;
            size++;
        }

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));
    }
    assert(nedge == graph->total_edges);
    assert(size == graph->total_nodes);

    /* Remove a random number of nodes from the graph. */
    nnode = (unsigned int)g_random_int_range(1, (int)size + 1);
    for (i = 0; i < nnode; i++) {
        do {
            j = (unsigned int)g_random_int_range(0, (int)size);
            v = node[j];
        } while (!gnx_has_node(graph, &v));

        assert(gnx_delete_node(graph, &v));
    }
    assert((size - nnode) == graph->total_nodes);

    free(node);
    gnx_destroy(graph);
}

/* Remove a self-loop node from an unweighted digraph.
 */
static void
delete_node_selfloop_directed_unweighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 32;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_edge(graph, &u, &u));
    assert(2 == graph->total_nodes);
    assert(2 == graph->total_edges);

    assert(gnx_delete_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Remove a self-loop node from a weighted digraph.
 */
static void
delete_node_selfloop_directed_weighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int high = 32;
    const int low = 0;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(2 == graph->total_nodes);
    assert(2 == graph->total_edges);

    assert(gnx_delete_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Remove a self-loop node from a graph that is undirected and unweighted.
 */
static void
delete_node_selfloop_undirected_unweighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 32;
    const int low = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_edge(graph, &u, &u));
    assert(2 == graph->total_nodes);
    assert(2 == graph->total_edges);

    assert(gnx_delete_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Remove a self-loop node from a graph that is undirected and weighted.
 */
static void
delete_node_selfloop_undirected_weighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int high = 32;
    const int low = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    random_edge(&low, &high, &u, &v);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(2 == graph->total_nodes);
    assert(2 == graph->total_edges);

    assert(gnx_delete_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/**************************************************************************
 * has edge
 *************************************************************************/

static void
has_edge(void)
{
    has_edge_empty();
    has_edge_one_node();
    has_edge_two_nodes_directed_weighted();
    has_edge_two_nodes_directed_unweighted();
    has_edge_two_nodes_no_selfloop();
    has_edge_two_nodes_undirected_weighted();
    has_edge_two_nodes_undirected_unweighted();
}

/* Query for an edge in an empty graph.
 */
static void
has_edge_empty(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 65;
    const int low = 0;

    /* An edge that is not a self-loop. */
    random_edge(&low, &high, &u, &v);

    graph = gnx_new();
    is_empty_graph(graph);
    assert(!gnx_has_node(graph, &u));
    assert(!gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly one node.
 */
static void
has_edge_one_node(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    /**********************************************************************
     * None of the nodes u,v is in the graph.
     *********************************************************************/

    graph = gnx_new();
    u = (unsigned int)g_random_int_range(low, high);
    assert(gnx_add_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    v = u;
    while (gnx_has_node(graph, &u) || gnx_has_node(graph, &v))
        random_edge(&low, &high, &u, &v);
    assert(u != v);

    assert(!gnx_has_node(graph, &u));
    assert(!gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);


    /**********************************************************************
     * Exactly one of the nodes u,v is in the graph.
     *********************************************************************/

    graph = gnx_new();
    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    if (g_random_boolean())
        assert(gnx_add_node(graph, &u));
    else
        assert(gnx_add_node(graph, &v));

    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert((gnx_has_node(graph, &u) && !gnx_has_node(graph, &v))
           || (!gnx_has_node(graph, &u) && gnx_has_node(graph, &v)));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly two nodes.  Here both of the
 * edge nodes are in the graph.  The graph is simple, directed, and weighted.
 */
static void
has_edge_two_nodes_directed_weighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly two nodes.  Here both of the
 * edge nodes are in the graph.  The graph is simple, directed, and unweighted.
 */
static void
has_edge_two_nodes_directed_unweighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly two nodes.  Here both of the
 * edge nodes are in the graph.  The graph does not allow self-loops.
 */
static void
has_edge_two_nodes_no_selfloop(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    graph = gnx_new();
    assert(!gnx_allows_selfloop(graph));
    assert(gnx_add_node(graph, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &u));
    assert(!gnx_has_edge(graph, &u, &u));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly two nodes.  Here both of the
 * edge nodes are in the graph.  The graph is simple, undirected, and weighted.
 */
static void
has_edge_two_nodes_undirected_weighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Query for an edge in a graph that has exactly two nodes.  Here both of the
 * edge nodes are in the graph.  The graph is simple, undirected, and
 * unweighted.
 */
static void
has_edge_two_nodes_undirected_unweighted(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const int high = 33;
    const int low = 0;

    u = (unsigned int)g_random_int_range(low, high);
    do {
        v = (unsigned int)g_random_int_range(low, high);
    } while (u == v);
    assert(u != v);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(gnx_has_node(graph, &u));
    assert(gnx_has_node(graph, &v));
    assert(!gnx_has_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/**************************************************************************
 * has node
 *************************************************************************/

static void
has_node(void)
{
    has_node_empty();
}

/* Query for a node in an empty graph.
 */
static void
has_node_empty(void)
{
    GnxGraph *graph;
    const unsigned int v = random_node_id();

    graph = gnx_new();
    is_empty_graph(graph);
    assert(!gnx_has_node(graph, &v));
    is_empty_graph(graph);

    gnx_destroy(graph);
}

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_empty();
    new_no_memory();
    new_undirected_no_selfloop_unweighted();
    new_undirected_no_selfloop_weighted();
    new_undirected_selfloop_unweighted();
    new_undirected_selfloop_weighted();
    new_directed_no_selfloop_unweighted();
    new_directed_no_selfloop_weighted();
    new_directed_selfloop_unweighted();
    new_directed_selfloop_weighted();
}

/* An empty graph.  A graph is empty if it has zero nodes and zero edges.
 */
static void
new_empty(void)
{
    GnxGraph *graph = gnx_new();
    is_empty_graph(graph);
    gnx_destroy(graph);
}

/* Test the function gnx_new_full() under low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;

    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_new());
    assert(ENOMEM == errno);

    gnx_alloc_reset_limit();
#endif
}

/* A graph that is undirected, does not allow self-loops, and is unweighted.
 */
static void
new_undirected_no_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, does not allow self-loops, and is weighted.
 */
static void
new_undirected_no_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, allows for self-loops, and is unweighted.
 */
static void
new_undirected_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, allows for self-loops, and is weighted.
 */
static void
new_undirected_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, does not allow self-loops, and is unweighted.
 */
static void
new_directed_no_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, does not allow self-loops, and is weighted.
 */
static void
new_directed_no_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, allows for self-loops, and is unweighted.
 */
static void
new_directed_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, allows for self-loops, and is weighted.
 */
static void
new_directed_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/base/add-edge", add_edge);
    g_test_add_func("/base/add-edge-weighted", add_edge_weighted);
    g_test_add_func("/base/add-node", add_node);
    g_test_add_func("/base/delete-node", delete_node);
    g_test_add_func("/base/has-edge", has_edge);
    g_test_add_func("/base/has-node", has_node);
    g_test_add_func("/base/new", new);

    return g_test_run();
}
