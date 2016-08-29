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

#include <glib.h>
#include <mpfr.h>

#include "base.h"
#include "sanity.h"
#include "util.h"

/**
 * @file util.h
 * @brief Miscellaneous utility functions that do not belong anywhere else.
 */

/**************************************************************************
 * internal data structures
 *************************************************************************/

/* @cond */
/* All powers of two 2^i for i = 0,...,31. */
static unsigned int power2[GNX_MAXIMUM_EXPONENT + 1] = {
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
/* The maximum power of two that we work with. */
static unsigned int max_power = 2147483648;  /* 2^31 */
/* @endcond */


/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_double_cmp(const double *a,
                            const double *b);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Compares two floating-point numbers.
 *
 * @param a A floating-point number.
 * @param b Another floating-point number.
 * @return A positive value if @f$a > b@f$, zero if @f$a = b@f$, and
 *         a negative value if @f$a < b@f$.
 */
static int
gnx_i_double_cmp(const double *a,
                 const double *b)
{
    int retval;
    mpfr_t ma, mb;

    g_return_val_if_fail(a, GNX_FAILURE);
    g_return_val_if_fail(b, GNX_FAILURE);

    mpfr_inits2(GNX_MPFR_PRECISION, ma, mb, (mpfr_ptr)NULL);

    (void)mpfr_set_d(ma, *a, MPFR_RNDN);
    (void)mpfr_set_d(mb, *b, MPFR_RNDN);
    retval = mpfr_cmp(ma, mb);

    mpfr_clears(ma, mb, (mpfr_ptr)NULL);
    mpfr_free_cache();

    return retval;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Chooses a node from a graph.
 *
 * Note that this function does not randomly choose a node of a graph.
 * The function merely iterates over the set of nodes and returns as soon
 * as a node is found.
 *
 * @param graph Choose a node from this graph.  The graph is assumed to have
 *        at least one node.
 * @return A node of the graph.
 */
unsigned int
gnx_any_node(GnxGraph *graph)
{
    GnxNodeIter iter;
    unsigned int v;

    gnx_i_check(graph);
    g_return_val_if_fail(graph->total_nodes, GNX_FAILURE);

    gnx_node_iter_init(&iter, graph);
    assert(gnx_node_iter_next(&iter, &v));

    return v;
}

/**
 * @brief Determines whether two graphs are the same.
 *
 * The function gnx_cmp() does not check whether two graphs are
 * structurally the same.  That is, it does not check for graph isomorphism.
 *
 * Rather, the function simply checks that the graphs have the same basic
 * properties such as directedness, allowance for self-loops, weightedness,
 * and the numbers of edges and nodes.  The function also checks that the
 * graphs have the same nodes and edges.
 *
 * @param g A graph to compare.
 * @param h Compare this graph with @a g.
 * @return Nonzero if both graphs are the same; zero otherwise.
 */
int
gnx_cmp(GnxGraph *g,
        GnxGraph *h)
{
    double weight_a, weight_b;
    GnxNeighborIter iternei;
    GnxNodeIter iter;
    gnxptr vptr, wptr;
    unsigned int u, v;

    gnx_i_check(g);
    gnx_i_check(h);

    if (g->total_nodes != h->total_nodes)
        return GNX_FAILURE;
    if (g->total_edges != h->total_edges)
        return GNX_FAILURE;
    if (g->directed != h->directed)
        return GNX_FAILURE;
    if (g->selfloop != h->selfloop)
        return GNX_FAILURE;
    if (g->weighted != h->weighted)
        return GNX_FAILURE;

    /* Iterate over each node u of G. */
    gnx_node_iter_init(&iter, g);
    while (gnx_node_iter_next(&iter, &u)) {
        if (!gnx_has_node(h, &u))
            return GNX_FAILURE;

        if (g->directed) {
            if (gnx_outdegree(g, &u) != gnx_outdegree(h, &u))
                return GNX_FAILURE;
            if (gnx_indegree(g, &u) != gnx_indegree(h, &u))
                return GNX_FAILURE;
        } else {
            if (gnx_degree(g, &u) != gnx_degree(h, &u))
                return GNX_FAILURE;
        }

        /* Compare the neighbors of u in G with the neighbors of u in H. */
        gnx_neighbor_iter_init(&iternei, g, &u);
        while (gnx_neighbor_iter_next(&iternei, &vptr, &wptr)) {
            v = *((unsigned int *)vptr);

            if (!gnx_has_edge(h, &u, &v))
                return GNX_FAILURE;
            if (g->weighted) {
                weight_a = *((double *)wptr);
                weight_b = gnx_edge_weight(h, &u, &v);
                if (!gnx_double_cmp_eq(&weight_a, &weight_b))
                    return GNX_FAILURE;
            }
        }
    }

    return GNX_SUCCESS;
}

/**
 * @brief Determines whether two floating-point numbers are equal.
 *
 * @param a A floating-point number.
 * @param b Another floating-point number.
 * @return Nonzero if @a a and @a b compare equal; zero otherwise.
 */
int
gnx_double_cmp_eq(const double *a,
                  const double *b)
{
    int retval = gnx_i_double_cmp(a, b);

    return (!retval) ? TRUE : FALSE;
}

/**
 * @brief Determines whether one floating-point number is less than or
 *        equal to another floating-point number.
 *
 * @param a A floating-point number.
 * @param b Another floating-point number.
 * @return Nonzero if @a a compares less than or equal to @a b; zero otherwise.
 */
int
gnx_double_cmp_le(const double *a,
                  const double *b)
{
    int retval = gnx_i_double_cmp(a, b);

    return (retval <= 0) ? TRUE : FALSE;
}

/**
 * @brief The least power of two that is greater than or equal to an integer.
 *
 * The maximum exponent @f$i@f$ in the power @f$2^i@f$ is defined as
 * #GNX_MAXIMUM_EXPONENT.
 *
 * @param n An integer.
 * @return The least power of two @f$k = 2^i@f$ such that @f$k >= n@f$.
 */
unsigned int
gnx_least_power2_ge(const unsigned int *n)
{
    unsigned int i;

    g_return_val_if_fail(n, GNX_FAILURE);
    g_assert(*n <= max_power);

    i = 0;
    while (*n > power2[i])
        i++;

    g_assert(*n <= power2[i]);

    return power2[i];
}

/**
 * @brief The least power of two that is greater than an integer.
 *
 * The maximum exponent @f$i@f$ in the power @f$2^i@f$ is defined as
 * #GNX_MAXIMUM_EXPONENT.
 *
 * @param n An integer.
 * @return The least power of two @f$k = 2^i@f$ such that @f$k > n@f$.
 */
unsigned int
gnx_least_power2_gt(const unsigned int *n)
{
    unsigned int i;

    g_return_val_if_fail(n, GNX_FAILURE);
    g_assert(*n < max_power);

    i = 0;
    while (*n >= power2[i])
        i++;

    g_assert(*n < power2[i]);

    return power2[i];
}
