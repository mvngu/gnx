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

/* Test the functions in the module src/util.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>

#include <glib.h>
#include <gnx.h>

#include "routine.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* any node of a graph */
static void any_node_one(void);
static void any_node_directed_selfloop_unweighted(void);
static void any_node_directed_selfloop_weighted(void);
static void any_node_directed_noselfloop_unweighted(void);
static void any_node_directed_noselfloop_weighted(void);
static void any_node_undirected_selfloop_unweighted(void);
static void any_node_undirected_selfloop_weighted(void);
static void any_node_undirected_noselfloop_unweighted(void);
static void any_node_undirected_noselfloop_weighted(void);

/* compare floating-point numbers */
static void compare_double_eq(void);
static void compare_double_le(void);

/* compare graphs */
static void compare_graph_directed(void);
static void compare_graph_edges(void);
static void compare_graph_empty(void);
static void compare_graph_nodes(void);
static void compare_graph_weighted(void);

/**************************************************************************
 * any node of a graph
 *************************************************************************/

static void
any_node(void)
{
    any_node_one();
    any_node_directed_selfloop_unweighted();
    any_node_directed_selfloop_weighted();
    any_node_directed_noselfloop_unweighted();
    any_node_directed_noselfloop_weighted();
    any_node_undirected_selfloop_unweighted();
    any_node_undirected_selfloop_weighted();
    any_node_undirected_noselfloop_unweighted();
    any_node_undirected_noselfloop_weighted();
}

/* Choose a node from a graph that has exactly one node.
 */
static void
any_node_one(void)
{
    GnxGraph *graph;
    unsigned int u, v;
    const double weight = (double)g_random_double();
    const int low = 0;
    const int high = 128;

    /* Directed, self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_edge(graph, &u, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_has_edge(graph, &u, &u));
    gnx_destroy(graph);

    /* Directed, self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_has_edge(graph, &u, &u));
    gnx_destroy(graph);

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_node(graph, &u));
    assert(1 == graph->total_nodes);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(gnx_has_node(graph, &u));
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_node(graph, &u));
    assert(1 == graph->total_nodes);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(gnx_has_node(graph, &u));
    gnx_destroy(graph);

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_edge(graph, &u, &u));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_has_edge(graph, &u, &u));
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_has_edge(graph, &u, &u));
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_node(graph, &u));
    assert(1 == graph->total_nodes);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(gnx_has_node(graph, &u));
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = random_node_id(&low, &high);
    assert(gnx_add_node(graph, &u));
    assert(1 == graph->total_nodes);
    v = gnx_any_node(graph);
    assert(u == v);
    assert(1 == graph->total_nodes);
    assert(gnx_has_node(graph, &u));
    gnx_destroy(graph);
}

/* Choose a node from a graph that is directed, allows self-loops, unweighted.
 */
static void
any_node_directed_selfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 7);

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    /* Insert a bunch of self-loops. */
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

    v = gnx_any_node(graph);
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
any_node_directed_selfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 7);

    graph = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    /* Insert a bunch of self-loops. */
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

    v = gnx_any_node(graph);
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
any_node_directed_noselfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    v = gnx_any_node(graph);
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
any_node_directed_noselfloop_weighted(void)
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

    /* Insert a bunch of edges. */
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

    v = gnx_any_node(graph);
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
any_node_undirected_selfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 7);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    /* Insert a bunch of self-loops. */
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

    v = gnx_any_node(graph);
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
any_node_undirected_selfloop_weighted(void)
{
    double weight;
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 7);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    /* Insert a bunch of self-loops. */
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

    v = gnx_any_node(graph);
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
any_node_undirected_noselfloop_unweighted(void)
{
    GnxGraph *graph;
    GnxSet *seen;
    unsigned int i, *node, u, v;
    const int low = 0;
    const int high = 64;
    const unsigned int nedge = (unsigned int)g_random_int_range(2, 32);

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    seen = gnx_init_set_full(GNX_FREE_ELEMENTS);

    /* Insert a bunch of edges. */
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

    v = gnx_any_node(graph);
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
any_node_undirected_noselfloop_weighted(void)
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

    /* Insert a bunch of edges. */
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

    v = gnx_any_node(graph);
    assert(gnx_set_has(seen, &v));
    assert(gnx_has_node(graph, &v));
    assert(seen->size == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
    gnx_destroy_set(seen);
}

/**************************************************************************
 * comparing floating-point numbers
 *************************************************************************/

static void
compare_double(void)
{
    compare_double_eq();
    compare_double_le();
}

/* Compare two floating-point numbers a and b for the relation a == b.
 */
static void
compare_double_eq(void)
{
    double a, b;
    const double low = -100.0;
    const double high = 100.0;
    const double mid = 0.0;

    assert(gnx_double_cmp_eq(&high, &high));
    assert(!gnx_double_cmp_eq(&low, &high));

    a = g_random_double_range(low, mid);
    b = g_random_double_range(mid, high);
    assert(!gnx_double_cmp_eq(&a, &b));
}

/* Compare two floating-point numbers a and b for the relation a <= b.
 */
static void
compare_double_le(void)
{
    double a, b;
    unsigned int i;
    const double num[3] = {-100.0, 0.0, 100.0};
    const double low = num[0];
    const double mid = num[1];
    const double high = num[2];

    assert(gnx_double_cmp_le(&low, &mid));
    assert(gnx_double_cmp_le(&low, &high));
    assert(gnx_double_cmp_le(&mid, &high));

    i = (unsigned int)g_random_int_range(0, 3);
    assert(gnx_double_cmp_le(&num[i], &num[i]));

    a = g_random_double_range(low, mid);
    b = g_random_double_range(mid, high);
    assert(gnx_double_cmp_le(&a, &b));
    assert(gnx_double_cmp_le(&a, &a));
    assert(gnx_double_cmp_le(&b, &b));
    assert(!gnx_double_cmp_le(&b, &a));
}

/**************************************************************************
 * comparing graphs
 *************************************************************************/

static void
compare_graph(void)
{
    compare_graph_directed();
    compare_graph_edges();
    compare_graph_empty();
    compare_graph_nodes();
    compare_graph_weighted();
}

/* Compare the edges of two digraphs.
 */
static void
compare_graph_directed(void)
{
    GnxGraph *g, *h;
    unsigned int u, v;

    /* An empty digraph versus a digraph with one edge. */
    g = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    h = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = 2;
    v = 3;
    assert(gnx_add_edge(h, &u, &v));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Digraphs with the same edges. */
    g = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = 1;
    v = 0;
    assert(gnx_add_edge(g, &u, &v));
    h = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(h, &u, &v));
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Digraphs with different edges. */
    g = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = 0;
    v = 1;
    assert(gnx_add_edge(g, &u, &v));
    v = 2;
    assert(gnx_add_edge(g, &u, &v));
    h = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    u = 0;
    v = 1;
    assert(gnx_add_edge(h, &u, &v));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
}

