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

/* Test the functions in the module src/visit.c.
 */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <gnx.h>

#include "constant.h"
#include "routine.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* breadth-first search */
static void bfs_directed(void);
static void bfs_directed_weighted(void);
static void bfs_graph(void);
static void bfs_isolated(void);
static void bfs_no_memory(void);
static void bfs_one_directed_edge(void);
static void bfs_one_node(void);
static void bfs_one_undirected_edge(void);
static void bfs_undirected_Flamig1995(void);
static void bfs_undirected_MylesEtAl2011(void);
static void bfs_undirected_SedgewickWayne2011(void);
static void bfs_undirected_Skiena2008(void);
static void bfs_undirected_two_components(void);

/* depth-first search */
static void dfs_advogato(void);
static void dfs_isolated(void);
static void dfs_no_memory(void);
static void dfs_undirected_graph_MylesEtAl2011(void);
static void dfs_undirected_triangle_graph(void);
static void dfs_directed_unweighted(void);
static void dfs_directed_weighted(void);
static void dfs_undirected_unweighted(void);
static void dfs_undirected_weighted(void);

/**************************************************************************
 * breadth-first search
 *************************************************************************/

static void
bfs(void)
{
    bfs_directed();
    bfs_directed_weighted();
    bfs_graph();
    bfs_isolated();
    bfs_no_memory();
    bfs_one_directed_edge();
    bfs_one_node();
    bfs_one_undirected_edge();
    bfs_undirected_Flamig1995();
    bfs_undirected_MylesEtAl2011();
    bfs_undirected_SedgewickWayne2011();
    bfs_undirected_Skiena2008();
    bfs_undirected_two_components();
}

/* Breadth-first search a digraph.
 */
static void
bfs_directed(void)
{
    GnxGraph *graph, *g, *tree;
    const unsigned int tail[7] = {0, 1, 1, 2, 2, 3, 4};
    const unsigned int head[7] = {1, 2, 3, 0, 4, 5, 5};
    const unsigned int size = 7;
    const unsigned int taila[5] = {0, 1, 1, 2, 3};
    const unsigned int heada[5] = {1, 2, 3, 4, 5};
    const unsigned int tree_size = 5;
    const unsigned int start = 0;

    /* Get the BFS tree. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    g = gnx_breadth_first_search(graph, &start);

    /* Compare with the known directed BFS tree. */
    tree = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(tree, taila, heada, &tree_size);
    assert(gnx_cmp(g, tree));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(tree);
}

/* Breadth-first search a weighted digraph.
 */
static void
bfs_directed_weighted(void)
{
    GnxGraph *g, *graph;
    unsigned int start;
    const unsigned int nnode = 7419;
    const unsigned int nedge = 56446;
    const int low = 0;
    const int high = (int)nnode;

    /* Read in the graph from file. */
    graph = gnx_read("data/network/advogato.csv",
                     GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_disable_selfloop(graph));
    assert(nnode >= graph->total_nodes);
    assert(nedge >= graph->total_edges);
    assert(graph->directed);
    assert(!graph->selfloop);
    assert(graph->weighted);

    /* Choose a node that is not isolated.  Refer to the internal function
     * is_isolated() for the definition of isolated node in a directed graph.
     */
    do {
        start = random_node_id(&low, &high);
    } while (is_isolated(graph, &start));

    g = gnx_breadth_first_search(graph, &start);
    assert(gnx_is_directed(g));
    assert(!graph->selfloop);
    assert(!gnx_is_weighted(g));
    assert(nnode >= g->total_nodes);
    assert(nedge >= g->total_edges);

    gnx_destroy(graph);
    gnx_destroy(g);
}

/* Breadth-first search a graph.
 */
