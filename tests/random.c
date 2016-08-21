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

/* Test the functions in the module src/random.c. */

#undef G_DISABLE_ASSERT

#include <gnx.h>

#include "routine.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* node randomly chosen */
static void node_one(void);
static void node_directed_selfloop_unweighted(void);
static void node_directed_selfloop_weighted(void);
static void node_directed_noselfloop_unweighted(void);
static void node_directed_noselfloop_weighted(void);
static void node_undirected_selfloop_unweighted(void);
static void node_undirected_selfloop_weighted(void);
static void node_undirected_noselfloop_unweighted(void);
static void node_undirected_noselfloop_weighted(void);

/**************************************************************************
 * node randomly chosen
 *************************************************************************/

static void
node(void)
{
    node_one();
    node_directed_selfloop_unweighted();
    node_directed_selfloop_weighted();
    node_directed_noselfloop_unweighted();
    node_directed_noselfloop_weighted();
    node_undirected_selfloop_unweighted();
    node_undirected_selfloop_weighted();
    node_undirected_noselfloop_unweighted();
    node_undirected_noselfloop_weighted();
}

/* Randomly choose a node from a graph that has exactly one node.
 */
static void
node_one(void)
{
    GnxGraph *graph;
    unsigned int u;
    const double weight = (double)g_random_double();
    const int low = 0;
    const int high = 42;
    const unsigned int v = random_node_id(&low, &high);

    /* Directed, self-loop, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &v, &v));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, self-loop, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loop, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loop, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &v, &v));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    u = gnx_random_node(graph);
    assert(v == u);
    assert(gnx_has_node(graph, &u));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);
}

/* Choose a node from a graph that is directed, allows self-loops, unweighted.
 */
static void
node_directed_selfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 8);

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        assert(gnx_add_edge(graph, &u, &v));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }

    /* Insert a bunch of self-loops into the graph. */
    for (i = 0; i < nloop; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_edge(graph, &v, &v));

        assert(gnx_add_edge(graph, &v, &v));

        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is directed, allows self-loops, weighted.
 */
static void
node_directed_selfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 8);

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }

    /* Insert a bunch of self-loops into the graph. */
    for (i = 0; i < nloop; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_edge(graph, &v, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &v, &v, &weight));

        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is directed, no self-loops, unweighted.
 */
static void
node_directed_noselfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        assert(gnx_add_edge(graph, &u, &v));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is directed, no self-loops, weighted.
 */
static void
node_directed_noselfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is undirected, allows self-loops, unweighted.
 */
static void
node_undirected_selfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 8);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        assert(gnx_add_edge(graph, &u, &v));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }

    /* Insert a bunch of self-loops into the graph. */
    for (i = 0; i < nloop; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_edge(graph, &v, &v));

        assert(gnx_add_edge(graph, &v, &v));

        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is undirected, allows self-loops, weighted.
 */
static void
node_undirected_selfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 8);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }

    /* Insert a bunch of self-loops into the graph. */
    for (i = 0; i < nloop; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_edge(graph, &v, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &v, &v, &weight));

        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is undirected, no self-loops, unweighted.
 */
static void
node_undirected_noselfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        assert(gnx_add_edge(graph, &u, &v));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/* Choose a node from a graph that is undirected, no self-loops, weighted.
 */
static void
node_undirected_noselfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges into the graph. */
    for (i = 0; i < nedge; i++) {
        do {
            random_edge(&low, &high, &u, &v);
        } while (gnx_has_edge(graph, &u, &v));

        weight = (double)g_random_double();
        assert(gnx_add_edgew(graph, &u, &v, &weight));

        if (!gnx_set_has(seen, &u)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = u;
            assert(gnx_set_add(seen, node));
        }
        if (!gnx_set_has(seen, &v)) {
            node = (unsigned int *)malloc(sizeof(unsigned int));
            *node = v;
            assert(gnx_set_add(seen, node));
        }
    }
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    v = gnx_random_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/random/node", node);

    return g_test_run();
}
