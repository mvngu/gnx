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

/* bottom-up traversal */
static void bottom_up_advogato(void);
static void bottom_up_flying_teams(void);
static void bottom_up_no_memory(void);
static void bottom_up_small_tree(void);

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

/* pre-order traversal */
static void pre_order_flying_teams_bfs(void);
static void pre_order_no_memory(void);
static void pre_order_small_tree_unweighted(void);
static void pre_order_small_tree_weighted(void);
static void pre_order_traversal(void);

/**************************************************************************
 * bottom-up traversal
 *************************************************************************/

static void
bottom_up(void)
{
    bottom_up_advogato();
    bottom_up_flying_teams();
    bottom_up_no_memory();
    bottom_up_small_tree();
}

/* A bottom-up traversal of a breadth-first search tree of a snapshot of the
 * Advogato trust network.
 */
static void
bottom_up_advogato(void)
{
    GnxArray *list;
    GnxGraph *tree;
    GnxNodeIter iter;
    GnxSet *leaf;
    unsigned int i, max, size, start, *u, v;
    const unsigned int root = 12;
    const unsigned int nnode = 5290;
    const unsigned int nedge = 5289;
    const unsigned int root_index = nnode - 1;

    /* Get the bottom-up traversal. */
    tree = gnx_read("data/visit/advogato-bfs.csv",
                    GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);
    list = gnx_bottom_up(tree, &root);
    assert(list);
    assert(list->size == tree->total_nodes);

    /* Verify the traversal. */
    max = 0;
    while (max < root_index) {
        /* Get the set of leaves of the tree, excluding the root node. */
        leaf = gnx_init_set_full(GNX_FREE_ELEMENTS);
        gnx_node_iter_init(&iter, tree);
        while (gnx_node_iter_next(&iter, &v)) {
            if ((1 == gnx_degree(tree, &v)) && (v != root)) {
                u = (unsigned int *)malloc(sizeof(unsigned int));
                assert(u);
                *u = v;
                assert(gnx_set_add(leaf, u));
            }
        }

        /* Compare the leaves with the traversal. */
        size = leaf->size;
        start = max;
        max += size;
        for (i = start; i < max; i++) {
            u = (unsigned int *)(list->cell[i]);
            assert(gnx_set_has(leaf, u));
            assert(gnx_set_delete(leaf, u));
            assert(gnx_has_node(tree, u));
            assert(gnx_delete_node(tree, u));
        }
        assert(0 == leaf->size);
        gnx_destroy_set(leaf);
    }

    /* Only the root node remains. */
    assert(1 == tree->total_nodes);
    assert(gnx_has_node(tree, &root));
    u = (unsigned int *)(list->cell[root_index]);
    assert(*u == root);

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

/* A bottom-up traversal of a breadth-first search tree of the flying teams
 * network.
 */
static void
bottom_up_flying_teams(void)
{
    GnxArray *list;
    GnxGraph *tree;
    unsigned int i, *node;
    const unsigned int root = 1;
    const unsigned int nnode = 48;
    const unsigned int nedge = 47;

    tree = gnx_read("data/visit/flying-teams-bfs.csv",
                    GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);
    list = gnx_bottom_up(tree, &root);
    assert(list);
    assert(list->size == tree->total_nodes);

    /* The first 30 nodes are leaves of T. */
    for (i = 0; i < 30; i++) {
        node = (unsigned int *)(list->cell[i]);
        assert(1 == gnx_degree(tree, node));
        assert(gnx_delete_node(tree, node));
    }

    /* The next 12 nodes are leaves of T_1. */
    for (i = 30; i < 42; i++) {
        node = (unsigned int *)(list->cell[i]);
        assert(1 == gnx_degree(tree, node));
        assert(gnx_delete_node(tree, node));
    }

    /* The next 4 nodes are leaves of T_2. */
    for (i = 42; i < 46; i++) {
        node = (unsigned int *)(list->cell[i]);
        assert(1 == gnx_degree(tree, node));
        assert(gnx_delete_node(tree, node));
    }

    /* The next node is the leaf node 7. */
    i = 46;
    node = (unsigned int *)(list->cell[i]);
    assert(7 == *node);
    assert(1 == gnx_degree(tree, node));
    assert(gnx_delete_node(tree, node));

    /* The final node is the root node. */
    i = 47;
    node = (unsigned int *)(list->cell[i]);
    assert(root == *node);

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

/* Test the function gnx_bottom_up() under low-memory scenarios.
 */
static void
bottom_up_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *tree;
    int alloc_size;
    const unsigned int tail[1] = {0};
    const unsigned int head[1] = {1};
    const unsigned int root = 0;
    const unsigned int nnode = 2;
    const unsigned int nedge = 1;

    tree = gnx_new();
    add_edges(tree, tail, head, &nedge);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    /* Cannot allocate memory for the set of traversal nodes. */
    alloc_size = GNX_ALLOC_SET_SIZE
        + (2 * GNX_ALLOC_ARRAY_SIZE)
        + GNX_ALLOC_QUEUE_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_bottom_up(tree, &root));
    assert(ENOMEM == errno);

    /* Cannot allocate memory for the queue of nodes in BFS. */
    alloc_size += GNX_ALLOC_ARRAY_SIZE
        + (2 * GNX_ALLOC_DICT_SIZE)
        + GNX_ALLOC_HEAP_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_bottom_up(tree, &root));
    assert(ENOMEM == errno);

    gnx_destroy(tree);
    gnx_alloc_reset_limit();
