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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "base.h"
#include "dict.h"
#include "sanity.h"
#include "set.h"
#include "util.h"

/**
 * @file base.h
 * @brief Fundamental algorithms on graphs.
 *
 * To construct a graph, use either of the functions gnx_new() or
 * gnx_new_full().  To destroy a graph, use gnx_destroy().
 */

/**************************************************************************
 * internal data structures
 *************************************************************************/

#define GNX_MAX_EXPONENT (32)

/* @cond */
/* All powers of two 2^i for i = 0,...,31. */
static unsigned int gnx_power2[GNX_MAX_EXPONENT] = {
    1,            /* 2^0 */
    2,            /* 2^1 */
    4,            /* 2^2 */
    8,            /* 2^3 */
    16,           /* 2^4 */
    32,           /* 2^5 */
    64,           /* 2^6 */
    128,          /* 2^7 */
    256,          /* 2^8 */
    512,          /* 2^9 */
    1024,         /* 2^10 */
    2048,         /* 2^11 */
    4096,         /* 2^12 */
    8192,         /* 2^13 */
    16384,        /* 2^14 */
    32768,        /* 2^15 */
    65536,        /* 2^16 */
    131072,       /* 2^17 */
    262144,       /* 2^18 */
    524288,       /* 2^19 */
    1048576,      /* 2^20 */
    2097152,      /* 2^21 */
    4194304,      /* 2^22 */
    8388608,      /* 2^23 */
    16777216,     /* 2^24 */
    33554432,     /* 2^25 */
    67108864,     /* 2^26 */
    134217728,    /* 2^27 */
    268435456,    /* 2^28 */
    536870912,    /* 2^29 */
    1073741824,   /* 2^30 */
    2147483648};  /* 2^31 */
/* @endcond */

/* @cond */
/* A node in a digraph.  In a digraph, we keep track of all nodes that are
 * in-neighbors or out-neighbors of a given node.
 */
typedef struct {
    unsigned int indegree;   /* The in-degree of the node.  For digraphs, the
                              * in-degree of a node v counts all nodes that are
                              * in-neighbors of v.
                              */
    unsigned int outdegree;  /* The out-degree of the node.  For digraphs, the
                              * out-degree of a node v counts all nodes that
                              * are out-neighbors of v.
                              */
    gnxptr inneighbor;       /* The set of all in-neighbors of a node.  This
                              * is a set regardless of whether the graph is
                              * weighted or unweighted.
                              */
    gnxptr outneighbor;      /* The collection of all out-neighbors of a node.
                              * If the graph is unweighted, then this is a set.
                              * For a weighted graph, this is a dictionary.
                              */
} GnxNodeDirected;
/* @endcond */

/* @cond */
/* A node in an undirected graph.
 */
typedef struct {
    unsigned int degree;  /* The degree of the node.  For undirected graphs,
                           * the degree of a node v counts the nodes that are
                           * adjacent to v.
                           */
    gnxptr neighbor;      /* The collection of all nodes that are adjacent to
                           * a node v.  For undirected graphs, this is all the
                           * nodes that are neighbors of v.  If the graph is
                           * weighted, then this is a dictionary.  For an
                           * unweighted graph, this is a set.
                           */
} GnxNodeUndirected;
/* @endcond */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_add_edge_unweighted(GnxGraph *graph,
                                     const unsigned int *u,
                                     const unsigned int *v);
static int gnx_i_add_edge_weighted(GnxGraph *graph,
                                   const unsigned int *u,
                                   const unsigned int *v,
                                   const double *w);
static int gnx_i_add_node_unweighted(GnxGraph *graph,
                                     const unsigned int *v);
static int gnx_i_add_node_weighted(GnxGraph *graph,
                                   const unsigned int *v);
static void gnx_i_delete_node_unweighted(GnxGraph *graph,
                                         const unsigned int *v);
static void gnx_i_delete_node_weighted(GnxGraph *graph,
                                       const unsigned int *v);
static int gnx_i_maybe_allocate_node(GnxGraph *graph,
                                     const unsigned int *v);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Inserts an unweighted edge into a graph.
 *
 * Note that we do not support multiple edges.  For the edge @f$(u, v)@f$ to be
 * successfully inserted into the graph, the edge must not already be in the
 * graph.
 *
 * @param graph The unweighted graph to update.
 * @param u An end point of the edge.
 * @param v The other end point of the edge.
 * @return Nonzero if the edge @f$(u, v)@f$ did not yet exist in the graph and
 *         is successfully inserted into the graph; zero otherwise.
 */
