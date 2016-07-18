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

#include <gnx.h>

/**************************************************************************
 * prototypes of helper functions
 *************************************************************************/

static void is_empty_graph(const GnxGraph *graph);
static void test_properties(const GnxGraph *graph,
                            const GnxBool directed,
                            const GnxBool selfloop,
                            const GnxBool weighted);

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

    g_test_add_func("/base/has-node", has_node);
    g_test_add_func("/base/new", new);

    return g_test_run();
}
