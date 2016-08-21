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

/* Test the functions in the module src/query.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <gnx.h>

#include "routine.h"

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

/* connected (undirected graph) */
static void connected_disconnected(void);
static void connected_empty(void);
static void connected_MylesEtAl2011(void);
static void connected_no_memory(void);
static void connected_one_edge(void);
static void connected_one_node(void);
static void connected_Zachary1977(void);

/* is tree */
static void is_tree_bfs_noselfloop_unweighted(void);
static void is_tree_bfs_noselfloop_weighted(void);
static void is_tree_bfs_selfloop_unweighted(void);
static void is_tree_bfs_selfloop_weighted(void);
static void is_tree_dfs_noselfloop_unweighted(void);
static void is_tree_dfs_noselfloop_weighted(void);
static void is_tree_dfs_selfloop_unweighted(void);
static void is_tree_dfs_selfloop_weighted(void);
static void is_tree_empty(void);
static void is_tree_one_edge(void);
static void is_tree_one_node(void);
static void is_tree_only_nodes(void);
static void is_tree_remove_random_edge(void);
static void is_tree_remove_random_node(void);

/**************************************************************************
 * connected (undirected graph)
 *************************************************************************/

static void
connected(void)
{
    connected_disconnected();
    connected_empty();
    connected_MylesEtAl2011();
    connected_no_memory();
    connected_one_edge();
    connected_one_node();
    connected_Zachary1977();
}

/* A connected graph will become disconnected after we add a node that is not
 * in the graph.
 */
