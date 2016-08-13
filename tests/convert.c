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

/* Test the functions in the module src/convert.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>

#include <gnx.h>

#include "routine.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* disable self-loops */
static void no_selfloop_already(void);
static void no_selfloop_empty(void);
static void no_selfloop_zero_edges(void);
static void no_selfloop_zero_selfloop(void);
static void no_selfloop_directed_unweighted(void);
static void no_selfloop_directed_weighted(void);
static void no_selfloop_undirected_unweighted(void);
static void no_selfloop_undirected_weighted(void);

/**************************************************************************
 * disable self-loops
 *************************************************************************/

static void
no_selfloop(void)
{
    no_selfloop_already();
    no_selfloop_empty();
    no_selfloop_zero_edges();
    no_selfloop_zero_selfloop();
    no_selfloop_directed_unweighted();
    no_selfloop_directed_weighted();
    no_selfloop_undirected_unweighted();
    no_selfloop_undirected_weighted();
}

/* Disable self-loops in a graph that already does not allow self-loops.
 */
static void
no_selfloop_already(void)
{
    GnxGraph *graph;
    const double weight = 3.14159;
    const unsigned int u = 0;
    const unsigned int v = 1;

    /* Directed, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    gnx_destroy(graph);

    /* Directed, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    gnx_destroy(graph);

    /* Undirected, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    gnx_destroy(graph);

    /* Undirected, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    gnx_destroy(graph);
}

/* Disable self-loops in an empty graph.
 */
static void
no_selfloop_empty(void)
{
    GnxGraph *graph;

    /* Directed, self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Directed, self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);
}

/* Disable self-loops in a graph that has zero edges.
 */
static void
no_selfloop_zero_edges(void)
{
    GnxGraph *graph;
    const unsigned int v = 0;

    /* Directed, self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(0 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);
}

/* Disable self-loops in a graph that has edges, but zero self-loops.
 */
static void
no_selfloop_zero_selfloop(void)
{
    GnxGraph *graph;
    const double weight = 3.14159;
    const unsigned int u = 0;
    const unsigned int v = 1;

    /* Directed, self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_allows_selfloop(graph));
    assert(gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));
    assert(!gnx_disable_selfloop(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);
}

/* Disable self-loops in a graph that is directed and unweighted.
 */
static void
no_selfloop_directed_unweighted(void)
{
    GnxGraph *graph;
    const unsigned int tail[5] = {0, 0, 0, 2, 3};
    const unsigned int head[5] = {0, 1, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int nnode = 4;
    const unsigned int nedge = 5;
    const unsigned int nloop = 2;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_allows_selfloop(graph));

    assert(gnx_disable_selfloop(graph));
    assert(nnode == graph->total_nodes);
    assert((nedge - nloop) == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));

    gnx_destroy(graph);
}

/* Disable self-loops in a graph that is directed and weighted.
 */
static void
no_selfloop_directed_weighted(void)
{
    GnxGraph *graph;
    const double weight[5]     = {0, 1, 2, 3, 4};
    const unsigned int tail[5] = {0, 0, 0, 2, 3};
    const unsigned int head[5] = {0, 1, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int nnode = 4;
    const unsigned int nedge = 5;
    const unsigned int nloop = 2;

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(graph, tail, head, weight, &size);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_allows_selfloop(graph));

    assert(gnx_disable_selfloop(graph));
    assert(nnode == graph->total_nodes);
    assert((nedge - nloop) == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));

    gnx_destroy(graph);
}

/* Disable self-loops in a graph that is undirected and unweighted.
 */
static void
no_selfloop_undirected_unweighted(void)
{
    GnxGraph *graph;
    const unsigned int tail[5] = {0, 0, 0, 2, 3};
    const unsigned int head[5] = {0, 1, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int nnode = 4;
    const unsigned int nedge = 5;
    const unsigned int nloop = 2;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_allows_selfloop(graph));

    assert(gnx_disable_selfloop(graph));
    assert(nnode == graph->total_nodes);
    assert((nedge - nloop) == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));

    gnx_destroy(graph);
}

/* Disable self-loops in a graph that is undirected and weighted.
 */
static void
no_selfloop_undirected_weighted(void)
{
    GnxGraph *graph;
    const double weight[5]     = {0, 1, 2, 3, 4};
    const unsigned int tail[5] = {0, 0, 0, 2, 3};
    const unsigned int head[5] = {0, 1, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int nnode = 4;
    const unsigned int nedge = 5;
    const unsigned int nloop = 2;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(graph, tail, head, weight, &size);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_allows_selfloop(graph));

    assert(gnx_disable_selfloop(graph));
    assert(nnode == graph->total_nodes);
    assert((nedge - nloop) == graph->total_edges);
    assert(!gnx_allows_selfloop(graph));

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

    g_test_add_func("/convert/disable-self-loop", no_selfloop);

    return g_test_run();
}