static int
gnx_i_add_edge_unweighted(GnxGraph *graph,
                          const unsigned int *u,
                          const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    unsigned int *x;
    const int directed = GNX_DIRECTED & graph->directed;

    if (!gnx_i_maybe_allocate_node(graph, u))
        return GNX_FAILURE;
    if (!gnx_i_maybe_allocate_node(graph, v))
        return GNX_FAILURE;

    /* A digraph. */
    if (directed) {
        /* Add v to the set of out-neighbors of u. */
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        g_assert(noded);
        x = gnx_set_has(graph->node, v);
        if (!gnx_set_add((GnxSet *)(noded->outneighbor), x))
            return GNX_FAILURE;
        (noded->outdegree)++;

        /* Add u to the set of in-neighbors of v. */
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);
        x = gnx_set_has(graph->node, u);
        if (!gnx_set_add((GnxSet *)(noded->inneighbor), x))
            return GNX_FAILURE;
        (noded->indegree)++;

        (graph->total_edges)++;

        return GNX_SUCCESS;
    }

    /* An undirected graph. */
    g_assert(!directed);

    /* Add u to the collection of neighbors of v.  If (u,v) is a self-loop,
     * then we would add u to its collection of neighbors.
     */
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);
    x = gnx_set_has(graph->node, u);
    if (!gnx_set_add((GnxSet *)(nodeu->neighbor), x))
        return GNX_FAILURE;
    (nodeu->degree)++;

    /* Add v to the collection of neighbors of u.  If (u,v) is a self-loop,
     * then we would skip this block.
     */
    if (*u != *v) {
        nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
        g_assert(nodeu);
        x = gnx_set_has(graph->node, v);
        if (!gnx_set_add((GnxSet *)(nodeu->neighbor), x))
            return GNX_FAILURE;
        (nodeu->degree)++;
    }

    (graph->total_edges)++;

    return GNX_SUCCESS;
}

/**
 * @brief Inserts a weighted edge into a graph.
 *
 * Note that we do not support multiple edges.  For the edge @f$(u, v)@f$ to be
 * successfully inserted into the graph, the edge must not already be in the
 * graph.
 *
 * @param graph The weighted graph to update.
 * @param u An end point of the edge.
 * @param v The other end point of the edge.
 * @param w The weight of the edge @f$(u, v)@f$.
 * @return Nonzero if the edge @f$(u, v)@f$ did not yet exist in the graph and
 *         is successfully inserted into the graph; zero otherwise.
 */
static int
gnx_i_add_edge_weighted(GnxGraph *graph,
                        const unsigned int *u,
                        const unsigned int *v,
                        const double *w)
{
    double *weight;
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    unsigned int *x;
    const int directed = GNX_DIRECTED & graph->directed;

    if (!gnx_i_maybe_allocate_node(graph, u))
        return GNX_FAILURE;
    if (!gnx_i_maybe_allocate_node(graph, v))
        return GNX_FAILURE;

    weight = (double *)malloc(sizeof(double));
    if (!weight)
        return GNX_FAILURE;
    *weight = *w;

    /* A digraph. */
    if (directed) {
        /* Add v to the dictionary of out-neighbors of u. */
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        g_assert(noded);
        x = gnx_set_has(graph->node, v);
        if (!gnx_dict_add((GnxDict *)(noded->outneighbor), x, weight)) {
            free(weight);
            return GNX_FAILURE;
        }
        (noded->outdegree)++;

        /* Add u to the set of in-neighbors of v. */
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);
        x = gnx_set_has(graph->node, u);
        if (!gnx_set_add((GnxSet *)(noded->inneighbor), x))
            return GNX_FAILURE;
        (noded->indegree)++;

        (graph->total_edges)++;

        return GNX_SUCCESS;
    }

    /* An undirected graph. */
    g_assert(!directed);

    /* Add u to the dictionary of neighbors of v.  If (u,v) is a self-loop,
     * then we would add u to its collection of neighbors.
     */
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);
    x = gnx_set_has(graph->node, u);
    if (!gnx_dict_add((GnxDict *)(nodeu->neighbor), x, weight)) {
        free(weight);
        return GNX_FAILURE;
    }
    (nodeu->degree)++;

    /* Add v to the dictionary of neighbors of u.  If (u,v) is a self-loop,
     * then we would skip over this block.
     */
    if (*u != *v) {
        weight = (double *)malloc(sizeof(double));
        if (!weight)
            return GNX_FAILURE;
        *weight = *w;
        nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
        g_assert(nodeu);
        x = gnx_set_has(graph->node, v);
        if (!gnx_dict_add((GnxDict *)(nodeu->neighbor), x, weight)) {
            free(weight);
            return GNX_FAILURE;
        }
        (nodeu->degree)++;
    }

    (graph->total_edges)++;

    return GNX_SUCCESS;
}

/**
 * @brief Inserts a new node into an unweighted graph.
 *
 * @param graph The unweighted graph to update.
 * @param v A node to insert.  We assume that the number of nodes in the graph
 *        is less than #GNX_MAXIMUM_NODES.
 * @return Nonzero if we successfully inserted the node into the graph; zero
 *         otherwise.  If we are unable to allocate memory for the new node,
 *         then @c errno is set to @c ENOMEM and we return zero.
 */
