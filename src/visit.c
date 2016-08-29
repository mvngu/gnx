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

#include "array.h"
#include "base.h"
#include "dict.h"
#include "query.h"
#include "queue.h"
#include "sanity.h"
#include "set.h"
#include "stack.h"
#include "util.h"
#include "visit.h"

/**
 * @file visit.h
 * @brief Traversing a graph.
 */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_bfs(GnxGraph *graph,
                     GnxGraph *g,
                     GnxSet *seen,
                     GnxQueue *queue);
static int gnx_i_default_order(GnxGraph *graph,
                               unsigned int *v,
                               GnxStack *stack);
static int gnx_i_dfs(GnxGraph *graph,
                     GnxGraph *g,
                     unsigned int *start,
                     GnxSet *seen,
                     GnxStack *stack);
static int gnx_i_dfs_push_onto_stack(GnxGraph *graph,
                                     unsigned int *u,
                                     GnxStack *stack,
                                     GnxDict *parent);
static int gnx_i_sorted_order(GnxGraph *graph,
                              unsigned int *v,
                              GnxStack *stack);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Traverses a graph via the strategy of breadth-first search.
 *
 * @param graph Traverse this graph.
 * @param g The search tree will be stored here.
 * @param seen A set of nodes that we have visited.
 * @param queue A queue of nodes to visit.
 * @return Nonzero if the traversal was successful; zero otherwise.
 */
static int
gnx_i_bfs(GnxGraph *graph,
          GnxGraph *g,
          GnxSet *seen,
          GnxQueue *queue)
{
    GnxNeighborIter iter;
    gnxptr vptr;
    unsigned int *u, *v;

    errno = 0;

    while (queue->size) {
        u = gnx_queue_pop(queue);
        g_assert(u);

        /* Iterate over each (out-)neighbor of u.  Ignore the edge weights. */
        gnx_neighbor_iter_init(&iter, graph, u);
        while (gnx_neighbor_iter_next(&iter, &vptr, NULL)) {
            v = (unsigned int *)vptr;
            g_assert(v);

            /* This should take care of nodes that we have seen.  Furthermore,
             * it should take care of the case where u has a self-loop.
             */
            if (gnx_set_has(seen, v))
                continue;

            g_assert(*u != *v);
            if (!gnx_set_add(seen, v))
                goto cleanup;
            if (!gnx_queue_append(queue, v))
                goto cleanup;
            if (!gnx_add_edge(g, u, v))
                goto cleanup;
        }
    }

    g_assert(g->total_nodes == seen->size);
    g_assert(g->total_nodes >= 2);
    g_assert(g->total_edges);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    return GNX_FAILURE;
}

/**
 * @brief Iterates over the neighbors of a node in default order.
 *
 * The default order means that we iterate over the neighbors of a node as
 * we encounter them.
 *
 * @param graph We want to iterate over some nodes in this graph.
 * @param v Iterate over the neighbors of this node.
 * @param stack Push each iterated neighbor onto this stack.
 * @return Nonzero if the iteration was successful; zero otherwise.
 */
static int
gnx_i_default_order(GnxGraph *graph,
                    unsigned int *v,
                    GnxStack *stack)
{
    GnxNeighborIter iter;
    gnxptr wptr;
    unsigned int *w;

    gnx_neighbor_iter_init(&iter, graph, v);
    while (gnx_neighbor_iter_next(&iter, &wptr, NULL)) {
        w = (unsigned int *)wptr;
        g_assert(w);
        if (!gnx_stack_push(stack, w))
            return GNX_FAILURE;
    }

    return GNX_SUCCESS;
}

/**
 * @brief Traverses a graph via the strategy of depth-first search.
 *
 * @param graph Traverse this graph.
 * @param g The search tree will be stored here.
 * @param start Start the traversal from this node.  The node is assumed to
 *        be in the graph.
 * @param seen A set of nodes that we have visited.
 * @param stack A stack of nodes to visit.
 * @return Nonzero if the traversal was successful; zero otherwise.  If we are
 *         unable to allocate memory while traversing the graph, then @c errno
 *         is set to @c ENOMEM and we return zero.
 */
