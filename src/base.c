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

#include <glib.h>

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
static int gnx_i_directed_edge_iter_next(GnxEdgeIter *iter,
                                         unsigned int *u,
                                         unsigned int *v);
static int gnx_i_maybe_allocate_node(GnxGraph *graph,
                                     const unsigned int *v);
static inline int gnx_i_next_directed_edge(GnxEdgeIter *iter,
                                           unsigned int *w);
static void gnx_i_next_undirected_edge(GnxEdgeIter *iter,
                                       int *has_head,
                                       unsigned int *w);
static int gnx_i_undirected_edge_iter_next(GnxEdgeIter *iter,
                                           unsigned int *u,
                                           unsigned int *v);

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

    if (!gnx_i_maybe_allocate_node(graph, u))
        return GNX_FAILURE;
    if (!gnx_i_maybe_allocate_node(graph, v))
        return GNX_FAILURE;

    /* A digraph. */
    if (graph->directed) {
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
    g_assert(!graph->directed);

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

    if (!gnx_i_maybe_allocate_node(graph, u))
        return GNX_FAILURE;
    if (!gnx_i_maybe_allocate_node(graph, v))
        return GNX_FAILURE;

    weight = (double *)malloc(sizeof(double));
    if (!weight)
        return GNX_FAILURE;
    *weight = *w;

    /* A digraph. */
    if (graph->directed) {
        /* Add v to the dictionary of out-neighbors of u. */
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        g_assert(noded);
        x = gnx_set_has(graph->node, v);
        g_assert(x);
        if (!gnx_dict_add((GnxDict *)(noded->outneighbor), x, weight))
            goto cleanup;
        (noded->outdegree)++;

        /* Add u to the set of in-neighbors of v. */
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);
        x = gnx_set_has(graph->node, u);
        g_assert(x);
        if (!gnx_set_add((GnxSet *)(noded->inneighbor), x))
            return GNX_FAILURE;
        (noded->indegree)++;

        (graph->total_edges)++;

        return GNX_SUCCESS;
    }

    /* An undirected graph. */
    g_assert(!graph->directed);

    /* Add u to the dictionary of neighbors of v.  If (u,v) is a self-loop,
     * then we would add u to its collection of neighbors.
     */
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);
    x = gnx_set_has(graph->node, u);
    g_assert(x);
    if (!gnx_dict_add((GnxDict *)(nodeu->neighbor), x, weight))
        goto cleanup;
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
        g_assert(x);
        if (!gnx_dict_add((GnxDict *)(nodeu->neighbor), x, weight))
            goto cleanup;
        (nodeu->degree)++;
    }

    (graph->total_edges)++;

    return GNX_SUCCESS;