static int
gnx_i_add_node_unweighted(GnxGraph *graph,
                          const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    GnxSet *adjacency, *adjacency_in;

    errno = 0;

    /* For an unweighted graph, the neighbors of a node are represented as a
     * set.
     */
    adjacency = gnx_init_set_full(GNX_DONT_FREE_ELEMENTS);
    if (!adjacency)
        goto cleanup;

    if (GNX_DIRECTED & graph->directed) {
        noded = (GnxNodeDirected *)malloc(sizeof(GnxNodeDirected));
        if (!noded)
            goto cleanup;

        adjacency_in = gnx_init_set_full(GNX_DONT_FREE_ELEMENTS);
        if (!adjacency_in) {
            free(noded);
            goto cleanup;
        }

        noded->indegree = 0;
        noded->outdegree = 0;
        noded->inneighbor = adjacency_in;
        noded->outneighbor = adjacency;
        graph->graph[*v] = noded;

        return GNX_SUCCESS;
    }

    nodeu = (GnxNodeUndirected *)malloc(sizeof(GnxNodeUndirected));
    if (!nodeu)
        goto cleanup;

    nodeu->degree = 0;
    nodeu->neighbor = adjacency;
    graph->graph[*v] = nodeu;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (adjacency)
        gnx_destroy_set(adjacency);
    return GNX_FAILURE;
}

/**
 * @brief Inserts a new node into a weighted graph.
 *
 * @param graph The weighted graph to update.
 * @param v A node to insert.  We assume that the number of nodes in the graph
 *        is less than #GNX_MAXIMUM_NODES.
 * @return Nonzero if we successfully inserted the node into the graph; zero
 *         otherwise.  If we are unable to allocate memory for the new node,
 *         then @c errno is set to @c ENOMEM and we return zero.
 */
static int
gnx_i_add_node_weighted(GnxGraph *graph,
                        const unsigned int *v)
{
    GnxDict *adjacency;
    GnxSet *adjacency_in;
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;

    errno = 0;

    /* For a weighted graph, the neighbors of a node v is represented as a
     * dictionary that has the following structure:
     *
     * v : {
     *     neighbor_1 : weight_1,
     *     ...
     *     neighbor_k : weight_k,
     * }
     *
     * Here, weight_i refers to the numeric weight of the edge
     * (v, neighbor_i).  The keys are the neighbor IDs and the values are
     * the corresponding edge weights.
     */
    adjacency = gnx_init_dict_full(GNX_DONT_FREE_KEYS, GNX_FREE_VALUES);
    if (!adjacency)
        goto cleanup;

    if (GNX_DIRECTED & graph->directed) {
        noded = (GnxNodeDirected *)malloc(sizeof(GnxNodeDirected));
        if (!noded)
            goto cleanup;

        adjacency_in = gnx_init_set_full(GNX_DONT_FREE_ELEMENTS);
        if (!adjacency_in) {
            free(noded);
            goto cleanup;
        }

        noded->indegree = 0;
        noded->outdegree = 0;
        noded->inneighbor = adjacency_in;
        noded->outneighbor = adjacency;
        graph->graph[*v] = noded;

        return GNX_SUCCESS;
    }

    nodeu = (GnxNodeUndirected *)malloc(sizeof(GnxNodeUndirected));
    if (!nodeu)
        goto cleanup;

    nodeu->degree = 0;
    nodeu->neighbor = adjacency;
    graph->graph[*v] = nodeu;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (adjacency)
        gnx_destroy_dict(adjacency);
    return GNX_FAILURE;
}

/**
 * @brief Removes a node from an unweighted graph.
 *
 * @param graph Update this unweighted graph.
 * @param v Remove this node from the graph.
 */