static int
gnx_i_dfs(GnxGraph *graph,
          GnxGraph *g,
          unsigned int *start,
          GnxSet *seen,
          GnxStack *stack)
{
    GnxDict *parent;
    unsigned int *p, *u;

    errno = 0;
    parent = gnx_init_dict();
    if (!parent)
        goto cleanup;

    while (stack->size) {
        u = gnx_stack_pop(stack);
        g_assert(u);

        /* This should take care of nodes that we have seen.  Furthermore,
         * it should take care of the case where u has a self-loop.
         */
        if (gnx_set_has(seen, u))
            continue;

        /* Add the edge (parent[u], u) to the DFS tree. */
        if (*u != *start) {
            p = (unsigned int *)gnx_dict_has(parent, u);
            g_assert(p);
            assert(gnx_dict_delete(parent, u));
            if (!gnx_add_edge(g, p, u))
                goto cleanup;
        }

        if (!gnx_set_add(seen, u))
            goto cleanup;
        if (!gnx_i_dfs_push_onto_stack(graph, u, stack, parent))
            goto cleanup;
    }

    g_assert(g->total_nodes == seen->size);
    g_assert(g->total_nodes >= 2);
    g_assert(g->total_edges);
    gnx_destroy_dict(parent);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (parent)
        gnx_destroy_dict(parent);
    return GNX_FAILURE;
}

/**
 * @brief Pushes all the neighbors of a node onto the stack.
 *
 * @param graph Traverse this graph via depth-first search.
 * @param u A node of of the graph.
 * @param stack A stack of nodes to visit.
 * @param parent This will store the parents of each neighbor of the node.
 * @return Nonzero if all neighbors of u are pushed onto the stack;
 *         zero otherwise.
 */
static int
gnx_i_dfs_push_onto_stack(GnxGraph *graph,
                          unsigned int *u,
                          GnxStack *stack,
                          GnxDict *parent)
{
    GnxNeighborIter iter;
    gnxptr vptr;
    unsigned int *v;

    errno = 0;
    gnx_neighbor_iter_init(&iter, graph, u);
    while (gnx_neighbor_iter_next(&iter, &vptr, NULL)) {
        v = (unsigned int *)vptr;

        if (!gnx_stack_push(stack, v))
            goto cleanup;

        /* If the key v is already in the dictionary, then replace the
         * previous parent of v with the current parent u.
         */
        if (gnx_dict_has(parent, v))
            assert(gnx_dict_delete(parent, v));
        if (!gnx_dict_add(parent, v, u))
            goto cleanup;
    }

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    return GNX_FAILURE;
}

/**
 * @brief Iterates over the neighbors of a node in sorted order.
 *
 * The sorted order means that we iterate over the neighbors of a node in
 * increasing order of node IDs.
 *
 * @param graph We want to iterate over some nodes in this graph.
 * @param v Iterate over the neighbors of this node.
 * @param stack Push each iterated neighbor onto this stack.
 * @return Nonzero if the iteration was successful; zero otherwise.
 */