static void
bfs_graph(void)
{
    GnxGraph *g, *graph;
    unsigned int n;
    const unsigned int tail[3] = {0, 0, 6};
    const unsigned int head[3] = {1, 5, 1};
    const unsigned int size = 3;
    const unsigned int start = 0;

    /* Undirected graph. */
    graph = gnx_new();
    add_edges(graph, tail, head, &size);
    g = gnx_breadth_first_search(graph, &start);
    assert(3 == g->total_edges);
    assert(4 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(n == size);
    assert(gnx_has_edge(g, &head[0], &tail[0]));
    gnx_destroy(graph);
    gnx_destroy(g);

    /* Directed graph. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    g = gnx_breadth_first_search(graph, &start);
    assert(2 == g->total_edges);
    assert(3 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(2 == n);
    gnx_destroy(graph);
    gnx_destroy(g);
}

/* Breadth-first search starting from a node that is isolated.
 */
static void
bfs_isolated(void)
{
    GnxGraph *graph;
    const double weight = (double)g_random_double();
    const unsigned int u = 0;
    const unsigned int v = 1;
    const unsigned int w = 2;

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_breadth_first_search(graph, &v));
    assert(!gnx_breadth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_breadth_first_search(graph, &v));
    assert(!gnx_breadth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_breadth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_breadth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);
}

/* Test the function gnx_breadth_first_search() under low-memory scenarios.
 */
static void
bfs_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    int alloc_size;
    const unsigned int u = 2;
    const unsigned int v = 3;

    graph = gnx_new();
    assert(gnx_add_edge(graph, &u, &v));

    /* Cannot add the start node to the set of seen nodes. */
    alloc_size = GNX_ALLOC_SIZE
        + GNX_ALLOC_SET_SIZE
        + GNX_ALLOC_QUEUE_SIZE
        + 1;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_breadth_first_search(graph, &u));
    assert(ENOMEM == errno);

    /* Cannot add another node to the set of seen nodes. */
    alloc_size += GNX_ALLOC_ARRAY_SIZE + 1;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_breadth_first_search(graph, &u));
    assert(ENOMEM == errno);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
}

/* Breadth-first search a digraph that has one edge.
 */
static void
bfs_one_directed_edge(void)
{
    GnxGraph *g, *graph;
    unsigned int n;
    const unsigned int u = 1;
    const unsigned int v = 2;

    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));

    /* Breadth-first search from the tail node. */
    g = gnx_breadth_first_search(graph, &u);
    assert(1 == g->total_edges);
    assert(2 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(1 == n);
    gnx_destroy(g);

    /* Breadth-first search from the head node. */
    g = gnx_breadth_first_search(graph, &v);
    assert(!g);
    gnx_destroy(g);

    gnx_destroy(graph);
}

/* Breadth-first search a graph that has one node.
 */