static void
gnx_i_delete_node_unweighted(GnxGraph *graph,
                             const unsigned int *v)
{
    GnxSet *neighbor, *neighbor_in;
    GnxSetIter iter;
    GnxNodeDirected *noded, *noded_w;
    GnxNodeUndirected *nodeu, *nodeu_w;
    gnxptr elem;
    unsigned int degree, *w;
    const int directed = GNX_DIRECTED & graph->directed;

    /* Removing a node from a graph requires that we also delete every edge
     * that is incident on the node.  First, we delete the incident edges.
     * Finally, we delete the node itself.
     */
    degree = 0;

    if (directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);

        /* Let w be an out-neighbor of v.  Then v is an in-neighbor of w.
         * Remove v from the collection of in-neighbors of w and then
         * decrement the in-degree of w.
         */
        neighbor = (GnxSet *)(noded->outneighbor);
        degree = neighbor->size;
        gnx_set_iter_init(&iter, neighbor);
        while (gnx_set_iter_next(&iter, &elem)) {
            w = (unsigned int *)elem;
            noded_w = (GnxNodeDirected *)(graph->graph[*w]);
            g_assert(noded_w);
            assert(gnx_set_delete((GnxSet *)(noded_w->inneighbor), v));
            (noded_w->indegree)--;
        }

        /* Let w be an in-neighbor of v.  Then v is an out-neighbor of w.
         * Remove v from the collection of out-neighbors of w and then
         * decrement the out-degree of w.
         */
        neighbor_in = (GnxSet *)(noded->inneighbor);
        degree += neighbor_in->size;
        gnx_set_iter_init(&iter, neighbor_in);
        while (gnx_set_iter_next(&iter, &elem)) {
            w = (unsigned int *)elem;
            noded_w = (GnxNodeDirected *)(graph->graph[*w]);
            g_assert(noded_w);
            assert(gnx_set_delete((GnxSet *)(noded_w->outneighbor), v));
            (noded_w->outdegree)--;
        }

        /* Delete all neighbors of v and then remove v itself. */
        gnx_destroy_set((GnxSet *)(noded->inneighbor));
        gnx_destroy_set((GnxSet *)(noded->outneighbor));
        free(noded);
        graph->graph[*v] = NULL;
        (graph->total_nodes)--;
        graph->total_edges -= degree;

        return;
    }

    g_assert(!directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);

    /* Let w be a neighbor of v.  Remove v from the collection of neighbors
     * of w and then decrement the degree of w.
     */
    neighbor = (GnxSet *)(nodeu->neighbor);
    degree = neighbor->size;
    gnx_set_iter_init(&iter, neighbor);
    while (gnx_set_iter_next(&iter, &elem)) {
        w = (unsigned int *)elem;

        /* Skip over any self-loop. */
        if (*v == *w)
            continue;

        nodeu_w = (GnxNodeUndirected *)(graph->graph[*w]);
        g_assert(nodeu_w);
        assert(gnx_set_delete((GnxSet *)(nodeu_w->neighbor), v));
        (nodeu_w->degree)--;
    }

    /* Delete all neighbors of v and then remove v itself. */
    gnx_destroy_set((GnxSet *)(nodeu->neighbor));
    free(nodeu);
    graph->graph[*v] = NULL;
    (graph->total_nodes)--;
    graph->total_edges -= degree;
}

/**
 * @brief Removes a node from a weighted graph.
 *
 * @param graph Update this weighted graph.
 * @param v Remove this node from the graph.
 */
static void
gnx_i_delete_node_weighted(GnxGraph *graph,
                           const unsigned int *v)
{
    GnxDict *neighbor;
    GnxDictIter iter;
    GnxNodeDirected *noded, *noded_w;
    GnxNodeUndirected *nodeu, *nodeu_w;
    GnxSet *neighbor_in;
    GnxSetIter iters;
    gnxptr key;
    unsigned int degree, *w;
    const int directed = GNX_DIRECTED & graph->directed;

    /* Removing a node from a graph requires that we also delete every edge
     * that is incident on the node.  First, we delete the incident edges.
     * Finally, we delete the node itself.
     */
    degree = 0;

    if (directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);

        /* Let w be an out-neighbor of v.  Then v is an in-neighbor of w.
         * Remove v from the collection of in-neighbors of w and then
         * decrement the in-degree of w.
         */
        neighbor = (GnxDict *)(noded->outneighbor);
        degree = neighbor->size;
        gnx_dict_iter_init(&iter, neighbor);
        while (gnx_dict_iter_next(&iter, &key, NULL)) {
            w = (unsigned int *)key;
            noded_w = (GnxNodeDirected *)(graph->graph[*w]);
            g_assert(noded_w);
            assert(gnx_set_delete((GnxSet *)(noded_w->inneighbor), v));
            (noded_w->indegree)--;
        }

        /* Let w be an in-neighbor of v.  Then v is an out-neighbor of w.
         * Remove v from the collection of out-neighbors of w and then
         * decrement the out-degree of w.
         */
        neighbor_in = (GnxSet *)(noded->inneighbor);
        degree += neighbor_in->size;
        gnx_set_iter_init(&iters, neighbor_in);
        while (gnx_set_iter_next(&iters, &key)) {
            w = (unsigned int *)key;
            noded_w = (GnxNodeDirected *)(graph->graph[*w]);
            g_assert(noded_w);
            assert(gnx_dict_delete((GnxDict *)(noded_w->outneighbor), v));
            (noded_w->outdegree)--;
        }

        /* Delete all neighbors of v and then remove v itself. */
        gnx_destroy_set((GnxSet *)(noded->inneighbor));
        gnx_destroy_dict((GnxDict *)(noded->outneighbor));
        free(noded);
        graph->graph[*v] = NULL;
        (graph->total_nodes)--;
        graph->total_edges -= degree;

        return;
    }

    g_assert(!directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);

    /* Let w be a neighbor of v.  Remove v from the collection of neighbors
     * of w and then decrement the degree of w.
     */
    neighbor = (GnxDict *)(nodeu->neighbor);
    degree = neighbor->size;
    gnx_dict_iter_init(&iter, neighbor);
    while (gnx_dict_iter_next(&iter, &key, NULL)) {
        w = (unsigned int *)key;

        /* Skip over any self-loop. */
        if (*v == *w)
            continue;

        nodeu_w = (GnxNodeUndirected *)(graph->graph[*w]);
        g_assert(nodeu_w);
        assert(gnx_dict_delete((GnxDict *)(nodeu_w->neighbor), v));
        (nodeu_w->degree)--;
    }

    /* Delete all neighbors of v and then remove v itself. */
    gnx_destroy_dict((GnxDict *)(nodeu->neighbor));
    free(nodeu);
    graph->graph[*v] = NULL;
    (graph->total_nodes)--;
    graph->total_edges -= degree;
}

