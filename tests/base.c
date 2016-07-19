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

/**************************************************************************
 * prototypes of helper functions
 *************************************************************************/

static void is_empty_graph(const GnxGraph *graph);
static void test_properties(const GnxGraph *graph,
                            const GnxBool directed,
                            const GnxBool selfloop,
                            const GnxBool weighted);

/* add node */
static void add_node_no_memory(void);
static void add_node_one_unweighted(void);
static void add_node_one_weighted(void);
static void add_node_random_unweighted(void);
static void add_node_random_weighted(void);
static void add_node_resize(void);

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

/* Test a graph for a specified list of properties. */
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
 * has node
 *************************************************************************/

static void
has_node(void)
{
    has_node_empty();
}

/* Query for a node in an empty graph. */
static void
has_node_empty(void)
{
    GnxGraph *graph;
    const unsigned int v = gnx_random_node_id();

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

/* An empty graph.  A graph is empty if it has zero nodes and zero edges. */
static void
new_empty(void)
{
    GnxGraph *graph = gnx_new();
    is_empty_graph(graph);
    gnx_destroy(graph);
}

/* Test the function gnx_new_full() under low-memory scenarios. */
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

/* A graph that is undirected, does not allow self-loops, and is unweighted. */
static void
new_undirected_no_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, does not allow self-loops, and is weighted. */
static void
new_undirected_no_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, allows for self-loops, and is unweighted. */
static void
new_undirected_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is undirected, allows for self-loops, and is weighted. */
static void
new_undirected_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, does not allow self-loops, and is unweighted. */
static void
new_directed_no_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, does not allow self-loops, and is weighted. */
static void
new_directed_no_selfloop_weighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, allows for self-loops, and is unweighted. */
static void
new_directed_selfloop_unweighted(void)
{
    GnxGraph *graph;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    test_properties(graph, GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);

    gnx_destroy(graph);
}

/* A graph that is directed, allows for self-loops, and is weighted. */
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

    g_test_add_func("/base/add-node", add_node);
    g_test_add_func("/base/has-node", has_node);
    g_test_add_func("/base/new", new);

    return g_test_run();
}