#endif
}

/* Bottom-up traversal of a small tree.
 */
static void
bottom_up_small_tree(void)
{
    GnxArray *list;
    GnxGraph *tree;
    unsigned int i, *node;
    const unsigned int root = 42;
    const unsigned int tail[11] = {42, 42, 4, 4, 4, 15,  3,  3,  5,  5, 11};
    const unsigned int head[11] = { 4, 15, 2, 3, 5,  7, 10, 11, 12, 13, 14};
    const unsigned int size = 11;

    tree = gnx_new();
    add_edges(tree, tail, head, &size);
    assert(12 == tree->total_nodes);
    list = gnx_bottom_up(tree, &root);
    assert(list);
    assert(list->size == tree->total_nodes);

    /* The first 6 nodes are leaves of T. */
    for (i = 0; i < 6; i++) {
        node = (unsigned int *)(list->cell[i]);
        assert(1 == gnx_degree(tree, node));
        assert(gnx_delete_node(tree, node));
    }

    /* The next 3 node are leaves of T_1. */
    for (i = 6; i < 9; i++) {
        node = (unsigned int *)(list->cell[i]);
        assert(1 == gnx_degree(tree, node));
        assert(gnx_delete_node(tree, node));
    }

    /* The next node is the leaf node 3. */
    i = 9;
    node = (unsigned int *)(list->cell[i]);
    assert(3 == *node);
    assert(1 == gnx_degree(tree, node));
    assert(gnx_delete_node(tree, node));

    /* The next node is the leaf node 4. */
    i = 10;
    node = (unsigned int *)(list->cell[i]);
    assert(4 == *node);
    assert(1 == gnx_degree(tree, node));
    assert(gnx_delete_node(tree, node));

    /* The final node is the root node. */
    i = 11;
    node = (unsigned int *)(list->cell[i]);
    assert(root == *node);

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

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
static void
dfs_directed_weighted(void)
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
 * pre-order traversal
 *************************************************************************/

static void
pre_order(void)
{
    pre_order_flying_teams_bfs();
    pre_order_no_memory();
    pre_order_small_tree_unweighted();
    pre_order_small_tree_weighted();
    pre_order_traversal();
}

/* A pre-order traversal of a breadth-first search tree of the flying teams
 * network.
 */
static void
pre_order_flying_teams_bfs(void)
{
    GnxArray *list;
    GnxGraph *tree;
    unsigned int i, v;
    const unsigned int known[48] = {
        1, 2, 3,
        4, 14,
        15,
        5, 6, 7,
        27,
        18, 20, 22, 26,
        28,
        29, 30, 41,
        34, 46,
        8, 9,
        11, 12,
        10, 37,
        13, 31,
        16, 17,
        21, 40, 42,
        24, 23, 25,
        19,
        32, 36, 38, 39, 48,
        33, 35,
        47,
        43, 44,
        45};
    const unsigned int root = 1;
    const unsigned int nnode = 48;

    tree = gnx_read("data/visit/flying-teams-bfs.csv",
                    GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(nnode == tree->total_nodes);
    list = gnx_pre_order(tree, &root, GNX_SORTED_ORDER);
    assert(list);

    /* Compare with the known result. */
    for (i = 0; i < nnode; i++) {
        v = *((unsigned int *)(list->cell[i]));
        assert(known[i] == v);
    }

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

/* Test the function gnx_pre_order() under low-memory scenarios.
 */
static void
pre_order_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxGraph *tree;
    int alloc_size;
    const unsigned int v = 1;

    tree = gnx_new();
    assert(gnx_add_node(tree, &v));

    /* Cannot allocate memory to hold the nodes in pre-order. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_pre_order(tree, &v, GNX_DEFAULT_ORDER));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to hold the neighbors of a node. */
    alloc_size = GNX_ALLOC_ARRAY_SIZE
        + GNX_ALLOC_SET_SIZE
        + GNX_ALLOC_STACK_SIZE
        + 1
        + GNX_ALLOC_BUCKET_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_pre_order(tree, &v, GNX_SORTED_ORDER));
    assert(ENOMEM == errno);

    gnx_destroy(tree);
    gnx_alloc_reset_limit();
#endif
}

/* A pre-order traversal of a small tree.  The tree is unweighted.
 */
static void
pre_order_small_tree_unweighted(void)
{
    GnxArray *list;
    GnxGraph *tree;
    const unsigned int nedge = 4;
    const unsigned int nnode = 5;
    const unsigned int tail[4] = {0, 1, 1, 3};
    const unsigned int head[4] = {1, 2, 3, 4};
    const unsigned int knownA[5] = {0, 1, 2, 3, 4};
    const unsigned int knownB[5] = {0, 1, 3, 4, 2};
    const unsigned int root = 0;
    const unsigned int size = 5;

    tree = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(tree, tail, head, &nedge);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    /**********************************************************************
     * Traverse the neighbors in default order.
     *********************************************************************/

    list = gnx_pre_order(tree, &root, GNX_DEFAULT_ORDER);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    assert(compare_arrays(list, knownA, &size)
           || compare_arrays(list, knownB, &size));

    gnx_destroy_array(list);

    /**********************************************************************
     * Traverse the neighbors in sorted order.
     *********************************************************************/

    list = gnx_pre_order(tree, &root, GNX_SORTED_ORDER);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    assert(compare_arrays(list, knownA, &size));

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

/* A pre-order traversal of a small tree.  The tree is weighted.
 */
static void
pre_order_small_tree_weighted(void)
{
    GnxArray *list;
    GnxGraph *tree;
    const unsigned int nedge = 4;
    const unsigned int nnode = 5;
    const double weight[4]     = {0, 1, 2, 3};
    const unsigned int tail[4] = {0, 1, 1, 3};
    const unsigned int head[4] = {1, 2, 3, 4};
    const unsigned int knownA[5] = {0, 1, 2, 3, 4};
    const unsigned int knownB[5] = {0, 1, 3, 4, 2};
    const unsigned int root = 0;
    const unsigned int size = 5;

    tree = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(tree, tail, head, weight, &nedge);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    /**********************************************************************
     * Traverse the neighbors in default order.
     *********************************************************************/

    list = gnx_pre_order(tree, &root, GNX_DEFAULT_ORDER);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    assert(compare_arrays(list, knownA, &size)
           || compare_arrays(list, knownB, &size));

    gnx_destroy_array(list);

    /**********************************************************************
     * Traverse the neighbors in sorted order.
     *********************************************************************/

    list = gnx_pre_order(tree, &root, GNX_SORTED_ORDER);
    assert(gnx_is_tree(tree));
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    assert(compare_arrays(list, knownA, &size));

    gnx_destroy(tree);
    gnx_destroy_array(list);
}

/* A pre-order traversal of a tree.
 */
static void
pre_order_traversal(void)
{
    GnxArray *list;
    GnxGraph *tree;
    unsigned int i, v;
    const double weight[11]     = { 0,  1, 2, 3, 4,  5,  6,  7,  8,  9, 10};
    const unsigned int tail[11] = {42, 42, 4, 4, 4, 15,  3,  3,  5,  5, 11};
    const unsigned int head[11] = { 4, 15, 2, 3, 5,  7, 10, 11, 12, 13, 14};
    const unsigned int root = 42;
    const unsigned int known[12] = {42, 4, 2, 3, 10, 11, 14, 5, 12, 13, 15, 7};
    const unsigned int nnode = 12;
    const unsigned int nedge = 11;

    /**********************************************************************
     * Unweighted.
     *********************************************************************/

    tree = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    add_edges(tree, tail, head, &nedge);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    list = gnx_pre_order(tree, &root, GNX_SORTED_ORDER);
    assert(nnode == list->size);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    /* The known pre-order is given in the array 'known'. */
    for (i = 0; i < nnode; i++) {
        v = *((unsigned int *)(list->cell[i]));
        assert(known[i] == v);
        assert(gnx_has_node(tree, &v));
    }

    gnx_destroy(tree);
    gnx_destroy_array(list);

    /**********************************************************************
     * Weighted.
     *********************************************************************/

    tree = gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    add_edges_weighted(tree, tail, head, weight, &nedge);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    list = gnx_pre_order(tree, &root, GNX_SORTED_ORDER);
    assert(nnode == list->size);
    assert(nnode == tree->total_nodes);
    assert(nedge == tree->total_edges);

    /* The known pre-order is given in the array 'known'. */
    for (i = 0; i < nnode; i++) {
        v = *((unsigned int *)(list->cell[i]));
        assert(known[i] == v);
        assert(gnx_has_node(tree, &v));
    }

    gnx_destroy(tree);
    gnx_destroy_array(list);
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
    g_test_add_func("/visit/bottom-up", bottom_up);
    g_test_add_func("/visit/dfs", dfs);
    g_test_add_func("/visit/pre-order", pre_order);

    return g_test_run();
}