/**
 * @brief Maybe allocate memory for an edge node.
 *
 * @param graph The graph to update.
 * @param v Whether we want to allocate memory for this edge node.
 * @return Nonzero if the memory allocation (if necessary) was successful;
 *         zero otherwise.
 */
static int
gnx_i_maybe_allocate_node(GnxGraph *graph,
                          const unsigned int *v)
{
    unsigned int *x;

    if (gnx_set_has(graph->node, v))
        return GNX_SUCCESS;

    x = (unsigned int *)malloc(sizeof(unsigned int));
    if (!x)
        return GNX_FAILURE;

    *x = *v;
    if (!gnx_set_add(graph->node, x)) {
        free(x);
        return GNX_FAILURE;
    }

    return GNX_SUCCESS;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Inserts an unweighted edge into a graph.
 *
 * Note that we do not support multiple edges.  For the edge @f$(u, v)@f$ to be
 * successfully inserted into the graph, the edge must not already be in the
 * graph.
 *
 * @param graph The graph to update.  The graph must be unweighted.
 * @param u An end point of the edge.
 * @param v The other end point of the edge.
 * @return Nonzero if the edge @f$(u, v)@f$ did not yet exist in the graph and
 *         is successfully inserted into the graph; zero otherwise.  We also
 *         return zero if the given edge is already in the graph.  If we are
 *         unable to allocate memory for the edge, then we set @c errno to
 *         @c ENOMEM and return zero.
 */
int
gnx_add_edge(GnxGraph *graph,
             const unsigned int *u,
             const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    int add_u = FALSE;  /* Assume that we have not added node u. */
    int add_v = FALSE;  /* Assume that we have not added node v. */
    int directed;

    errno = 0;
    g_return_val_if_fail(!gnx_is_weighted(graph), GNX_FAILURE);
    if (gnx_has_edge(graph, u, v))
        return GNX_FAILURE;
    if ((GNX_NO_SELFLOOP & graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

    directed = GNX_DIRECTED & graph->directed;

    /* Add the nodes to the graph as appropriate. */
    if (!gnx_has_node(graph, u)) {
        if (!gnx_add_node(graph, u))
            goto cleanup;

        add_u = TRUE;
    }
    if (!gnx_has_node(graph, v)) {
        if (!gnx_add_node(graph, v))
            goto cleanup;

        add_v = TRUE;
    }

    if (!gnx_i_add_edge_unweighted(graph, u, v))
        goto cleanup;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (add_u) {
        if (directed) {
            noded = (GnxNodeDirected *)(graph->graph[*u]);
            gnx_destroy_set((GnxSet *)(noded->inneighbor));
            gnx_destroy_set((GnxSet *)(noded->outneighbor));
            free(noded);
        } else {
            nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
            gnx_destroy_set((GnxSet *)(nodeu->neighbor));
            free(nodeu);
        }
        graph->graph[*u] = NULL;
        (graph->total_nodes)--;
    }
    if (add_v) {
        if (directed) {
            noded = (GnxNodeDirected *)(graph->graph[*v]);
            gnx_destroy_set((GnxSet *)(noded->inneighbor));
            gnx_destroy_set((GnxSet *)(noded->outneighbor));
            free(noded);
        } else {
            nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
            gnx_destroy_set((GnxSet *)(nodeu->neighbor));
            free(nodeu);
        }
        graph->graph[*v] = NULL;
        (graph->total_nodes)--;
    }
    return GNX_FAILURE;
}

/**
 * @brief Inserts a weighted edge into a graph.
 *
 * Note that we do not support multiple edges.  For the edge @f$(u, v)@f$ to be
 * successfully inserted into the graph, the edge must not already be in the
 * graph.
 *
 * @param graph The weighted graph to update.
 * @param u An end point of the edge.
 * @param v The other end point of the edge.
 * @param w The weight of the edge @f$(u, v)@f$.
 * @return Nonzero if the edge @f$(u, v)@f$ did not yet exist in the graph and
 *         is successfully inserted into the graph; zero otherwise.  We also
 *         return zero if the given edge is already in the graph.  If we are
 *         unable to allocate memory for the edge, then we set @c errno to
 *         @c ENOMEM and return zero.
 */
int
gnx_add_edgew(GnxGraph *graph,
              const unsigned int *u,
              const unsigned int *v,
              const double *w)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    int add_u = FALSE;  /* Assume that we have not added node u. */
    int add_v = FALSE;  /* Assume that we have not added node v. */
    int directed;

    errno = 0;
    g_return_val_if_fail(gnx_is_weighted(graph), GNX_FAILURE);
    g_return_val_if_fail(w, GNX_FAILURE);
    if (gnx_has_edge(graph, u, v))
        return GNX_FAILURE;
    if ((GNX_NO_SELFLOOP & graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

    directed = GNX_DIRECTED & graph->directed;

    /* Add the nodes to the graph as appropriate. */
    if (!gnx_has_node(graph, u)) {
        if (!gnx_add_node(graph, u))
            goto cleanup;

        add_u = TRUE;
    }
    if (!gnx_has_node(graph, v)) {
        if (!gnx_add_node(graph, v))
            goto cleanup;

        add_v = TRUE;
    }

    if (!gnx_i_add_edge_weighted(graph, u, v, w))
        goto cleanup;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (add_u) {
        if (directed) {
            noded = (GnxNodeDirected *)(graph->graph[*u]);
            gnx_destroy_set((GnxSet *)(noded->inneighbor));
            gnx_destroy_dict((GnxDict *)(noded->outneighbor));
            free(noded);
        } else {
            nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
            gnx_destroy_dict((GnxDict *)(nodeu->neighbor));
            free(nodeu);
        }
        graph->graph[*u] = NULL;
        (graph->total_nodes)--;
    }
    if (add_v) {
        if (directed) {
            noded = (GnxNodeDirected *)(graph->graph[*v]);
            gnx_destroy_set((GnxSet *)(noded->inneighbor));
            gnx_destroy_dict((GnxDict *)(noded->outneighbor));
            free(noded);
        } else {
            nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
            gnx_destroy_dict((GnxDict *)(nodeu->neighbor));
            free(nodeu);
        }
        graph->graph[*v] = NULL;
        (graph->total_nodes)--;
    }
    return GNX_FAILURE;
}

/**
 * @brief Inserts a new node into a graph.
 *
 * @param graph The graph to update.
 * @param v A node to insert.  We assume that the number of nodes in the graph
 *        is less than #GNX_MAXIMUM_NODES.
 * @return Nonzero if the node did not yet exist and we have successfully
 *         inserted the node into the graph; zero otherwise.  We also return
 *         zero if the given node is already in the graph.  If we are unable
 *         to allocate memory for the new node, then @c errno is set to
 *         @c ENOMEM and we return zero.
 */
int
gnx_add_node(GnxGraph *graph,
             const unsigned int *v)
{
    gnxptr *new_graph;
    unsigned int i, new_capacity;

    errno = 0;
    if (gnx_has_node(graph, v))
        return GNX_FAILURE;
    g_return_val_if_fail(graph->total_nodes < GNX_MAXIMUM_NODES, GNX_FAILURE);

    /* Possibly resize the array of adjacency lists.  There are two conditions
     * that each could trigger a resize.
     *
     * (1) The current number of nodes in the graph is at least the capacity of
     *     the graph.  This is the case if each value of graph->graph[i] is not
     *     NULL.
     * (2) The given node ID is greater than the maximum index of the array of
     *     adjacency lists.
     *
     * Assume that the node v to insert is not in the graph.  Let k be the
     * maximum index of the array of adjacency lists.  If case (1), then we
     * have v > k.  If case (2), then we clearly have v > k as well.  In either
     * case, we only need to consider the smallest power of two such that
     * v < 2^i for some non-negative integer i.
     */
    if ((graph->total_nodes >= graph->capacity) || (*v >= graph->capacity)) {
        g_assert(*v >= graph->capacity);

        /* Compute the new capacity of the graph.  This is done by finding the
         * smallest power of 2 such that v < 2^i for some non-negative integer
         * i.
         */
        i = GNX_DEFAULT_EXPONENT;
        while (*v >= gnx_power2[i])
            i++;

        g_assert(i < GNX_MAX_EXPONENT);
        new_capacity = gnx_power2[i];
        g_assert(new_capacity <= GNX_MAXIMUM_NODES);

        new_graph
            = (gnxptr *)realloc(graph->graph, new_capacity * sizeof(gnxptr));
        if (!new_graph)
            goto cleanup;
        /* Zero out the new memory. */
        for (i = graph->capacity; i < new_capacity; i++)
            new_graph[i] = NULL;

        graph->capacity = new_capacity;
        graph->graph = new_graph;
    }

    /* Add the node to the graph. */
    if (GNX_WEIGHTED & graph->weighted) {
        if (!gnx_i_add_node_weighted(graph, v))
            goto cleanup;
    } else {
        if (!gnx_i_add_node_unweighted(graph, v))
            goto cleanup;
    }

    (graph->total_nodes)++;
    g_assert(graph->total_nodes <= GNX_MAXIMUM_NODES);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    return GNX_FAILURE;
}

/**
 * @brief Whether self-loops are allowed in a graph.
 *
 * @param graph The graph to query.
 * @return Nonzero if the graph allows for self-loops; zero otherwise.
 */
int
gnx_allows_selfloop(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->selfloop & GNX_SELFLOOP;
}

/**
 * @brief Removes a node from a graph.
 *
 * @param graph We want to update this graph.
 * @param v Remove this node from the graph.
 * @return Nonzero if the node was in the graph and is now successfully
 *         removed from the graph; zero otherwise.  We also return zero if the
 *         graph is empty (the graph has zero nodes) or the node is not in the
 *         graph.
 */
int
gnx_delete_node(GnxGraph *graph,
                const unsigned int *v)
{
    if (!gnx_has_node(graph, v))
        return GNX_FAILURE;

    if (GNX_WEIGHTED & graph->weighted)
        gnx_i_delete_node_weighted(graph, v);
    else
        gnx_i_delete_node_unweighted(graph, v);

    return GNX_SUCCESS;
}

/**
 * @brief Destroys a graph.
 *
 * @param graph The graph to destroy.
 */
void
gnx_destroy(GnxGraph *graph)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    int directed;
    unsigned int i;

    if (!graph)
        return;
    gnx_destroy_set(graph->node);
    if (graph->graph) {
        directed = GNX_DIRECTED & graph->directed;

        if (GNX_WEIGHTED & graph->weighted) {
            /* A weighted graph. */
            if (directed) {
                for (i = 0; i < graph->capacity; i++) {
                    noded = (GnxNodeDirected *)(graph->graph[i]);
                    if (noded) {
                        gnx_destroy_set((GnxSet *)(noded->inneighbor));
                        gnx_destroy_dict((GnxDict *)(noded->outneighbor));
                        free(noded);
                        graph->graph[i] = NULL;
                    }
                }
            } else {
                for (i = 0; i < graph->capacity; i++) {
                    nodeu = (GnxNodeUndirected *)(graph->graph[i]);
                    if (nodeu) {
                        gnx_destroy_dict((GnxDict *)(nodeu->neighbor));
                        free(nodeu);
                        graph->graph[i] = NULL;
                    }
                }
            }
        } else {
            /* An unweighted graph. */
            if (directed) {
                for (i = 0; i < graph->capacity; i++) {
                    noded = (GnxNodeDirected *)(graph->graph[i]);
                    if (noded) {
                        gnx_destroy_set((GnxSet *)(noded->inneighbor));
                        gnx_destroy_set((GnxSet *)(noded->outneighbor));
                        free(noded);
                        graph->graph[i] = NULL;
                    }
                }
            } else {
                for (i = 0; i < graph->capacity; i++) {
                    nodeu = (GnxNodeUndirected *)(graph->graph[i]);
                    if (nodeu) {
                        gnx_destroy_set((GnxSet *)(nodeu->neighbor));
                        free(nodeu);
                        graph->graph[i] = NULL;
                    }
                }
            }
        }

        free(graph->graph);
        graph->graph = NULL;
    }
    free(graph);
    graph = NULL;
}

/**
 * @brief Whether a graph contains a given edge.
 *
 * For a digraph, the directed edge @f$(u,v)@f$ means that the edge goes from
 * @f$u@f$ to @f$v@f$.  The tail of the edge is @f$u@f$ and the head of the
 * edge is @f$v@f$.
 *
 * @param graph The graph to query.
 * @param u An end point of an edge.
 * @param v The other end point of an edge.
 * @return Nonzero if the graph contains the edge @f$(u, v)@f$; zero otherwise.
 *         We also return zero if the graph is empty.
 */
int
gnx_has_edge(const GnxGraph *graph,
             const unsigned int *u,
             const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    int directed;
    unsigned int a, b;

    if (!gnx_has_node(graph, u) || !gnx_has_node(graph, v))
        return GNX_FAILURE;
    if ((GNX_NO_SELFLOOP & graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

    directed = GNX_DIRECTED & graph->directed;

    /* Weighted graphs. */
    if (GNX_WEIGHTED & graph->weighted) {
        if (directed) {
            noded = (GnxNodeDirected *)(graph->graph[*u]);
            if (gnx_dict_has((GnxDict *)(noded->outneighbor), v))
                return GNX_SUCCESS;

            return GNX_FAILURE;
        }

        gnx_undirected_edge_order(u, v, &a, &b);
        nodeu = (GnxNodeUndirected *)(graph->graph[a]);
        if (gnx_dict_has((GnxDict *)(nodeu->neighbor), &b))
            return GNX_SUCCESS;

        return GNX_FAILURE;
    }

    /* Unweighted graphs. */
    g_assert(GNX_UNWEIGHTED & graph->weighted);
    if (directed) {
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        if (gnx_set_has((GnxSet *)(noded->outneighbor), v))
            return GNX_SUCCESS;

        return GNX_FAILURE;
    }

    gnx_undirected_edge_order(u, v, &a, &b);
    nodeu = (GnxNodeUndirected *)(graph->graph[a]);
    if (gnx_set_has((GnxSet *)(nodeu->neighbor), &b))
        return GNX_SUCCESS;

    return GNX_FAILURE;
}

/**
 * @brief Whether a graph contains a node.
 *
 * @param graph The graph to query.
 * @param v A node to query.
 * @return Nonzero if the node is in the graph; zero otherwise.  We also return
 *         zero if the graph is empty.
 */
int
gnx_has_node(const GnxGraph *graph,
             const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;

    gnx_i_check(graph);
    gnx_i_check_node(v);

    if (!graph->total_nodes)
        return GNX_FAILURE;
    if (*v >= graph->capacity)
        return GNX_FAILURE;

    if (GNX_DIRECTED & graph->directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        if (!noded)
            return GNX_FAILURE;

        return GNX_SUCCESS;
    }

    g_assert(GNX_UNDIRECTED & graph->directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    if (!nodeu)
        return GNX_FAILURE;

    return GNX_SUCCESS;
}

/**
 * @brief Whether a graph is directed.
 *
 * @param graph The graph to test for directedness.
 * @return Nonzero if the graph is directed; zero otherwise.
 */
int
gnx_is_directed(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->directed & GNX_DIRECTED;
}

/**
 * @brief Whether a graph is weighted.
 *
 * @param graph The graph to test for weightedness.
 * @return Nonzero if the graph is weighted; zero otherwise.
 */
int
gnx_is_weighted(const GnxGraph *graph)
{
    gnx_i_check(graph);

    return graph->weighted & GNX_WEIGHTED;
}

/**
 * @brief Initializes a new graph with default properties.
 *
 * This is a convenience function for when you require a graph that is
 * undirected, unweighted, and without self-loops.  The graph should be
 * destroyed with gnx_destroy().  If you want to set the properties of a
 * new graph, use the function gnx_new_full() instead.
 *
 * @sa gnx_new_full() to set the properties of a new graph.
 *
 * @return See the return value of gnx_new_full().
 */
GnxGraph*
gnx_new(void)
{
    return gnx_new_full(GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
}

/**
 * @brief Initializes a new graph with some basic properties.
 *
 * A graph should be destroyed with gnx_destroy().  The function gnx_new_full()
 * allows you to set some basic properties of a new graph.  If you want a graph
 * that is undirected, unweighted, and without self-loops you can instead use
 * the convenience function gnx_new().
 *
 * @sa gnx_new() A convenience function to initialize a graph that is
 *     undirected, unweighted, and without self-loops.
 *
 * @param directed Whether to create a directed graph.  The possible values
 *        are: #GNX_UNDIRECTED or #GNX_DIRECTED.
 * @param selfloop Whether to allow self-loops in a graph.  The possible values
 *        are: #GNX_NO_SELFLOOP or #GNX_SELFLOOP.
 * @param weighted Whether to create a weighted graph.  The possible values
 *        are: #GNX_UNWEIGHTED or #GNX_WEIGHTED.
 * @return An initialized graph.  If we cannot allocate memory for a new graph,
 *         then we set @c errno to @c ENOMEM and return @c NULL.
 */
GnxGraph*
gnx_new_full(const GnxBool directed,
             const GnxBool selfloop,
             const GnxBool weighted)
{
    GnxGraph *graph;
    const unsigned int reserved_nodes = GNX_DEFAULT_ALLOC_SIZE;

    errno = 0;
    gnx_i_check_properties(directed, selfloop, weighted);

    graph = (GnxGraph *)malloc(sizeof(GnxGraph));
    if (!graph)
        goto cleanup;

    graph->node = NULL;
    graph->graph = NULL;

    /* A collection of nodes of the graph.  This collection does not
     * necessarily contain all the nodes of the graph.  Its purpose is to help
     * us determine whether to allocate memory for a node.
     */
    graph->node = gnx_init_set_full(GNX_FREE_ELEMENTS);
    if (!graph->node)
        goto cleanup;

    /* By default, we allocate enough memory for a graph with a specified
     * number of nodes.  As more nodes are added to the graph, we might need
     * to resize this array.
     */
    graph->graph = (gnxptr *)calloc(reserved_nodes, sizeof(gnxptr));
    if (!graph->graph)
        goto cleanup;

    graph->directed = directed;
    graph->selfloop = selfloop;
    graph->weighted = weighted;
    graph->capacity = reserved_nodes;
    graph->total_edges = 0;
    graph->total_nodes = 0;

    return graph;

cleanup:
    errno = ENOMEM;
    gnx_destroy(graph);
    return NULL;
}