static void
connected_disconnected(void)
{
    GnxGraph *graph;
    unsigned int v;
    const int low = 35;
    const int high = 2 * low;
    const unsigned int nnode = 34;
    const unsigned int nedge = 78;

    graph = gnx_read("data/network/karate.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);

    v = random_node_id(&low, &high);
    assert(gnx_add_node(graph, &v));
    assert(!gnx_is_connected(graph));
    assert((nnode + 1) == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);
}

/* The empty graph is not connected.
 */
static void
connected_empty(void)
{
    GnxGraph *graph;

    /* Undirected, self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_connected(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_connected(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_connected(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_connected(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);
}

/* An undirected graph of common grape cultivars.  The graph was adapted from
 * (Myles et al. 2011).
 *
 * S. Myles, A. R. Boyko, C. L. Owens, P. J. Brown, F. Grassi,
 *     M. K. Aradhya, B. Prins, A. Reynolds, J.-M. Chia, D. Ware,
 *     C. D. Bustamante, and E. S. Buckler.  Genetic structure and
 *     domestication history of the grape.  PNAS, 108:3530--3535, 2011.
 */
static void
connected_MylesEtAl2011(void)
{
    GnxGraph *graph;
    const unsigned int v = 48;
    const unsigned int nnode = 54;
    const unsigned int nedge = 80;

    /* The wine network is connected. */
    graph = gnx_read("data/network/wine.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_is_connected(graph));

    /* Now disconnect the network. */
    assert(gnx_delete_node(graph, &v));
    assert(!gnx_is_connected(graph));

    gnx_destroy(graph);
}

/* Test the function gnx_is_connected() under low-memory scenarios.
 */
static void
connected_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    const unsigned int u = 0;
    const unsigned int v = 1;

    graph = gnx_new();
    assert(gnx_add_edge(graph, &u, &v));

    /* Cannot allocate memory for the set of visited nodes. */
    gnx_alloc_set_limit(0);
    assert(!gnx_is_connected(graph));
    assert(ENOMEM == errno);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
}

/* Whether a graph that has one edge is connected.
 */
static void
connected_one_edge(void)
{
    GnxGraph *graph;
    const double weight = 2.71828;
    const unsigned int u = 0;
    const unsigned int v = 1;
    const unsigned int nnode = 2;
    const unsigned int nedge = 1;

    /* Undirected, self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &u));
    assert(gnx_add_node(graph, &v));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(!gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(gnx_add_node(graph, &v));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(!gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    gnx_destroy(graph);
}

/* An undirected graph that has one node is trivially connected.
 */
static void
connected_one_node(void)
{
    GnxGraph *graph;
    const double weight = 3.14159;
    const unsigned int v = 0;

    /* Undirected, self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &v, &v));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loop, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_connected(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);
}

/* The karate club network in (Zachary 1977).  The network has 34 nodes and
 * 78 edges.  Note that the node IDs in the network starts from 1, not zero.
 *
 * W. W. Zachary.  An information flow model for conflict and fission
 *     in small groups.  J. Anthropol. Res., 33:452--473, 1977.
 */
static void
connected_Zachary1977(void)
{
    GnxGraph *graph;
    unsigned int i, v;
    const int low = 1;
    const int high = 34;
    const unsigned int nnode = 34;
    const unsigned int nedge = 78;
    const unsigned int nloop = (unsigned int)g_random_int_range(2, 8);

    /**********************************************************************
     * Undirected, no self-loop, unweighted.
     *********************************************************************/

    graph = gnx_read("data/network/karate.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);

    assert(gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);

    gnx_destroy(graph);

    /**********************************************************************
     * Undirected, self-loop, unweighted.
     *********************************************************************/

    graph = gnx_read("data/network/karate.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);

    /* Insert a bunch of self-loops. */
    for (i = 0; i < nloop; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_edge(graph, &v, &v));

        assert(gnx_add_edge(graph, &v, &v));
    }
    assert(nnode == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    assert(gnx_is_connected(graph));
    assert(nnode == graph->total_nodes);
    assert((nedge + nloop) == graph->total_edges);

    gnx_destroy(graph);
}

/**************************************************************************
 * is tree
 *************************************************************************/

static void
is_tree(void)
{
    is_tree_bfs_noselfloop_unweighted();
    is_tree_bfs_noselfloop_weighted();
    is_tree_bfs_selfloop_unweighted();
    is_tree_bfs_selfloop_weighted();
    is_tree_dfs_noselfloop_unweighted();
    is_tree_dfs_noselfloop_weighted();
    is_tree_dfs_selfloop_unweighted();
    is_tree_dfs_selfloop_weighted();
    is_tree_empty();
    is_tree_one_edge();
    is_tree_one_node();
    is_tree_only_nodes();
    is_tree_remove_random_edge();
    is_tree_remove_random_node();
}

/* A breadth-first search of a graph that is undirected, does not allow
 * self-loops, and unweighted.  The BFS graph is a tree.
 */
static void
is_tree_bfs_noselfloop_unweighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67;

    g = gnx_read("data/network/corporate-interlocks.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_breadth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A breadth-first search of a graph that is undirected, does not allow
 * self-loops, and weighted.  The BFS graph is a tree.
 */
static void
is_tree_bfs_noselfloop_weighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67;

    g = gnx_read("data/network/corporate-interlocks.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_breadth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A breadth-first search of a graph that is undirected, allows self-loops,
 * and unweighted.  The BFS graph is a tree.
 */
static void
is_tree_bfs_selfloop_unweighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nloop = 3;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67 + nloop;

    g = gnx_read("data/network/corporate-interlocks-selfloops.csv",
                 GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_breadth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A breadth-first search of a graph that is undirected, allows self-loops,
 * and weighted.  The BFS graph is a tree.
 */
static void
is_tree_bfs_selfloop_weighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nloop = 3;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67 + nloop;

    g = gnx_read("data/network/corporate-interlocks-selfloops.csv",
                 GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_breadth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A depth-first search of a graph that is undirected, does not allow
 * self-loops, and unweighted.  The DFS graph is a tree.
 */
static void
is_tree_dfs_noselfloop_unweighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67;

    g = gnx_read("data/network/corporate-interlocks.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_depth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A depth-first search of a graph that is undirected, does not allow
 * self-loops, and weighted.  The DFS graph is a tree.
 */
static void
is_tree_dfs_noselfloop_weighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67;

    g = gnx_read("data/network/corporate-interlocks.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_depth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A depth-first search of a graph that is undirected, allows self-loops, and
 * unweighted.  The DFS graph is a tree.
 */
static void
is_tree_dfs_selfloop_unweighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nloop = 3;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67 + nloop;

    g = gnx_read("data/network/corporate-interlocks-selfloops.csv",
                 GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_depth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* A depth-first search of a graph that is undirected, allows self-loops, and
 * weighted.  The DFS graph is a tree.
 */
static void
is_tree_dfs_selfloop_weighted(void)
{
    GnxGraph *g, *h;
    unsigned int start;
    const unsigned int nloop = 3;
    const unsigned int nnode = 15;
    const unsigned int nedge = 67 + nloop;

    g = gnx_read("data/network/corporate-interlocks-selfloops.csv",
                 GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(nnode == g->total_nodes);
    assert(nedge == g->total_edges);

    start = gnx_random_node(g);
    h = gnx_depth_first_search(g, &start);
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    assert(gnx_is_tree(h));
    assert(!h->directed);
    assert(!h->selfloop);
    assert(!h->weighted);
    assert(h->total_edges == (h->total_nodes - 1));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* An empty graph is not a tree.
 */
static void
is_tree_empty(void)
{
    GnxGraph *graph;

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_tree(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_tree(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_tree(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    is_empty_graph(graph);
    assert(!gnx_is_tree(graph));
    is_empty_graph(graph);
    gnx_destroy(graph);
}

/* Whether a graph is a tree.  The graph has exactly one edge.
 */
static void
is_tree_one_edge(void)
{
    GnxGraph *graph;
    const double weight = 2.17828;
    const unsigned int u = 0;
    const unsigned int v = 1;

    /* Undirected, self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &u));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_is_tree(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &u, &weight));
    assert(gnx_add_node(graph, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_is_tree(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);
}

/* A graph that has one node may or may not be a tree.
 */
static void
is_tree_one_node(void)
{
    GnxGraph *graph;
    const double weight = 2.17828;
    const unsigned int v = 1;

    /* Undirected, self-loops, unweighted (1.1). */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, unweighted (1.2). */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &v, &v));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted (2.1). */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, self-loops, weighted (2.2). */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &v, &v, &weight));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_node(graph, &v));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    assert(gnx_is_tree(graph));
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);
}

/* A graph that has only nodes is not a tree.
 */
static void
is_tree_only_nodes(void)
{
    GnxGraph *graph;
    unsigned int v;
    const int low = 2;
    const int high = 20;
    const int max = 11;
    const unsigned int size = (unsigned int)g_random_int_range(low, max);

    /**********************************************************************
     * Undirected, self-loops, unweighted.
     *********************************************************************/

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);

    while (graph->total_nodes < size) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_node(graph, &v));

        assert(gnx_add_node(graph, &v));
    }
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(!gnx_is_tree(graph));
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);

    /**********************************************************************
     * Undirected, self-loops, weighted.
     *********************************************************************/

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);

    while (graph->total_nodes < size) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_node(graph, &v));

        assert(gnx_add_node(graph, &v));
    }
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(!gnx_is_tree(graph));
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);

    /**********************************************************************
     * Undirected, no self-loops, unweighted.
     *********************************************************************/

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    while (graph->total_nodes < size) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_node(graph, &v));

        assert(gnx_add_node(graph, &v));
    }
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(!gnx_is_tree(graph));
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);

    /**********************************************************************
     * Undirected, no self-loops, weighted.
     *********************************************************************/

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);

    while (graph->total_nodes < size) {
        do {
            v = random_node_id(&low, &high);
        } while (gnx_has_node(graph, &v));

        assert(gnx_add_node(graph, &v));
    }
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    assert(!gnx_is_tree(graph));
    assert(size == graph->total_nodes);
    assert(0 == graph->total_edges);

    gnx_destroy(graph);
}

/* Remove some edges from a graph and test whether the resulting graph is
 * a tree.
 */
static void
is_tree_remove_random_edge(void)
{
    GnxGraph *g, *h;
    int high;
    unsigned int how_many, i, u, v;
    const int low = 0;
    const unsigned int start = 48;

    /* Construct a breadth-first search tree. */
    g = gnx_read("data/network/wine.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    h = gnx_breadth_first_search(g, &start);
    assert(gnx_is_tree(h));

    /* Randomly delete some edges from the tree. */
    high = (int)(h->total_nodes);
    how_many = (unsigned int)g_random_int_range(1, 8);
    for (i = 0; i < how_many; i++) {
        do {
            u = random_node_id(&low, &high);
            v = random_node_id(&low, &high);
        } while (!gnx_has_edge(h, &u, &v));

        assert(gnx_delete_edge(h, &u, &v));
    }

    assert(!gnx_is_tree(h));

    gnx_destroy(g);
    gnx_destroy(h);
}

/* Remove some nodes from a graph and test whether the resulting graph is
 * a tree.
 */
static void
is_tree_remove_random_node(void)
{
    GnxGraph *g, *h;
    int high;
    unsigned int how_many, i, v;
    const int low = 0;
    const unsigned int start = 48;

    /* Construct a breadth-first search tree. */
    g = gnx_read("data/network/wine.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    h = gnx_breadth_first_search(g, &start);
    assert(gnx_is_tree(h));

    /* Randomly delete some nodes from the tree. */
    high = (int)(h->total_nodes);
    how_many = (unsigned int)g_random_int_range(1, 8);
    for (i = 0; i < how_many; i++) {
        do {
            v = random_node_id(&low, &high);
        } while (!gnx_has_node(h, &v));

        assert(gnx_delete_node(h, &v));
    }

    assert(!gnx_is_tree(h));

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

    g_test_add_func("/query/connected", connected);
    g_test_add_func("/query/is-tree", is_tree);

    return g_test_run();
}