/* Compare the edges of two graphs.
 */
static void
compare_graph_edges(void)
{
    GnxGraph *g, *h;
    unsigned int u, v;

    /* An empty graph versus a graph with one edge. */
    g = gnx_new();
    h = gnx_new();
    u = 2;
    v = 3;
    assert(gnx_add_edge(h, &u, &v));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with the same edges. */
    g = gnx_new();
    u = 1;
    v = 0;
    assert(gnx_add_edge(g, &u, &v));
    h = gnx_new();
    u = 0;
    v = 1;
    assert(gnx_add_edge(h, &u, &v));
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with different edges. */
    g = gnx_new();
    u = 0;
    v = 1;
    assert(gnx_add_edge(g, &u, &v));
    v = 2;
    assert(gnx_add_edge(g, &u, &v));
    h = gnx_new();
    u = 0;
    v = 1;
    assert(gnx_add_edge(h, &u, &v));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
}

/* Compare empty graphs.
 */
static void
compare_graph_empty(void)
{
    GnxGraph *g, *h;

    /* Empty graphs with default properties. */
    g = gnx_new();
    h = gnx_new();
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Empty graphs with custom properties. */
    g = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    h = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Empty graphs with different properties. */
    /* directedness */
    g = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    h = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
    /* allowance for self-loops */
    g = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    h = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
    /* weightedness */
    g = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    h = gnx_new_full(GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
}

/* Compare the nodes of two graphs.
 */
static void
compare_graph_nodes(void)
{
    GnxGraph *g, *h;
    unsigned int u, v;

    /* An empty graph versus a graph with one node. */
    g = gnx_new();
    h = gnx_new();
    u = 0;
    assert(gnx_add_node(h, &u));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with the same nodes. */
    u = 0;
    v = 1;
    g = gnx_new();
    assert(gnx_add_node(g, &u));
    assert(gnx_add_node(g, &v));
    h = gnx_new();
    assert(gnx_add_node(h, &u));
    assert(gnx_add_node(h, &v));
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with different nodes. */
    u = 1;
    v = 3;
    g = gnx_new();
    assert(gnx_add_node(g, &u));
    h = gnx_new();
    assert(gnx_add_node(h, &u));
    assert(gnx_add_node(h, &v));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
}

/* Compare the edges of two weighted graphs.
 */
static void
compare_graph_weighted(void)
{
    double weight;
    GnxGraph *g, *h;
    unsigned int u, v;

    /* An empty graph versus a graph with one edge. */
    g = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    h = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = 2;
    v = 3;
    weight = (double)g_random_double();
    assert(gnx_add_edgew(h, &u, &v, &weight));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with the same edges. */
    g = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = 1;
    v = 0;
    weight = (double)g_random_double();
    assert(gnx_add_edgew(g, &u, &v, &weight));
    h = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(h, &u, &v, &weight));
    assert(gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);

    /* Graphs with different edges. */
    g = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = 0;
    v = 1;
    weight = (double)g_random_double();
    assert(gnx_add_edgew(g, &u, &v, &weight));
    v = 2;
    assert(gnx_add_edgew(g, &u, &v, &weight));
    h = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    u = 0;
    v = 1;
    weight = (double)g_random_double() + 1.0;
    assert(gnx_add_edgew(h, &u, &v, &weight));
    assert(!gnx_cmp(g, h));
    gnx_destroy(g);
    gnx_destroy(h);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/util/any-node", any_node);
    g_test_add_func("/util/compare-double", compare_double);
    g_test_add_func("/util/compare-graph", compare_graph);

    return g_test_run();
}