cleanup:
    free(weight);
    return GNX_FAILURE;
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

    if (graph->directed) {
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

    g_assert(!graph->directed);
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

    if (graph->directed) {
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

    g_assert(!graph->directed);
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
    unsigned int degree, w;

    /* Removing a node from a graph requires that we also delete every edge
     * that is incident on the node.  First, we delete the incident edges.
     * Finally, we delete the node itself.
     */
    degree = 0;

    if (graph->directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);

        /* Let w be an out-neighbor of v.  Then v is an in-neighbor of w.
         * Remove v from the collection of in-neighbors of w and then
         * decrement the in-degree of w.
         */
        neighbor = (GnxSet *)(noded->outneighbor);
        degree = neighbor->size;
        gnx_set_iter_init(&iter, neighbor);
        while (gnx_set_iter_next(&iter, &w)) {
            noded_w = (GnxNodeDirected *)(graph->graph[w]);
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
        while (gnx_set_iter_next(&iter, &w)) {
            noded_w = (GnxNodeDirected *)(graph->graph[w]);
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

    g_assert(!graph->directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);

    /* Let w be a neighbor of v.  Remove v from the collection of neighbors
     * of w and then decrement the degree of w.
     */
    neighbor = (GnxSet *)(nodeu->neighbor);
    degree = neighbor->size;
    gnx_set_iter_init(&iter, neighbor);
    while (gnx_set_iter_next(&iter, &w)) {
        /* Skip over any self-loop. */
        if (*v == w)
            continue;

        nodeu_w = (GnxNodeUndirected *)(graph->graph[w]);
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
    unsigned int degree, w;

    /* Removing a node from a graph requires that we also delete every edge
     * that is incident on the node.  First, we delete the incident edges.
     * Finally, we delete the node itself.
     */
    degree = 0;

    if (graph->directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);

        /* Let w be an out-neighbor of v.  Then v is an in-neighbor of w.
         * Remove v from the collection of in-neighbors of w and then
         * decrement the in-degree of w.
         */
        neighbor = (GnxDict *)(noded->outneighbor);
        degree = neighbor->size;
        gnx_dict_iter_init(&iter, neighbor);
        while (gnx_dict_iter_next(&iter, &w, NULL)) {
            noded_w = (GnxNodeDirected *)(graph->graph[w]);
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
        while (gnx_set_iter_next(&iters, &w)) {
            noded_w = (GnxNodeDirected *)(graph->graph[w]);
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

    g_assert(!graph->directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(nodeu);

    /* Let w be a neighbor of v.  Remove v from the collection of neighbors
     * of w and then decrement the degree of w.
     */
    neighbor = (GnxDict *)(nodeu->neighbor);
    degree = neighbor->size;
    gnx_dict_iter_init(&iter, neighbor);
    while (gnx_dict_iter_next(&iter, &w, NULL)) {
        /* Skip over any self-loop. */
        if (*v == w)
            continue;

        nodeu_w = (GnxNodeUndirected *)(graph->graph[w]);
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
 * @brief Retrieves the next directed edge.
 *
 * We advance the edge iterator by one step and retrieve the directed edge
 * at the current position.
 *
 * @param iter An edge iterator that has been initialized via the function
 *        gnx_edge_iter_init().
 * @param u Store here the ID of the current tail node.  If @c NULL, then we
 *        will not retrieve the ID of the current tail node.
 * @param v Store here the ID of the current head node.  If @c NULL, then we
 *        will not retrieve the ID of the current head node.
 * @return Nonzero if we have not yet exhausted all directed edges of the
 *         graph; zero otherwise.  If nonzero, then there is a directed edge
 *         that we have not visited.  If zero, then we have exhausted all
 *         directed edges and the iterator is now invalid.
 */
static int
gnx_i_directed_edge_iter_next(GnxEdgeIter *iter,
                              unsigned int *u,
                              unsigned int *v)
{
    GnxNodeDirected *node;
    unsigned int i, w;

    /* Are we bootstrapping the process? */
    if (iter->bootstrap) {
        iter->bootstrap = FALSE;

        /* The graph has zero edges. */
        if (!(iter->graph->total_edges))
            return GNX_FAILURE;

        /* Find the first node that has an out-neighbor. */
        for (i = 0; i < iter->graph->capacity; i++) {
            if (iter->graph->graph[i]) {
                if (gnx_outdegree(iter->graph, &i))
                    break;
            }
        }
        g_assert(i < iter->graph->capacity);
        iter->i = i;
        node = (GnxNodeDirected *)(iter->graph->graph[iter->i]);
        g_assert(node);

        /* Initialize an iterator over the out-neighbors of node i. */
        if (iter->weighted)
            gnx_dict_iter_init(&(iter->dict), (GnxDict *)(node->outneighbor));
        else
            gnx_set_iter_init(&(iter->set), (GnxSet *)(node->outneighbor));

        assert(gnx_i_next_directed_edge(iter, &w));

        if (u)
            *u = iter->i;
        if (v)
            *v = w;

        return GNX_SUCCESS;
    }

    /* We have successfully retrieved another directed edge. */
    if (gnx_i_next_directed_edge(iter, &w)) {
        if (u)
            *u = iter->i;
        if (v)
            *v = w;

        return GNX_SUCCESS;
    }

    /* We have exhausted all out-neighbors of node i.  Find the next node that
     * has at least one out-neighbor.
     */
    (iter->i)++;
    for (i = iter->i; i < iter->graph->capacity; i++) {
        if (iter->graph->graph[i]) {
            if (gnx_outdegree(iter->graph, &i))
                break;
        }
    }

    /* We have found another node that has an out-neighbor. */
    if (i < iter->graph->capacity) {
        iter->i = i;
        node = (GnxNodeDirected *)(iter->graph->graph[iter->i]);
        g_assert(node);

        /* Initialize an iterator over the out-neighbors of node i. */
        if (iter->weighted)
            gnx_dict_iter_init(&(iter->dict), (GnxDict *)(node->outneighbor));
        else
            gnx_set_iter_init(&(iter->set), (GnxSet *)(node->outneighbor));

        assert(gnx_i_next_directed_edge(iter, &w));

        if (u)
            *u = iter->i;
        if (v)
            *v = w;

        return GNX_SUCCESS;
    }

    /* All directed edges have been exhausted. */
    g_assert(i >= iter->graph->capacity);
    return GNX_FAILURE;
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

/**
 * @brief Retrieve a directed edge
 *
 * Find a directed edge that emanates from node i.
 *
 * @param iter An edge iterator that has been initialized via the function
 *        gnx_edge_iter_init().
 * @param w This will hold an out-neighbor of i.
 *
 * @return Nonzero if a directed edge is found; zero otherwise.
 */
static inline int
gnx_i_next_directed_edge(GnxEdgeIter *iter,
                         unsigned int *w)
{
    if (iter->weighted)
        return gnx_dict_iter_next(&(iter->dict), w, NULL);

    return gnx_set_iter_next(&(iter->set), w);
}

/**
 * @brief Retrieve an undirected edge.
 *
 * Find the first neighbor w of node i such that i <= w.
 *
 * @param iter An edge iterator that has been initialized via the function
 *        gnx_edge_iter_init().
 * @param has_head Whether we have found a neighbor w of i such that i <= w.
 * @param w This will hold a neighbor of i.
 */
static void
gnx_i_next_undirected_edge(GnxEdgeIter *iter,
                           int *has_head,
                           unsigned int *w)
{
    *has_head = FALSE;
    if (iter->weighted) {
        while (gnx_dict_iter_next(&(iter->dict), w, NULL)) {
            if (iter->i <= *w) {
                *has_head = TRUE;
                break;
            }
        }
    } else {
        while (gnx_set_iter_next(&(iter->set), w)) {
            if (iter->i <= *w) {
                *has_head = TRUE;
                break;
            }
        }
    }
}

/**
 * @brief Retrieves the next undirected edge.
 *
 * We advance the edge iterator by one step and retrieve the undirected edge
 * at the current position.
 *
 * @param iter An edge iterator that has been initialized via the function
 *        gnx_edge_iter_init().
 * @param u Store here the ID of the current tail node.  If @c NULL, then we
 *        will not retrieve the ID of the current tail node.
 * @param v Store here the ID of the current head node.  If @c NULL, then we
 *        will not retrieve the ID of the current head node.
 * @return Nonzero if we have not yet exhausted all undirected edges of the
 *         graph; zero otherwise.  If nonzero, then there is an undirected edge
 *         that we have not visited.  If zero, then we have exhausted all
 *         undirected edges and the iterator is now invalid.
 */
static int
gnx_i_undirected_edge_iter_next(GnxEdgeIter *iter,
                                unsigned int *u,
                                unsigned int *v)
{
    GnxNodeUndirected *node;
    int has_head;
    unsigned int i, w;

    /* Are we bootstrapping the process? */
    if (iter->bootstrap) {
        iter->bootstrap = FALSE;

        /* The graph has zero edges. */
        if (!(iter->graph->total_edges))
            return GNX_FAILURE;

        /* If (u,v) is an undirected edge, then we assume that u <= v.  Find
         * the first node u that has a neighbor and the first node v such that
         * (u,v) is an undirected edge in the graph and u <= v.
         */
        iter->i = UINT_MAX;
        for (;;) {
            /* Find the first node that has a neighbor. */
            for (i = (iter->i + 1); i < iter->graph->capacity; i++) {
                if (iter->graph->graph[i]) {
                    if (gnx_degree(iter->graph, &i))
                        break;
                }
            }
            g_assert(i < iter->graph->capacity);
            iter->i = i;
            node = (GnxNodeUndirected *)(iter->graph->graph[iter->i]);
            g_assert(node);

            /* Initialize an iterator over the neighbors of node i. */
            if (iter->weighted)
                gnx_dict_iter_init(&(iter->dict), (GnxDict *)(node->neighbor));
            else
                gnx_set_iter_init(&(iter->set), (GnxSet *)(node->neighbor));

            /* Retrieve an undirected edge.  Find the first neighbor w of i
             * such that i <= w.  If i does not have any such neighbor w, then
             * we find the next node that has a neighbor.
             */
            gnx_i_next_undirected_edge(iter, &has_head, &w);

            if (has_head)
                break;
        }

        if (u)
            *u = iter->i;
        if (v)
            *v = w;

        return GNX_SUCCESS;
    }

    /* We have iterated over an undirected edge of the graph.  Now step to the
     * next undirected edge.  Find a neighbor w of i such that i <= w.
     */
    gnx_i_next_undirected_edge(iter, &has_head, &w);

    /* We have successfully retrieved another undirected edge. */
    if (has_head) {
        if (u)
            *u = iter->i;
        if (v)
            *v = w;

        return GNX_SUCCESS;
    }

    /* We have exhausted all neighbors of node i.  If (u,v) is an undirected
     * edge, then we assume that u <= v.  Find the first node u that has a
     * neighbor and the first node v such that (u,v) is an undirected edge in
     * the graph and u <= v.
     */
    for (;;) {
        /* Find the first node that has a neighbor. */
        for (i = (iter->i + 1); i < iter->graph->capacity; i++) {
            if (iter->graph->graph[i]) {
                if (gnx_degree(iter->graph, &i))
                    break;
            }
        }

        /* None of the remaining nodes have a neighbor. */
        if (i >= iter->graph->capacity)
            break;

        g_assert(i < iter->graph->capacity);
        iter->i = i;
        node = (GnxNodeUndirected *)(iter->graph->graph[iter->i]);
        g_assert(node);

        /* Initialize an iterator over the neighbors of node i. */
        if (iter->weighted)
            gnx_dict_iter_init(&(iter->dict), (GnxDict *)(node->neighbor));
        else
            gnx_set_iter_init(&(iter->set), (GnxSet *)(node->neighbor));

        /* Retrieve an undirected edge.  Find the first neighbor w of i
         * such that i <= w.  If i does not have any such neighbor w, then
         * we find the next node that has a neighbor.
         */
        gnx_i_next_undirected_edge(iter, &has_head, &w);

        if (has_head) {
            if (u)
                *u = iter->i;
            if (v)
                *v = w;

            return GNX_SUCCESS;
        }
    }

    /* All undirected edges have been exhausted. */
    g_assert(i >= iter->graph->capacity);
    return GNX_FAILURE;
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
    int add_u = FALSE;  /* Assume that we have not added node u. */
    int add_v = FALSE;  /* Assume that we have not added node v. */

    errno = 0;
    g_return_val_if_fail(!gnx_is_weighted(graph), GNX_FAILURE);
    if (gnx_has_edge(graph, u, v))
        return GNX_FAILURE;
    if ((!graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

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
    if (add_u)
        assert(gnx_delete_node(graph, u));
    if (add_v)
        assert(gnx_delete_node(graph, v));
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
    int add_u = FALSE;  /* Assume that we have not added node u. */
    int add_v = FALSE;  /* Assume that we have not added node v. */

    errno = 0;
    g_return_val_if_fail(gnx_is_weighted(graph), GNX_FAILURE);
    g_return_val_if_fail(w, GNX_FAILURE);
    if (gnx_has_edge(graph, u, v))
        return GNX_FAILURE;
    if ((!graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

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
    if (add_u)
        assert(gnx_delete_node(graph, u));
    if (add_v)
        assert(gnx_delete_node(graph, v));
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

    return graph->selfloop;
}

/**
 * @brief The degree of a node in an undirected graph.
 *
 * The degree of a node @f$v@f$ is the number of neighbors that @f$v@f$ has.
 * If a graph is undirected, then gnx_degree() returns the degree of @f$v@f$ as
 * the number of nodes that are incident on @f$v@f$.
 *
 * @param graph The graph to query.  The graph is assumed to be undirected.
 * @param v Find the degree of this node in the given graph.  The node is
 *        assumed to be in the graph.
 * @return The degree of the node if the node is in the graph.
 */
unsigned int
gnx_degree(const GnxGraph *graph,
           const unsigned int *v)
{
    GnxNodeUndirected *node;

    g_return_val_if_fail(gnx_has_node(graph, v), GNX_FAILURE);
    g_return_val_if_fail(!graph->directed, GNX_FAILURE);

    node = (GnxNodeUndirected *)(graph->graph[*v]);
    g_assert(node);
    return node->degree;
}

/**
 * @brief Removes an edge from a graph.
 *
 * @param graph Update this graph.
 * @param u An end point of the edge to remove.
 * @param v The other end point of the edge to remove.
 * @return Nonzero if the edge @f$(u,v)@f$ was in the graph and is now removed;
 *         zero otherwise.  We also return zero if the graph is empty or the
 *         given edge is not in the graph.
 */
int
gnx_delete_edge(GnxGraph *graph,
                const unsigned int *u,
                const unsigned int *v)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    int weighted;

    if (!gnx_has_edge(graph, u, v))
        return GNX_FAILURE;

    weighted = GNX_WEIGHTED & graph->weighted;

    /* Weighted graph. */
    if (weighted) {
        if (graph->directed) {
            /* Remove v from the out-neighbors of u. */
            noded = (GnxNodeDirected *)(graph->graph[*u]);
            g_assert(noded);
            assert(gnx_dict_delete((GnxDict *)(noded->outneighbor), v));
            (noded->outdegree)--;

            /* Remove u from the in-neighbors of v. */
            noded = (GnxNodeDirected *)(graph->graph[*v]);
            g_assert(noded);
            assert(gnx_set_delete((GnxSet *)(noded->inneighbor), u));
            (noded->indegree)--;
        } else {
            /* Remove v from the neighbors of u. */
            nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
            g_assert(nodeu);
            assert(gnx_dict_delete((GnxDict *)(nodeu->neighbor), v));
            (nodeu->degree)--;

            /* Remove u from the neighbors of v.  If (u,v) is a self-loop, then
             * we skip this block.
             */
            if (*u != *v) {
                nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
                g_assert(nodeu);
                assert(gnx_dict_delete((GnxDict *)(nodeu->neighbor), u));
                (nodeu->degree)--;
            }
        }

        (graph->total_edges)--;
        return GNX_SUCCESS;
    }

    /* Unweighted graph. */
    g_assert(!weighted);
    if (graph->directed) {
        /* Remove v from the out-neighbors of u. */
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        g_assert(noded);
        assert(gnx_set_delete((GnxSet *)(noded->outneighbor), v));
        (noded->outdegree)--;

        /* Remove u from the in-neighbors of v. */
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        g_assert(noded);
        assert(gnx_set_delete((GnxSet *)(noded->inneighbor), u));
        (noded->indegree)--;
    } else {
        /* Remove v from the neighbors of u. */
        nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
        g_assert(nodeu);
        assert(gnx_set_delete((GnxSet *)(nodeu->neighbor), v));
        (nodeu->degree)--;

        /* Remove u from the neighbors of v.  If (u,v) is a self-loop, then
         * we skip this block.
         */
        if (*u != *v) {
            nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
            g_assert(nodeu);
            assert(gnx_set_delete((GnxSet *)(nodeu->neighbor), u));
            (nodeu->degree)--;
        }
    }

    (graph->total_edges)--;
    return GNX_SUCCESS;
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
    unsigned int i;

    if (!graph)
        return;
    gnx_destroy_set(graph->node);
    if (graph->graph) {
        if (GNX_WEIGHTED & graph->weighted) {
            /* A weighted graph. */
            if (graph->directed) {
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
            if (graph->directed) {
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
 * @brief Initializes an edge iterator.
 *
 * An edge iterator is used to iterate over the edges of a graph.  If you
 * modify the graph after calling the function gnx_edge_iter_init(), then the
 * iterator becomes invalid.
 *
 * @param iter An uninitialized edge iterator.  Note that an edge iterator is
 *        typically allocated on the runtime stack.
 * @param graph Iterate over the edges of this graph.
 */
void
gnx_edge_iter_init(GnxEdgeIter *iter,
                   GnxGraph *graph)
{
    g_return_if_fail(iter);
    gnx_i_check(graph);

    iter->bootstrap = TRUE;
    iter->directed = graph->directed;
    iter->weighted = GNX_WEIGHTED & graph->weighted;
    iter->graph = graph;
    iter->i = 0;
}

/**
 * @brief Retrieves the next edge.
 *
 * We advance the edge iterator by one step and retrieve the edge at the
 * current position.  If the graph is undirected, then the edges @f$(u,v)@f$
 * and @f$(v,u)@f$ are the same edge, hence we will iterate over @f$(u,v)@f$ at
 * most once and ignore @f$(v,u)@f$.  That is, we iterate over the unique edges
 * of an undirected graph.
 *
 * @param iter An edge iterator that has been initialized via the function
 *        gnx_edge_iter_init().
 * @param u This will store the ID of the current tail node.  If @c NULL, then
 *        we will not retrieve the ID of the current tail node.
 * @param v This will store the ID of the current head node.  If @c NULL, then
 *        we will not retrieve the ID of the current head node.
 * @return Nonzero if we have not yet exhausted all edges of the graph;
 *         zero otherwise.  If nonzero, then there is an edge that we have
 *         not visited.  If zero, then the iterator is now invalid.
 */
int
gnx_edge_iter_next(GnxEdgeIter *iter,
                   unsigned int *u,
                   unsigned int *v)
{
    g_return_val_if_fail(iter, GNX_FAILURE);

    if (iter->directed)
        return gnx_i_directed_edge_iter_next(iter, u, v);

    return gnx_i_undirected_edge_iter_next(iter, u, v);
}

/**
 * @brief Retrieves the weight of the given edge.
 *
 * @param graph The weighted graph to query.
 * @param u An end point of a weighted edge.
 * @param v The other end point of the weighted edge.  The edge @f$(u,v)@f$ is
 *        assumed to be in the graph.
 * @return The weight of the edge @f$(u,v)@f$.
 */
double
gnx_edge_weight(const GnxGraph *graph,
                const unsigned int *u,
                const unsigned int *v)
{
    double *weight;
    GnxDict *neighbor;
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;

    g_return_val_if_fail(gnx_has_edge(graph, u, v), GNX_FAILURE);
    g_return_val_if_fail(GNX_WEIGHTED & graph->weighted, GNX_FAILURE);

    /* Directed graph. */
    if (graph->directed) {
        noded = (GnxNodeDirected *)(graph->graph[*u]);
        g_assert(noded);
        neighbor = (GnxDict *)(noded->outneighbor);
        weight = (double *)gnx_dict_has(neighbor, v);
        g_assert(weight);
        return *weight;
    }

    /* Undirected graph. */
    g_assert(!graph->directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*u]);
    g_assert(nodeu);
    neighbor = (GnxDict *)(nodeu->neighbor);
    weight = (double *)gnx_dict_has(neighbor, v);
    g_assert(weight);
    return *weight;
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
    unsigned int a, b;

    if (!gnx_has_node(graph, u) || !gnx_has_node(graph, v))
        return GNX_FAILURE;
    if ((!graph->selfloop) && (*u == *v))
        return GNX_FAILURE;

    /* Weighted graphs. */
    if (GNX_WEIGHTED & graph->weighted) {
        if (graph->directed) {
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
    if (graph->directed) {
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

    if (graph->directed) {
        noded = (GnxNodeDirected *)(graph->graph[*v]);
        if (!noded)
            return GNX_FAILURE;

        return GNX_SUCCESS;
    }

    g_assert(!graph->directed);
    nodeu = (GnxNodeUndirected *)(graph->graph[*v]);
    if (!nodeu)
        return GNX_FAILURE;

    return GNX_SUCCESS;
}

/**
 * @brief The in-degree of a node.
 *
 * The degree of a node @f$v@f$ is the number of neighbors that @f$v@f$ has.
 * If a graph is directed, then gnx_indegree() returns the in-degree of
 * @f$v@f$ as the number of edges that have @f$v@f$ as their head end point.
 *
 * @param graph The graph to query.  We assume that this is a digraph.
 * @param v Find the in-degree of this node in the given graph.  The node is
 *        assumed to be in the graph.
 * @return The in-degree of the given node.
 */
unsigned int
gnx_indegree(const GnxGraph *graph,
             const unsigned int *v)
{
    GnxNodeDirected *node;

    g_return_val_if_fail(gnx_has_node(graph, v), GNX_FAILURE);
    g_return_val_if_fail(graph->directed, GNX_FAILURE);

    node = (GnxNodeDirected *)(graph->graph[*v]);
    g_assert(node);
    return node->indegree;
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

    return graph->directed;
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
 * @brief Initializes an iterator over the neighbors of a node.
 *
 * @param iter An uninitialized neighbor iterator.  Note that a neighbor
 *        iterator is typically allocated on the runtime stack.
 * @param graph We want to iterate over some nodes of this graph.
 * @param v Iterate over the neighbors of this node.  If the graph is
 *        undirected, then we will iterate over all the neighbors of v.  If the
 *        graph is directed, then we only iterate over the out-neighbors of v.
 * @return Nonzero if the initialization was successful; zero otherwise.
 */
void
gnx_neighbor_iter_init(GnxNeighborIter *iter,
                       GnxGraph *graph,
                       const unsigned int *v)
{
    g_return_if_fail(iter);
    g_return_if_fail(v);
    gnx_i_check(graph);

    iter->bootstrap = TRUE;
    iter->directed = graph->directed;
    iter->weighted = GNX_WEIGHTED & graph->weighted;
    iter->graph = graph;
    iter->v = *v;
}

/**
 * @brief Retrieves the next neighbor of a node.
 *
 * Note that if the graph is undirected, then we will iterate over all
 * neighbors of a node.  For a digraph, we only iterate over the out-neighbors
 * of a node.
 *
 * @param iter An iterator over the neighbors of node @f$v@f$.  This must have
 *        been initialized by the function gnx_neighbor_iter_init().
 * @param w This will store a neighbor of @f$v@f$.  Pass @c NULL if you want to
 *        ignore a neighbor.
 * @param weight If the graph is weighted, then this will store the weight of
 *        the edge @f$(v,w)@f$.  Pass in @c NULL if you want to ignore an edge
 *        weight.  If the graph is unweighted, then we will ignore this
 *        parameter.
 * @return Nonzero if we have not yet exhausted all neighbors of @f$v@f$;
 *         zero otherwise.  If nonzero, then there is a neighbor of @f$v@f$
 *         that we have not visited.  We return zero if the graph has zero
 *         nodes or the given node @f$v@f$ is not in the graph.  If zero, then
 *         the iterator is now invalid.
 */
int
gnx_neighbor_iter_next(GnxNeighborIter *iter,
                       unsigned int *w,
                       double *weight)
{
    GnxNodeDirected *noded;
    GnxNodeUndirected *nodeu;
    gnxptr value;
    int has_more;

    g_return_val_if_fail(iter, GNX_FAILURE);

    /* We are bootstrapping the process. */
    if (iter->bootstrap) {
        iter->bootstrap = FALSE;

        if (!(iter->graph->total_nodes))
            return GNX_FAILURE;
        if (!gnx_has_node(iter->graph, &(iter->v)))
            return GNX_FAILURE;

        /* Initialize an iterator over the neighbors of v. */
        if (iter->directed) {
            noded = (GnxNodeDirected *)(iter->graph->graph[iter->v]);
            g_assert(noded);

            if (iter->weighted) {
                gnx_dict_iter_init(&(iter->iterd),
                                   (GnxDict *)(noded->outneighbor));
            } else {
                gnx_set_iter_init(&(iter->iters),
                                  (GnxSet *)(noded->outneighbor));
            }
        } else {
            nodeu = (GnxNodeUndirected *)(iter->graph->graph[iter->v]);
            g_assert(nodeu);

            if (iter->weighted) {
                gnx_dict_iter_init(&(iter->iterd),
                                   (GnxDict *)(nodeu->neighbor));
            } else {
                gnx_set_iter_init(&(iter->iters),
                                  (GnxSet *)(nodeu->neighbor));
            }
        }
    }

    /* Retrieve a neighbor of v. */
    if (iter->weighted)
        has_more = gnx_dict_iter_next(&(iter->iterd), w, &value);
    else
        has_more = gnx_set_iter_next(&(iter->iters), w);

    if (has_more) {
        if (iter->weighted && weight)
            *weight = *((double *)value);

        return GNX_SUCCESS;
    }

    return GNX_FAILURE;
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

    graph->directed = GNX_DIRECTED & directed;
    graph->selfloop = GNX_SELFLOOP & selfloop;
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

/**
 * @brief Initializes the node iterator.
 *
 * A node iterator is used to iterate over the nodes of a graph.  If you modify
 * the graph after calling the function gnx_node_iter_init(), then the iterator
 * becomes invalid.
 *
 * @param iter An uninitialized node iterator.  Note that a node iterator is
 *        typically allocated on the runtime stack.
 * @param graph Iterate over the nodes of this graph.
 */
void
gnx_node_iter_init(GnxNodeIter *iter,
                   GnxGraph *graph)
{
    g_return_if_fail(iter);
    gnx_i_check(graph);

    iter->bootstrap = TRUE;
    iter->graph = graph;
    iter->i = 0;
}

/**
 * @brief Retrieves the next node ID.
 *
 * We advance the node iterator by one step and retrieve the node ID at the
 * current position.
 *
 * @param iter A node iterator that has been initialized via the function
 *        gnx_node_iter_init().
 * @param v Store the current node ID here.  If @c NULL, then we will ignore
 *        the current node ID.
 * @return Nonzero if we have not yet exhausted all nodes of the graph;
 *         zero otherwise.  If nonzero, then there is a node that we have
 *         not visited.  If zero, then the iterator is now invalid.
 */
int
gnx_node_iter_next(GnxNodeIter *iter,
                   unsigned int *v)
{
    g_return_val_if_fail(iter, GNX_FAILURE);

    /* We are bootstrapping the process.  Search for the first node in the
     * graph.
     */
    if (iter->bootstrap) {
        /* The graph has zero nodes. */
        if (!(iter->graph->total_nodes))
            return GNX_FAILURE;

        /* Search for the node with the lowest ID. */
        for (iter->i = 0; iter->i < iter->graph->capacity; (iter->i)++) {
            if (iter->graph->graph[iter->i]) {
                if (v)
                    *v = iter->i;

                iter->bootstrap = FALSE;
                (iter->i)++;
                return GNX_SUCCESS;
            }
        }
    }

    /* The graph has at least one node and we have reached at least one of
     * those nodes.  Now search for the next node in the graph.
     */
    while (iter->i < iter->graph->capacity) {
        if (iter->graph->graph[iter->i]) {
            if (v)
                *v = iter->i;

            (iter->i)++;
            return GNX_SUCCESS;
        }
        (iter->i)++;
    }

    /* We have exhausted all nodes of the graph. */
    return GNX_FAILURE;
}

/**
 * @brief The out-degree of a node in a digraph.
 *
 * The degree of a node @f$v@f$ is the number of neighbors that @f$v@f$ has.
 * If a graph is directed, then gnx_outdegree() returns the out-degree of
 * @f$v@f$ as the number of edges that have @f$v@f$ as their tail end point.
 *
 * @param graph The graph to query.  The graph is assumed to be directed.
 * @param v Find the out-degree of this node in the given graph.  The node is
 *        assumed to be in the graph.
 * @return The out-degree of the node if the node is in the graph.
 */
unsigned int
gnx_outdegree(const GnxGraph *graph,
              const unsigned int *v)
{
    GnxNodeDirected *node;

    g_return_val_if_fail(gnx_has_node(graph, v), GNX_FAILURE);
    g_return_val_if_fail(graph->directed, GNX_FAILURE);

    node = (GnxNodeDirected *)(graph->graph[*v]);
    g_assert(node);
    return node->outdegree;
}