static void
bfs_one_node(void)
{
    GnxGraph *graph;
    const unsigned int s = 0;

    /* Undirected graph. */
    graph = gnx_new();
    assert(gnx_add_node(graph, &s));
    assert(NULL == gnx_breadth_first_search(graph, &s));
    gnx_destroy(graph);

    /* Directed graph. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_node(graph, &s));
    assert(NULL == gnx_breadth_first_search(graph, &s));
    gnx_destroy(graph);
}

/* Breadth-first search an undirected graph that has one edge.
 */
static void
bfs_one_undirected_edge(void)
{
    GnxGraph *g, *graph;
    unsigned int n;
    const unsigned int u = 2;
    const unsigned int v = 0;

    graph = gnx_new();
    assert(gnx_add_edge(graph, &u, &v));
    g = gnx_breadth_first_search(graph, &v);
    assert(1 == g->total_edges);
    assert(2 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(1 == n);

    gnx_destroy(graph);
    gnx_destroy(g);
}

/* The undirected graph from Figure 14.1 on page 342 in (Flamig 1995).  The
 * BFS tree is in Figure 14.16 on page 367.
 *
 * B. Flamig.  Practical Algorithms in C++.  John Wiley & Sons, 1995.
 */
static void
bfs_undirected_Flamig1995(void)
{
    GnxGraph *graph, *g, *h;
    unsigned int i;
    const unsigned int start = 0;
    const unsigned int ntry = 100;
    /* undirected graph */
    const unsigned int tail[8] = {0, 0, 0, 0, 2, 4, 4, 6};
    const unsigned int head[8] = {1, 2, 4, 7, 3, 5, 6, 7};
    const unsigned int size = 8;
    /* BFS tree */
    const unsigned int bfstail[7] = {0, 0, 0, 0, 2, 4, 7};
    const unsigned int bfshead[7] = {1, 2, 4, 7, 3, 5, 6};
    const unsigned int bfssize = 7;

    graph = gnx_new();
    add_edges(graph, tail, head, &size);

    /* A known BFS tree. */
    h = gnx_new();
    add_edges(h, bfstail, bfshead, &bfssize);

    /* Keep on generating a BFS tree and compare the resulting tree with a
     * known BFS tree.  We do so because there is more than one BFS for a given
     * graph.  The function gnx_breadth_first_search() returns one BFS tree.
     */
    i = 0;
    while (i < ntry) {
        g = gnx_breadth_first_search(graph, &start);
        if (gnx_cmp(g, h))
            break;

        i++;
        gnx_destroy(g);
        g = NULL;
    }

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(h);
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
bfs_undirected_MylesEtAl2011(void)
{
    GnxGraph *graph, *g, *h;
    unsigned int i;
    const unsigned int ntry = 100;
    const unsigned int start = 48;

    /* The grape network. */
    graph = gnx_read("data/network/wine.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(54 == graph->total_nodes);
    assert(80 == graph->total_edges);

    /* A reference BFS tree. */
    h = gnx_read("data/visit/wine-bfs.csv",
                 GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);

    /* Keep on generating a BFS tree and compare the resulting tree with a
     * known BFS tree.  We do so because there is more than one BFS for a given
     * graph.  The function gnx_breadth_first_search() returns one BFS tree.
     */
    i = 0;
    while (i < ntry) {
        g = gnx_breadth_first_search(graph, &start);
        assert(54 == g->total_nodes);
        assert(53 == g->total_edges);
        if (gnx_cmp(g, h))
            break;

        i++;
        gnx_destroy(g);
        g = NULL;
    }

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(h);
}

/* The undirected graph on page 539 in (Sedgewick & Wayne 2011).
 *
 * R. Sedgewick and K. Wayne.  Algorithms.  4th edition, Addison-Wesley, 2011.
 */
static void
bfs_undirected_SedgewickWayne2011(void)
{
    GnxGraph *graph, *g, *h;
    unsigned int i;
    const unsigned int ntry = 100;
    const unsigned int start = 0;
    /* the undirected graph */
    const unsigned int tail[8] = {0, 0, 0, 1, 2, 2, 3, 3};
    const unsigned int head[8] = {1, 2, 5, 2, 3, 4, 4, 5};
    const unsigned int size = 8;
    /* the known BFS tree */
    const unsigned int bfstail[5] = {0, 0, 0, 2, 2};
    const unsigned int bfshead[5] = {1, 2, 5, 3, 4};
    const unsigned int bfssize = 5;

    /* The BFS tree. */
    graph = gnx_new();
    add_edges(graph, tail, head, &size);

    /* The known BFS tree. */
    h = gnx_new();
    add_edges(h, bfstail, bfshead, &bfssize);

    /* Keep on generating a BFS tree and compare the resulting tree with a
     * known BFS tree.  We do so because there is more than one BFS for a given
     * graph.  The function gnx_breadth_first_search() returns one BFS tree.
     */
    i = 0;
    while (i < ntry) {
        g = gnx_breadth_first_search(graph, &start);
        if (gnx_cmp(g, h))
            break;

        i++;
        gnx_destroy(g);
        g = NULL;
    }

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(h);
}

/* The undirected graph from Figure 5.9 on page 162 in (Skiena 2008).
 *
 * S. S. Skiena.  The Algorithm Design Manual. 2nd edition, Springer, 2008.
 */
static void
bfs_undirected_Skiena2008(void)
{
    GnxGraph *graph, *g, *h;
    unsigned int i;
    const unsigned int ntry = 100;
    const unsigned int start = 1;
    /* the undirected graph */
    const unsigned int tail[7] = {1, 1, 1, 2, 2, 3, 4};
    const unsigned int head[7] = {2, 5, 6, 3, 5, 4, 5};
    const unsigned int size = 7;
    /* the known BFS tree */
    const unsigned int bfstail[5] = {1, 1, 1, 2, 5};
    const unsigned int bfshead[5] = {2, 5, 6, 3, 4};
    const unsigned int bfssize = 5;

    /* The BFS tree. */
    graph = gnx_new();
    add_edges(graph, tail, head, &size);

    /* The known BFS tree from (Skiena 2008). */
    h = gnx_new();
    add_edges(h, bfstail, bfshead, &bfssize);

    /* Keep on generating a BFS tree and compare the resulting tree with a
     * known BFS tree.  We do so because there is more than one BFS for a given
     * graph.  The function gnx_breadth_first_search() returns one BFS tree.
     */
    i = 0;
    while (i < ntry) {
        g = gnx_breadth_first_search(graph, &start);
        if (gnx_cmp(g, h))
            break;

        i++;
        gnx_destroy(g);
        g = NULL;
    }

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(h);
}

/* Breadth-first search an undirected graph that has two components.
 */
static void
bfs_undirected_two_components(void)
{
    GnxGraph *g, *graph;
    unsigned int n, start;
    const unsigned int tail[3] = {0, 0, 7};
    const unsigned int head[3] = {1, 5, 6};
    const unsigned int size = 3;

    graph = gnx_new();
    add_edges(graph, tail, head, &size);

    /* Breadth-first search on the larger component. */
    start = 0;
    g = gnx_breadth_first_search(graph, &start);
    assert(2 == g->total_edges);
    assert(3 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(2 == n);
    gnx_destroy(g);

    /* Breadth-first search on the smaller component. */
    start = 6;
    g = gnx_breadth_first_search(graph, &start);
    assert(1 == g->total_edges);
    assert(2 == g->total_nodes);
    n = verify_edges(graph, g);
    assert(1 == n);
    gnx_destroy(g);

    gnx_destroy(graph);
}

/**************************************************************************
 * depth-first search
 *************************************************************************/

static void
dfs(void)
{
    dfs_advogato();
    dfs_isolated();
    dfs_no_memory();
    dfs_undirected_graph_MylesEtAl2011();
    dfs_undirected_triangle_graph();
    dfs_directed_unweighted();
    dfs_directed_weighted();
    dfs_undirected_unweighted();
    dfs_undirected_weighted();
}

/* Depth-first search a graph that is directed, does not allow self-loops,
 * and is weighted.
 */
static void
dfs_advogato(void)
{
    GnxGraph *g, *graph;
    unsigned int start;
    const unsigned int nnode = 7419;
    const unsigned int nedge = 56446;
    const int low = 0;
    const int high = (int)nnode;

    graph = gnx_read("data/network/advogato.csv",
                     GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(nnode == graph->total_nodes);
    assert(nedge == graph->total_edges);
    assert(gnx_disable_selfloop(graph));
    assert(gnx_is_directed(graph));
    assert(!gnx_allows_selfloop(graph));
    assert(gnx_is_weighted(graph));

    /* Choose a node with at least one out-degree. */
    do {
        start = random_node_id(&low, &high);
    } while (is_isolated(graph, &start));

    g = gnx_depth_first_search(graph, &start);
    assert(gnx_is_directed(g));
    assert(!gnx_allows_selfloop(g));
    assert(!gnx_is_weighted(g));
    assert(nnode >= g->total_nodes);
    assert(nedge >= g->total_edges);

    gnx_destroy(graph);
    gnx_destroy(g);
}

/* Depth-first search starting from a node that is isolated.
 */
static void
dfs_isolated(void)
{
    GnxGraph *graph;
    const double weight = (double)g_random_double();
    const unsigned int u = 0;
    const unsigned int v = 1;
    const unsigned int w = 2;

    /* Directed, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_depth_first_search(graph, &v));
    assert(!gnx_depth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Directed, no self-loops, weighted. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_depth_first_search(graph, &v));
    assert(!gnx_depth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, unweighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(gnx_add_edge(graph, &u, &v));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_depth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    /* Undirected, no self-loops, weighted. */
    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(gnx_add_edgew(graph, &u, &v, &weight));
    assert(gnx_add_node(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(!gnx_depth_first_search(graph, &w));
    assert(3 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);
}

/* Test the function gnx_depth_first_search() under low-memory scenarios.
 */
static void
dfs_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *graph;
    int alloc_size;
    const unsigned int u = 3;
    const unsigned int v = 5;

    graph = gnx_new();
    assert(gnx_add_edge(graph, &u, &v));

    /* Cannot initialize the depth-first search tree. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_depth_first_search(graph, &u));
    assert(ENOMEM == errno);

    /* Cannot initialize the dictionary of parent nodes. */
    alloc_size = GNX_ALLOC_SIZE
        + GNX_ALLOC_SET_SIZE
        + GNX_ALLOC_STACK_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_depth_first_search(graph, &u));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to hold a node that we have seen. */
    alloc_size += GNX_ALLOC_DICT_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_depth_first_search(graph, &u));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to hold the parent of a node. */
    alloc_size += GNX_ALLOC_BUCKET_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_depth_first_search(graph, &u));
    assert(ENOMEM == errno);

    gnx_destroy(graph);
    gnx_alloc_reset_limit();
#endif
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
dfs_undirected_graph_MylesEtAl2011(void)
{
    GnxGraph *graph, *g;
    const unsigned int start = 48;

    /* The wine network. */
    graph = gnx_read("data/network/wine.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!gnx_is_directed(graph));
    assert(54 == graph->total_nodes);
    assert(80 == graph->total_edges);

    /* A depth-first search tree. */
    g = gnx_depth_first_search(graph, &start);
    assert(54 == g->total_nodes);
    assert(53 == g->total_edges);

    gnx_destroy(graph);
    gnx_destroy(g);
}

/* Depth-first search the triangle graph that is undirected, does not allow
 * self-loops, and is unweighted.
 */
static void
dfs_undirected_triangle_graph(void)
{
    GnxGraph *graph, *g, *treea, *treeb;
    const unsigned int tail[3] = {0, 0, 1};
    const unsigned int head[3] = {1, 2, 2};
    const unsigned int size = 3;
    const unsigned int tree_size = 2;
    const unsigned int taila[3] = {0, 1};
    const unsigned int heada[3] = {1, 2};
    const unsigned int tailb[3] = {0, 1};
    const unsigned int headb[3] = {2, 2};
    const unsigned int start = 0;

    graph = gnx_new();
    add_edges(graph, tail, head, &size);
    g = gnx_depth_first_search(graph, &start);

    /* The triangle graph is as follows:
     *
     *         0
     *        / \
     *       1---2
     *
     * If we do a depth-first search from 0, then we either have the edge
     * (0,1) or the edge (0,2).  In either case, we have the edge (1,2).
     * Thus we either have the DFS tree:
     *
     *         0
     *        /
     *       1---2
     *
     * or the DFS tree:
     *
     *         0
     *          \
     *       1---2
     */
    treea = gnx_new();
    add_edges(treea, taila, heada, &tree_size);
    treeb = gnx_new();
    add_edges(treeb, tailb, headb, &tree_size);
    assert(gnx_cmp(g, treea) || gnx_cmp(g, treeb));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(treea);
    gnx_destroy(treeb);
}

/* Depth-first search a graph that is directed, does not allow self-loops, and
 * is unweighted.
 */
static void
dfs_directed_unweighted(void)
{
    GnxGraph *graph, *g, *treea, *treeb;
    const unsigned int tail[7] = {0, 1, 1, 2, 2, 3, 4};
    const unsigned int head[7] = {1, 2, 3, 0, 4, 5, 5};
    const unsigned int size = 7;
    const unsigned int taila[5] = {0, 1, 1, 2, 4};
    const unsigned int heada[5] = {1, 2, 3, 4, 5};
    const unsigned int tailb[5] = {0, 1, 1, 2, 3};
    const unsigned int headb[5] = {1, 2, 3, 4, 5};
    const unsigned int tree_size = 5;
    const unsigned int start = 0;

    /* Get the DFS tree. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    g = gnx_depth_first_search(graph, &start);

    /* For the given directed graph, we have either of the following two
     * directed DFS trees:
     *
     *     (0,1)    (0,1)
     *     (1,2)    (1,2)
     *     (1,3)    (1,3)
     *     (2,4)    (2,4)
     *     (4,5)    (3,5)
     */
    treea = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(treea, taila, heada, &tree_size);
    treeb = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(treeb, tailb, headb, &tree_size);
    assert(gnx_cmp(g, treea) || gnx_cmp(g, treeb));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(treea);
    gnx_destroy(treeb);
}

/* Depth-first search a graph that is directed, does not allow self-loops, and
 * is weighted.
 */
static void dfs_directed_weighted(void)
{
    GnxGraph *graph, *g, *treea, *treeb;
    const double weight[7]     = {0, 1, 2, 3, 4, 5, 6};
    const unsigned int tail[7] = {0, 1, 1, 2, 2, 3, 4};
    const unsigned int head[7] = {1, 2, 3, 0, 4, 5, 5};
    const unsigned int size = 7;
    const unsigned int taila[5] = {0, 1, 1, 2, 4};
    const unsigned int heada[5] = {1, 2, 3, 4, 5};
    const unsigned int tailb[5] = {0, 1, 1, 2, 3};
    const unsigned int headb[5] = {1, 2, 3, 4, 5};
    const unsigned int tree_size = 5;
    const unsigned int start = 0;

    /* Get the DFS tree. */
    graph = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(graph, tail, head, weight, &size);
    g = gnx_depth_first_search(graph, &start);

    /* For the given directed graph, we have either of the following two
     * directed DFS trees:
     *
     *     (0,1)    (0,1)
     *     (1,2)    (1,2)
     *     (1,3)    (1,3)
     *     (2,4)    (2,4)
     *     (4,5)    (3,5)
     */
    treea = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(treea, taila, heada, &tree_size);
    treeb = gnx_new_full(GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(treeb, tailb, headb, &tree_size);
    assert(gnx_cmp(g, treea) || gnx_cmp(g, treeb));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(treea);
    gnx_destroy(treeb);
}

/* Depth-first search a graph that is undirected, does not allow self-loops,
 * and is unweighted.
 */
static void
dfs_undirected_unweighted(void)
{
    GnxGraph *graph, *g, *treea, *treeb, *treec, *treed;
    const unsigned int tail[5] = {0, 0, 1, 1, 2};
    const unsigned int head[5] = {1, 2, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int taila[3] = {0, 1, 2};
    const unsigned int heada[3] = {1, 2, 3};
    const unsigned int tailb[3] = {0, 1, 3};
    const unsigned int headb[3] = {1, 3, 2};
    const unsigned int tailc[3] = {0, 2, 1};
    const unsigned int headc[3] = {2, 1, 3};
    const unsigned int taild[3] = {0, 2, 3};
    const unsigned int headd[3] = {2, 3, 1};
    const unsigned int tree_size = 3;
    const unsigned int start = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(graph, tail, head, &size);
    g = gnx_depth_first_search(graph, &start);
    assert(4 == g->total_nodes);
    assert(3 == g->total_edges);

    /* Given the above diamond graph, a depth-first search from 0 should
     * result in one of the following four DFS trees:
     *
     * (0,1)    (0,1)    (0,2)    (0,2)
     * (1,2)    (1,3)    (2,1)    (2,3)
     * (2,3)    (3,2)    (1,3)    (3,1)
     */
    treea = gnx_new();
    add_edges(treea, taila, heada, &tree_size);
    treeb = gnx_new();
    add_edges(treeb, tailb, headb, &tree_size);
    treec = gnx_new();
    add_edges(treec, tailc, headc, &tree_size);
    treed = gnx_new();
    add_edges(treed, taild, headd, &tree_size);
    assert(gnx_cmp(g, treea)
           || gnx_cmp(g, treeb)
           || gnx_cmp(g, treec)
           || gnx_cmp(g, treed));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(treea);
    gnx_destroy(treeb);
    gnx_destroy(treec);
    gnx_destroy(treed);
}

/* Depth-first search a graph that is undirected, does not allow self-loops,
 * and is weighted.
 */
static void
dfs_undirected_weighted(void)
{
    GnxGraph *graph, *g, *treea, *treeb, *treec, *treed;
    const double weight[5]     = {0, 1, 2, 3, 4};
    const unsigned int tail[5] = {0, 0, 1, 1, 2};
    const unsigned int head[5] = {1, 2, 2, 3, 3};
    const unsigned int size = 5;
    const unsigned int taila[3] = {0, 1, 2};
    const unsigned int heada[3] = {1, 2, 3};
    const unsigned int tailb[3] = {0, 1, 3};
    const unsigned int headb[3] = {1, 3, 2};
    const unsigned int tailc[3] = {0, 2, 1};
    const unsigned int headc[3] = {2, 1, 3};
    const unsigned int taild[3] = {0, 2, 3};
    const unsigned int headd[3] = {2, 3, 1};
    const unsigned int tree_size = 3;
    const unsigned int start = 0;

    graph = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(graph, tail, head, weight, &size);
    g = gnx_depth_first_search(graph, &start);
    assert(4 == g->total_nodes);
    assert(3 == g->total_edges);

    /* Given the above diamond graph, a depth-first search from 0 should
     * result in one of the following four DFS trees:
     *
     * (0,1)    (0,1)    (0,2)    (0,2)
     * (1,2)    (1,3)    (2,1)    (2,3)
     * (2,3)    (3,2)    (1,3)    (3,1)
     */
    treea = gnx_new();
    add_edges(treea, taila, heada, &tree_size);
    treeb = gnx_new();
    add_edges(treeb, tailb, headb, &tree_size);
    treec = gnx_new();
    add_edges(treec, tailc, headc, &tree_size);
    treed = gnx_new();
    add_edges(treed, taild, headd, &tree_size);
    assert(gnx_cmp(g, treea)
           || gnx_cmp(g, treeb)
           || gnx_cmp(g, treec)
           || gnx_cmp(g, treed));

    gnx_destroy(graph);
    gnx_destroy(g);
    gnx_destroy(treea);
    gnx_destroy(treeb);
    gnx_destroy(treec);
    gnx_destroy(treed);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/visit/bfs", bfs);
    g_test_add_func("/visit/dfs", dfs);

    return g_test_run();
}