static int
gnx_i_sorted_order(GnxGraph *graph,
                   unsigned int *v,
                   GnxStack *stack)
{
    GnxArray *neighbor;
    GnxNeighborIter iter;
    gnxptr wptr;
    unsigned int i, *w;
    const unsigned int degree = gnx_degree(graph, v);
    const unsigned int capacity = gnx_least_power2_ge(&degree);

    errno = 0;

    /* The set of neighbors as an array. */
    neighbor = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    if (!neighbor)
        goto cleanup;
    gnx_neighbor_iter_init(&iter, graph, v);
    while (gnx_neighbor_iter_next(&iter, &wptr, NULL)) {
        g_assert(wptr);
        assert(gnx_array_append(neighbor, wptr));
    }

    /* Sort the array of neighbors.  Then push the sorted neighbors onto
     * the stack.  We first push the neighbor with the largest ID, then the
     * neighbor with the second largest ID, and so on all the way to the
     * neighbor that has the smallest ID.  When we finally pop the stack
     * we will be traversing the neighbors in increasing order of node ID.
     */
    assert(gnx_array_sort(neighbor));
    i = neighbor->size;
    while (i) {
        i--;
        w = (unsigned int *)(neighbor->cell[i]);
        if (!gnx_stack_push(stack, w))
            goto cleanup;
    }
    gnx_destroy_array(neighbor);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    gnx_destroy_array(neighbor);
    return GNX_FAILURE;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Traverses a graph via the strategy of breadth-first search.
 *
 * If the given graph is directed, then we traverse the graph via out-neighbors
 * of nodes.
 *
 * @param graph Traverse this graph.
 * @param s Start the traversal from this node.  The node is assumed to be in
 *        the graph.
 * @return A breadth-first search tree that is rooted at @f$s@f$.  To destroy
 *         the tree, use the function gnx_destroy().  We return @c NULL if
 *         @a s does not have neighbors.  If we are unable to allocate memory,
 *         then @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxGraph*
gnx_breadth_first_search(GnxGraph *graph,
                         const unsigned int *s)
{
    GnxGraph *g = NULL;
    GnxQueue *queue = NULL;
    GnxSet *seen = NULL;
    int success;
    unsigned int directed, start;

    errno = 0;
    g_return_val_if_fail(gnx_has_node(graph, s), NULL);
    if (graph->directed) {
        if (!gnx_outdegree(graph, s))
            return NULL;
        if ((1 == gnx_outdegree(graph, s)) && gnx_has_edge(graph, s, s))
            return NULL;
    } else {
        if (!gnx_degree(graph, s))
            return NULL;
        if ((1 == gnx_degree(graph, s)) && gnx_has_edge(graph, s, s))
            return NULL;
    }

    /* Initialize the BFS tree. */
    directed = (graph->directed) ? GNX_DIRECTED : GNX_UNDIRECTED;
    g = gnx_new_full(directed, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    if (!g)
        goto cleanup;

    /* This will store the set of nodes that we have visited. */
    seen = gnx_init_set();
    if (!seen)
        goto cleanup;

    /* A queue of nodes to visit. */
    queue = gnx_init_queue();
    if (!queue)
        goto cleanup;

    /* Append the starting node to the queue. */
    start = *s;
    if (!gnx_queue_append(queue, &start))
        goto cleanup;

    /* Start the traversal. */
    if (!gnx_set_add(seen, &start))
        goto cleanup;

    success = gnx_i_bfs(graph, g, seen, queue);

    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);
    if (success)
        return g;

    gnx_destroy(g);
    return NULL;

cleanup:
    errno = ENOMEM;
    gnx_destroy(g);
    gnx_destroy_queue(queue);
    gnx_destroy_set(seen);
    return NULL;
}

/**
 * @brief Traverses a graph via the strategy of depth-first search.
 *
 * If the given graph is directed, then we traverse the graph via out-neighbors
 * of nodes.
 *
 * @sa gnx_pre_order()
 *
 * @param graph Traverse this graph.
 * @param s Start the traversal from this node.  The node is assumed to be in
 *        the graph.
 * @return A depth-first search tree that is rooted at @f$s@f$.  To destroy
 *         the tree, use the function gnx_destroy().  We return @c NULL if
 *         @a s does not have neighbors.  If we are unable to allocate memory,
 *         then @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxGraph*
gnx_depth_first_search(GnxGraph *graph,
                       const unsigned int *s)
{
    GnxGraph *g = NULL;
    GnxSet *seen = NULL;
    GnxStack *stack = NULL;
    int success;
    unsigned int directed, start;

    errno = 0;
    g_return_val_if_fail(gnx_has_node(graph, s), NULL);
    if (graph->directed) {
        if (!gnx_outdegree(graph, s))
            return NULL;
        if ((1 == gnx_outdegree(graph, s)) && gnx_has_edge(graph, s, s))
            return NULL;
    } else {
        if (!gnx_degree(graph, s))
            return NULL;
        if ((1 == gnx_degree(graph, s)) && gnx_has_edge(graph, s, s))
            return NULL;
    }

    /* Initialize the DFS tree. */
    directed = (graph->directed) ? GNX_DIRECTED : GNX_UNDIRECTED;
    g = gnx_new_full(directed, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    if (!g)
        goto cleanup;

    /* This will store the set of nodes that we have visited. */
    seen = gnx_init_set();
    if (!seen)
        goto cleanup;

    /* A stack of nodes to visit. */
    stack = gnx_init_stack();
    if (!stack)
        goto cleanup;

    /* Append the starting node to the stack. */
    start = *s;
    if (!gnx_stack_push(stack, &start))
        goto cleanup;

    success = gnx_i_dfs(graph, g, &start, seen, stack);

    gnx_destroy_stack(stack);
    gnx_destroy_set(seen);
    if (success)
        return g;

    gnx_destroy(g);
    return NULL;

cleanup:
    errno = ENOMEM;
    gnx_destroy(g);
    gnx_destroy_stack(stack);
    gnx_destroy_set(seen);
    return NULL;
}

/**
 * @brief A pre-order traversal of a tree.
 *
 * The pre-order traversal of a tree is similar to depth-first search.
 *
 * @sa gnx_depth_first_search()
 *
 * @param tree The tree to traverse.  The given graph must be undirected and
 *        must not allow self-loops.
 * @param root The root of the tree.  This must be a node of the tree.
 * @param order An ordering of the neighbors of a node.  Possible values are:
 *        <ul>
 *        <li>#GNX_DEFAULT_ORDER: We iterate over the neighbors of a node in
 *            the order in which we encounter them.  This ordering is not
 *            guaranteed to be reproducible.  However, with this ordering we
 *            have a worst-case time complexity of @f$O(n)@f$ for a tree that
 *            has @f$n@f$ nodes.</li>
 *        <li>#GNX_SORTED_ORDER: The neighbors of a node are visited in
 *            increasing order of their IDs.  The worst-case runtime depends on
 *            the sorting function.</li>
 *        </ul>
 * @return An array of the nodes of the given tree in pre-order.  The given
 *         root is the first element of the array.  The array size is the
 *         number of nodes in the tree.   When you no longer need the array,
 *         you must destroy it via the function gnx_destroy_array().  If we are
 *         unable to allocate memory during the traversal, then @c errno is set
 *         to @c ENOMEM and we return @c NULL.
 */
GnxArray*
gnx_pre_order(GnxGraph *tree,
              const unsigned int *root,
              const GnxMethod order)
{
    GnxArray *list = NULL;
    GnxSet *seen = NULL;
    GnxStack *stack = NULL;
    int default_order, sorted_order;
    unsigned int nnode, *node, start, *v;

    errno = 0;
    g_return_val_if_fail(gnx_is_tree(tree), NULL);
    nnode = tree->total_nodes;
    g_return_val_if_fail(nnode <= GNX_MAXIMUM_NODES, NULL);
    g_return_val_if_fail(gnx_has_node(tree, root), NULL);
    gnx_i_check_order(order);
    default_order = GNX_DEFAULT_ORDER & order;
    sorted_order = GNX_SORTED_ORDER & order;

    /* This will hold the nodes in pre-order. */
    list = gnx_init_array_full(&(tree->capacity), GNX_FREE_ELEMENTS, GNX_UINT);
    if (!list)
        goto cleanup;

    /* This will hold all nodes that we have visited. */
    seen = gnx_init_set();
    if (!seen)
        goto cleanup;

    /* Push the root onto the empty stack. */
    start = *root;
    stack = gnx_init_stack();
    if (!stack)
        goto cleanup;
    if (!gnx_stack_push(stack, &start))
        goto cleanup;

    /* Perform the pre-order traversal. */
    while (stack->size) {
        v = gnx_stack_pop(stack);
        g_assert(v);

        if (gnx_set_has(seen, v))
            continue;

        node = (unsigned int *)malloc(sizeof(unsigned int));
        if (!node)
            goto cleanup;
        *node = *v;
        assert(gnx_array_append(list, node));
        if (!gnx_set_add(seen, v))
            goto cleanup;

        /* Iterate over the neighbors of v using the default order. */
        if (default_order) {
            if (!gnx_i_default_order(tree, v, stack))
                goto cleanup;
            continue;
        }

        /* Iterate over the neighbors of v in increasing order of node ID. */
        g_assert(sorted_order);
        if (!gnx_i_sorted_order(tree, v, stack))
            goto cleanup;
    }

    g_assert(nnode == list->size);
    gnx_destroy_set(seen);
    gnx_destroy_stack(stack);
    return list;

cleanup:
    errno = ENOMEM;
    gnx_destroy_array(list);
    gnx_destroy_set(seen);
    gnx_destroy_stack(stack);
    return NULL;
}
