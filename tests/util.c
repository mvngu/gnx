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

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

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

    g_test_add_func("/util/compare-double", compare_double);
    g_test_add_func("/util/compare-graph", compare_graph);

    return g_test_run();
}
