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
#include <string.h>

#include <glib.h>

#include "base.h"
#include "io.h"
#include "sanity.h"
#include "set.h"

/**
 * @file io.h
 * @brief Read graphs from and write graphs to files.
 *
 * The gnx format for specifying graphs is similar to an edge list format.
 * A file that contains a graph must list the edges of the graph, one edge per
 * line and following the format:
 *
 * <pre>
 * node1,node2[,weight]
 * </pre>
 *
 * That is, each line contains an edge both of whose nodes are delimited by a
 * comma.  The optional @a weight specifies the weight of the edge.  It is
 * possible for a graph to have a node that does not link to any other node in
 * the graph.  Such isolated nodes can be specified on a line in the graph file
 * as follows:
 *
 * <pre>
 * node
 * </pre>
 *
 * That is, the line contains a node ID followed by the new line character.
 * The line will be interpreted as specifying a node of the graph.
 *
 * Comments are delimited by the hash symbol @c #.  Do not mix on the same line
 * the specification of an edge (or an isolated node) followed by a comment.
 * Below is an example of a graph that is specificied in the gnx format.
 *
 * <pre>
 * \# This is a comment.
 * \# This is a graph with node 3 as the isolated node.
 * 0,1
 * 1,2
 * 3
 * </pre>
 *
 * Here is an example of a weighted graph.
 *
 * <pre>
 * \# This is a weighted graph with one isolated node.
 * 0,1,3.14159
 * 1,2,2.71828
 * 42
 * </pre>
 */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static void gnx_i_io_cleanup(GIOChannel *file,
                             gchar *line,
                             GError *error,
                             GnxSet *seen);
static int gnx_i_node_str2int(gchar *node,
                              unsigned int *u,
                              const gchar *filename,
                              const unsigned int *line);
static int gnx_i_read_edge(gchar *line,
                           const gchar *filename,
                           const unsigned int *n,
                           unsigned int *u,
                           unsigned int *v,
                           const GnxBool weighted,
                           double *weight);
static int gnx_i_read_has_weight(const gchar *line);
static int gnx_i_weight_str2d(gchar *weight,
                              double *w,
                              const gchar *filename,
                              const unsigned int *line);
static int gnx_i_write_directed(GnxGraph *graph,
                                const char *filename);
static int gnx_i_write_edge(GIOChannel *file,
                            const unsigned int *u,
                            const unsigned int *v,
                            const double *weight);
static int gnx_i_write_node(GIOChannel *file,
                            const unsigned int *v);
static int gnx_i_write_undirected(GnxGraph *graph,
                                  const char *filename);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Clean up some junks that result from opening an I/O channel and
 *        reading from or writing to that channel.
 *
 * @param file The I/O channel.
 * @param line A line that was read from or written to the I/O channel.
 * @param error Any error that occurred by reading from or writing to
 *        the I/O channel.
 * @param seen The set of nodes that have been written to file.
 */
static void
gnx_i_io_cleanup(GIOChannel *file,
                 gchar *line,
                 GError *error,
                 GnxSet *seen)
{
    if (line)
        g_free(line);
    if (error)
        g_error_free(error);
    if (file) {
        (void)g_io_channel_shutdown(file, FALSE, &error);
        if (error)
            g_error_free(error);
        g_io_channel_unref(file);
    }
    gnx_destroy_set(seen);
}

/**
 * @brief Converts a string representation of a node ID to an integer.
 *
 * @param node A string representation of a node ID.  Before calling this
 *        function, you must strip the string of all leading and trailing
 *        white spaces.  Consider using the function g_strstrip() from glib.
 * @param u This will store the integer representation of the node ID.  If
 *        the conversion fails, this will be @c NULL.
 * @param filename The node was read from this file.
 * @param line The node occurred at this line number in @a filename.
 * @return Nonzero if the conversion was successful; zero otherwise.
 */
static int
gnx_i_node_str2int(gchar *node,
                   unsigned int *u,
                   const gchar *filename,
                   const unsigned int *line)
{
    gint64 u64;
    int valid;
    unsigned int i;

    /* A valid node ID? */
    i = 0;
    valid = GNX_SUCCESS;
    while (node[i] != '\0') {
        if (!g_ascii_isdigit(node[i])) {
            valid = GNX_FAILURE;
            break;
        }
        i++;
    }
    /* An empty string. */
    if (!i)
        valid = GNX_FAILURE;
    if (!valid) {
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
              "invalid node ID at line %u in %s: %s",
              *line, filename, node);
        return GNX_FAILURE;
    }

    /* The function g_ascii_strtoll() from glib is known to set errno.  As per
     * the CERT rule ERR30-C, we set errno to zero prior to calling the
     * function.  We then check the return value of the function to see whether
     * an overflow has occurred.  If the return value is ambiguous, we then
     * check the value of errno.  Rule ERR30-C of CERT reads
     * (accessed on 2015-10-27):
     *
     *     The value of errno is initialized to zero at program startup, but
     *     it is never subsequently set to zero by any C standard library
     *     function.  The value of errno may be set to nonzero by a C standard
     *     library function call whether or not there is an error, provided the
     *     use of errno is not documented in the description of the function.
     *     It is meaningful for a program to inspect the contents of errno only
     *     after an error might have occurred.  More precisely, errno is
     *     meaningful only after a library function that sets errno on error
     *     has returned an error code.
     *
     * The proper use of errno is also covered in Question 20.4 of C-FAQ
     * (accessed on 2015-10-27):
     *
     *     In general, you should detect errors by checking return values, and
     *     use errno only to distinguish among the various causes of an error,
     *     such as ``File not found'' or ``Permission denied''.  (Typically,
     *     you use perror or strerror to print these discriminating error
     *     messages.)  It's only necessary to detect errors with errno when a
     *     function does not have a unique, unambiguous, out-of-band error
     *     return (i.e. because all of its possible return values are valid;
     *     one example is atoi).  In these cases (and in these cases only;
     *     check the documentation to be sure whether a function allows this),
     *     you can detect errors by setting errno to 0, calling the function,
     *     then testing errno.  (Setting errno to 0 first is important, as no
     *     library function ever does that for you.)
     */

    errno = 0;
    u64 = g_ascii_strtoll(g_strstrip(node), NULL, GNX_BASE_10);

    if ((u64 > G_MAXINT) || (u64 < G_MININT)) {
        u = NULL;
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
              "overflow at line %u in %s: %s",
              *line, filename, strerror(ERANGE));
        return GNX_FAILURE;
    }

    g_assert(u64 != G_MAXINT64);
    g_assert(u64 != G_MININT64);
    *u = (unsigned int)u64;

    return GNX_SUCCESS;
}

/**
 * @brief Converts a string representation of an edge (or node) into integer
 *        IDs.
 *
 * @param line A string representation of an edge or node.  This is a line
 *        that was read from @a filename.
 * @param filename The edge or node was read from this file.
 * @param n The edge or node occurred at this line number in @a filename.
 * @param u The tail component of the edge will be stored here.
 * @param v The head component of the edge will be stored here.
 * @param weighted Whether we are reading a weighted graph.  Possible values
 *        are: #GNX_WEIGHTED or #GNX_UNWEIGHTED.  It is possible to pass in
 *        #GNX_UNWEIGHTED even though the graph to read is weighted.  In this
 *        case, we will ignore all edge weights and obtain an unweighted
 *        version of the graph.
 * @param weight The weight (if any) of the edge will be stored here.
 * @return Nonzero if @a line represents a valid edge or a node;
 *         #GNX_COMMENT_LINE if @a line represents a comment;
 *         #GNX_NODE_LINE if @a line represents a node; zero otherwise.
 *         If @a line represents a valid edge, then @a u will store the tail
 *         component and @a v will store the head component.  If @a line
 *         represents a node, then @a u will store the node ID, @a v will not
 *         be set, and we return #GNX_NODE_LINE.
 */
static int
gnx_i_read_edge(gchar *line,
                const gchar *filename,
                const unsigned int *n,
                unsigned int *u,
                unsigned int *v,
                const GnxBool weighted,
                double *weight)
{
    gchar *tmp, **token;
    int isedge, valid;
    const gint all_tokens = -1;
    const int isweighted = GNX_WEIGHTED & weighted;

    if (line[0] == GNX_COMMENT)
        return GNX_COMMENT_LINE;

    /* If the graph to read is weighted, then we expect each line to be an
     * edge, followed by the corresponding edge weight.  If a node is isolated,
     * i.e. the node has zero neighbors, then the node ID must be the only
     * value on the line.
     */
    if (isweighted) {
        /* Is there an edge weight? */
        tmp = g_strrstr(line, GNX_EDGE_DELIMITER);

        if (tmp) {
            /* We expect the line to have an edge weight. */
            if (!gnx_i_read_has_weight(g_strstrip(line))) {
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
                      "edge weight not found at line %u in %s: %s",
                      *n, filename, g_strstrip(line));
                return GNX_FAILURE;
            }
        }
    }

    isedge = TRUE;

    /* Does the line represent an edge? */
    tmp = g_strrstr(line, GNX_EDGE_DELIMITER);
    if (!tmp)
        isedge = FALSE;

    /* Does the line represent a node? */
    if (!isedge) {
        valid = gnx_i_node_str2int(g_strstrip(line), u, filename, n);
        return (valid) ? GNX_NODE_LINE : GNX_FAILURE;
    }

    g_assert(isedge);

    /* First, we get the tail node of the edge. */
    token = g_strsplit(line, GNX_EDGE_DELIMITER, all_tokens);
    valid = gnx_i_node_str2int(g_strstrip(token[0]), u, filename, n);
    if (!valid) {
        g_strfreev(token);
        return GNX_FAILURE;
    }

    /* Next, we get the head node of the edge. */
    valid = gnx_i_node_str2int(g_strstrip(token[1]), v, filename, n);
    if (!valid) {
        g_strfreev(token);
        return GNX_FAILURE;
    }

    /* Finally, we get the edge weight. */
    if (isweighted) {
        valid = gnx_i_weight_str2d(g_strstrip(token[2]), weight, filename, n);
        if (!valid) {
            g_strfreev(token);
            return GNX_FAILURE;
        }
    }

    g_strfreev(token);

    return GNX_SUCCESS;
}

/**
 * @brief Whether the given line has an edge weight.
 *
 * @param line A line that was read from a file.
 * @return Nonzero if @a line has an edge and an edge weight; zero otherwise.
 */
static int
gnx_i_read_has_weight(const gchar *line)
{
    int i = 0;
    int ncomma = 0;
    const char comma = ',';
    const int required_commas = 2;

    while (line[i] != '\0') {
        if (line[i] == comma)
            ncomma++;
        if (ncomma == required_commas)
            break;

        i++;
    }

    if (ncomma != required_commas)
        return GNX_FAILURE;
    if (line[i + 1] == '\0')
        return GNX_FAILURE;
    if (line[i + 1] == ' ')
        return GNX_FAILURE;
    if (line[i + 1] == '\t')
        return GNX_FAILURE;

    return GNX_SUCCESS;
}

/**
 * @brief Converts to double the string representation of the edge weight.
 *
 * @param weight A string representation of an edge weight.  Before calling
 *        this function, you must strip the string of all leading and trailing
 *        white spaces.  Consider using the function g_strstrip() from glib.
 * @param w This will store the double representation of the edge weight.  If
 *        the conversion fails, this will be @c NULL.
 * @param filename The edge weight was read from this file.
 * @param line The edge weight occurred at this line number in @a filename.
 * @return Nonzero if the conversion was successful; zero otherwise.
 */
static int
gnx_i_weight_str2d(gchar *weight,
                   double *w,
                   const gchar *filename,
                   const unsigned int *line)
{
    gchar **token;
    gdouble edge_weight;
    gint64 u64;
    int valid;
    unsigned int i, nminus, nperiod;
    const char minus_sign = '-';
    const char period = '.';
    const gint all_tokens = -1;
    const unsigned int max_minus_signs = 1;
    const unsigned int max_periods = 1;

    g_assert(weight[0] != '\0');

    /* A valid edge weight? */

    i = 0;
    nminus = 0;
    nperiod = 0;
    valid = TRUE;

    while (weight[i] != '\0') {
        if (weight[i] == period) {
            nperiod++;
            i++;
            continue;
        }
        if (weight[i] == minus_sign) {
            nminus++;
            i++;
            continue;
        }
        if (!g_ascii_isdigit(weight[i])) {
            valid = FALSE;
            break;
        }
        i++;
    }

    if (nperiod > max_periods)
        valid = FALSE;
    if (nminus > max_minus_signs)
        valid = FALSE;

    if (!valid) {
        w = NULL;
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
              "invalid edge weight at line %u in %s: %s",
              *line, filename, weight);
        return GNX_FAILURE;
    }

    /* The functions g_ascii_strtoll() and g_ascii_strtod() from glib are known
     * to set errno.  As per the CERT rule ERR30-C, we set errno to zero prior
     * to calling any of these functions.  Rule ERR30-C of CERT reads
     * (accessed on 2015-10-27):
     *
     *     The value of errno is initialized to zero at program startup, but
     *     it is never subsequently set to zero by any C standard library
     *     function.  The value of errno may be set to nonzero by a C standard
     *     library function call whether or not there is an error, provided the
     *     use of errno is not documented in the description of the function.
     *     It is meaningful for a program to inspect the contents of errno only
     *     after an error might have occurred.  More precisely, errno is
     *     meaningful only after a library function that sets errno on error
     *     has returned an error code.
     *
     * The proper use of errno is also covered in Question 20.4 of C-FAQ
     * (accessed on 2015-10-27):
     *
     *     In general, you should detect errors by checking return values, and
     *     use errno only to distinguish among the various causes of an error,
     *     such as ``File not found'' or ``Permission denied''.  (Typically,
     *     you use perror or strerror to print these discriminating error
     *     messages.)  It's only necessary to detect errors with errno when a
     *     function does not have a unique, unambiguous, out-of-band error
     *     return (i.e. because all of its possible return values are valid;
     *     one example is atoi).  In these cases (and in these cases only;
     *     check the documentation to be sure whether a function allows this),
     *     you can detect errors by setting errno to 0, calling the function,
     *     then testing errno.  (Setting errno to 0 first is important, as no
     *     library function ever does that for you.)
     */

    /* First, we determine whether the integer part of the edge weight can
     * fit in an int.
     */
    token = g_strsplit(weight, &period, all_tokens);
    errno = 0;
    u64 = g_ascii_strtoll(token[0], NULL, GNX_BASE_10);
    g_strfreev(token);
    if ((u64 > G_MAXINT) || (u64 < G_MININT)) {
        w = NULL;
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
              "edge weight overflow at line %u in %s: %s",
              *line, filename, strerror(ERANGE));
        return GNX_FAILURE;
    }

    /* Next, we determine whether the whole edge weight can overflow. */
    errno = 0;
    edge_weight = g_ascii_strtod(weight, NULL);
    if (ERANGE == errno) {
        w = NULL;
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
              "edge weight overflow at line %u in %s: %s",
              *line, filename, strerror(ERANGE));
        return GNX_FAILURE;
    }

    *w = (double)edge_weight;

    return GNX_SUCCESS;
}

/**
 * @brief Writes a directed graph to file.
 *
 * @param graph The directed graph to output to file.
 * @param filename Write the graph to this file.  This file must not already
 *        exist.
 * @return Nonzero if the graph was successfully written to file; zero
 *         otherwise.  If we cannot write to file, then @c errno is set to
 *         @c EIO and we return zero.
 */
static int
gnx_i_write_directed(GnxGraph *graph,
                     const char *filename)
{
    double weight;
    GError *error = NULL;
    GnxNeighborIter iternei;
    GnxNodeIter iter;
    gnxptr vptr, wptr;
    int valid;
    GIOChannel *file;
    unsigned int u, v;

    errno = 0;
    file = g_io_channel_new_file((const gchar *)filename, "w", &error);
    if (!file) {
        errno = EIO;
        if (error)
            g_error_free(error);

        return GNX_FAILURE;
    }

    /* Iterate over each node. */
    gnx_node_iter_init(&iter, graph);
    while (gnx_node_iter_next(&iter, &u)) {
        /* Node u is isolated.  It has zero out-neighbors and zero
         * in-neighbors.
         */
        if (!gnx_outdegree(graph, &u) && !gnx_indegree(graph, &u)) {
            if (!gnx_i_write_node(file, &u))
                goto cleanup;

            continue;
        }

        /* Iterate over each out-neighbor v of u. */
        gnx_neighbor_iter_init(&iternei, graph, &u);
        while (gnx_neighbor_iter_next(&iternei, &vptr, &wptr)) {
            v = *((unsigned int *)vptr);

            if (graph->weighted) {
                weight = *((double *)wptr);
                valid = gnx_i_write_edge(file, &u, &v, &weight);
            } else {
                valid = gnx_i_write_edge(file, &u, &v, NULL);
            }

            if (!valid)
                goto cleanup;
        }
    }

    (void)g_io_channel_flush(file, &error);
    gnx_i_io_cleanup(file, NULL, error, NULL);

    return GNX_SUCCESS;

cleanup:
    errno = EIO;
    gnx_i_io_cleanup(file, NULL, NULL, NULL);
    return GNX_FAILURE;
}

/**
 * @brief Writes an edge to a file.
 *
 * @param file Write the edge to this file.  The file must already be opened.
 * @param u The tail of the edge.
 * @param v The head of the edge.
 * @param weight If the graph is weighted, this must be the weight of the
 *        edge @f$(u,v)@f$.  If the graph is unweighted, pass in NULL.
 * @return Nonzero if the edge @f$(u,v)@f$ was successfully written to file;
 *         zero otherwise.  If we cannot write an edge to file, then @c errno
 *         is set to @c EIO and we return zero.
 */
static int
gnx_i_write_edge(GIOChannel *file,
                 const unsigned int *u,
                 const unsigned int *v,
                 const double *weight)
{
    char buffer[G_ASCII_DTOSTR_BUF_SIZE];
    gchar *line;
    GError *error = NULL;
    GIOStatus status;
    gsize bytes_written;
    const gssize null_term_str = -1;

    errno = 0;
    if (weight) {
        line = g_strdup_printf("%d,%d,%s%s", *u, *v,
                               g_ascii_dtostr(buffer, sizeof(buffer), *weight),
                               GNX_NEWLINE);
    } else {
        line = g_strdup_printf("%d,%d%s", *u, *v, GNX_NEWLINE);
    }

    status = g_io_channel_write_chars(file, line, null_term_str,
                                      &bytes_written, &error);
    if ((status == G_IO_STATUS_ERROR)
        || (status == G_IO_STATUS_AGAIN)) {
        errno = EIO;
        gnx_i_io_cleanup(NULL, line, error, NULL);
        return GNX_FAILURE;
    }

    g_assert(status == G_IO_STATUS_NORMAL);
    g_free(line);

    return GNX_SUCCESS;
}

/**
 * @brief Writes a node to a file.
 *
 * @param file Write the node to this file.  This file must already be opened.
 * @param v Write this node to file.
 * @return Nonzero if the node was successfully written to file;
 *         zero otherwise.  If we cannot write the node to file, then we set
 *         @c errno to @c EIO and return zero.
 */
static int
gnx_i_write_node(GIOChannel *file,
                 const unsigned int *v)
{
    gchar *line;
    GError *error = NULL;
    GIOStatus status;
    gsize bytes_written;
    const gssize null_term_str = -1;

    errno = 0;
    line = g_strdup_printf("%d%s", *v, GNX_NEWLINE);
    status = g_io_channel_write_chars(file, line, null_term_str,
                                      &bytes_written, &error);
    if ((status == G_IO_STATUS_ERROR)
        || (status == G_IO_STATUS_AGAIN)) {
        errno = EIO;
        gnx_i_io_cleanup(NULL, line, error, NULL);
        return GNX_FAILURE;
    }

    g_assert(status == G_IO_STATUS_NORMAL);
    g_free(line);

    return GNX_SUCCESS;
}

/**
 * @brief Writes an undirected graph to file.
 *
 * @param graph The undirected graph to output to file.
 * @param filename Write the graph to this file.  This file must not already
 *        exist.
 * @return Nonzero if the graph was successfully written to file; zero
 *         otherwise.  If we cannot write to file, then we set @c errno to
 *         @c EIO and return zero.
 */
static int
gnx_i_write_undirected(GnxGraph *graph,
                       const char *filename)
{
    double weight;
    GError *error = NULL;
    GnxNeighborIter iternei;
    GnxNodeIter iter;
    GIOChannel *file;
    gnxptr vptr, wptr;
    int valid;
    unsigned int u, v;

    errno = 0;
    file = g_io_channel_new_file((const gchar *)filename, "w", &error);
    if (!file) {
        errno = EIO;
        if (error)
            g_error_free(error);

        return GNX_FAILURE;
    }

    /* Iterate over each node u. */
    gnx_node_iter_init(&iter, graph);
    while (gnx_node_iter_next(&iter, &u)) {
        /* Node u is isolated.  It does not link to any other node. */
        if (!gnx_degree(graph, &u)) {
            if (!gnx_i_write_node(file, &u))
                goto cleanup;

            continue;
        }

        /* Iterate over each neighbor v of u.  Since the graph is undirected,
         * both of the edges (u,v) and (v,u) are the same.  We only write the
         * edge (u,v) to file provided that u <= v.
         */
        gnx_neighbor_iter_init(&iternei, graph, &u);
        while (gnx_neighbor_iter_next(&iternei, &vptr, &wptr)) {
            v = *((unsigned int *)vptr);

            if (u > v)
                continue;

            if (graph->weighted) {
                weight = *((double *)wptr);
                valid = gnx_i_write_edge(file, &u, &v, &weight);
            } else {
                valid = gnx_i_write_edge(file, &u, &v, NULL);
            }

            if (!valid)
                goto cleanup;
        }
    }

    (void)g_io_channel_flush(file, &error);
    gnx_i_io_cleanup(file, NULL, error, NULL);

    return GNX_SUCCESS;

cleanup:
    errno = EIO;
    gnx_i_io_cleanup(file, NULL, NULL, NULL);
    return GNX_FAILURE;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Reads in a graph from a file.
 *
 * @param filename Reads in a graph from this file.  The file is assumed to
 *        contain a graph that is specified according to the gnx format.
 *        See the section <em>Detailed Description</em> for an explanation of
 *        the gnx format.
 * @param directed Whether the graph in @a filename is directed.
 *        The possible values are: #GNX_UNDIRECTED or #GNX_DIRECTED.
 * @param selfloop Whether the graph in @a filename has self-loops.  The
 *        possible values are: #GNX_NO_SELFLOOP or #GNX_SELFLOOP.
 * @param weighted Whether the graph in @a filename is weighted.  The possible
 *        values are: #GNX_UNWEIGHTED or #GNX_WEIGHTED.  If
 *        #GNX_WEIGHTED, then we expect each edge that is read to be
 *        accompanied by a numeric weight.  If weighted, it is your
 *        responsibility to ensure that the integer part of the edge weight
 *        fits in an int and that the whole weight fits in a double.
 * @return If reading from @a filename was successful, we return a new graph
 *         with the graph structure as specified in @a filename.  We return
 *         @c NULL if @a filename does not specify a graph structure.  If
 *         @a filename does not exist, we set @c errno to @c ENOENT and return
 *         @c NULL.  If @a filename is a directory, we set @c errno to
 *         @c EISDIR and return @c NULL.  If we cannot read from @a filename,
 *         then @c errno is set to @c EIO and we return @c NULL.
 */
GnxGraph*
gnx_read(const char *filename,
         const GnxBool directed,
         const GnxBool selfloop,
         const GnxBool weighted)
{
    double weight;
    gchar *line;
    GError *error = NULL;
    int valid;
    GIOChannel *file;
    GIOStatus status;
    GnxGraph *graph;
    gsize length;
    unsigned int i, u, v;

    errno = 0;
    g_return_val_if_fail(filename, NULL);
    gnx_i_check_properties(directed, selfloop, weighted);

    if (!g_file_test((const gchar *)filename, G_FILE_TEST_EXISTS)) {
        errno = ENOENT;
        return NULL;
    }
    if (g_file_test((const gchar *)filename, G_FILE_TEST_IS_DIR)) {
        errno = EISDIR;
        return NULL;
    }

    file = g_io_channel_new_file((const gchar *)filename, "r", &error);
    if (error) {
        errno = EIO;
        gnx_i_io_cleanup(NULL, NULL, error, NULL);
        return NULL;
    }

    i = 0;
    graph = gnx_new_full(directed, selfloop, weighted);

    for (;;) {
        i++;
        status = g_io_channel_read_line(file, &line, &length, NULL, &error);
        if (G_IO_STATUS_NORMAL == status) {
            /* Parse the line. */
            valid = gnx_i_read_edge(line, (const gchar *)filename,
                                    &i, &u, &v, weighted, &weight);

            if (GNX_COMMENT_LINE == valid) {
                gnx_i_io_cleanup(NULL, line, error, NULL);
                continue;
            }

            if (!valid) {
                gnx_i_io_cleanup(file, line, error, NULL);
                gnx_destroy(graph);
                graph = NULL;
                break;
            }

            /* Does the line contain a node? */
            if (GNX_NODE_LINE == valid) {
                valid = gnx_add_node(graph, &u);
                if (!valid) {
                    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
                          "%s\nnode at line %u in %s: %s",
                          "unable to insert node into graph",
                          i, filename, g_strstrip(line));
                    gnx_i_io_cleanup(file, line, error, NULL);
                    gnx_destroy(graph);
                    graph = NULL;
                    break;
                }
                gnx_i_io_cleanup(NULL, line, error, NULL);
                continue;
            }

            /* Try to insert the edge (and possibly the edge weight) into the
             * graph.
             */

            if (GNX_WEIGHTED & graph->weighted)
                valid = gnx_add_edgew(graph, &u, &v, &weight);
            else
                valid = gnx_add_edge(graph, &u, &v);

            if (!valid) {
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE,
                      "%s\nedge at line %u in %s: %s",
                      "unable to insert edge into graph",
                      i, filename, g_strstrip(line));
                gnx_i_io_cleanup(file, line, error, NULL);
                gnx_destroy(graph);
                graph = NULL;
                break;
            }

            gnx_i_io_cleanup(NULL, line, error, NULL);
        } else if (G_IO_STATUS_EOF == status) {
            gnx_i_io_cleanup(file, line, error, NULL);
            break;
        } else if ((G_IO_STATUS_ERROR == status)
                   || (G_IO_STATUS_AGAIN == status)) {
            errno = EIO;
            gnx_i_io_cleanup(file, line, error, NULL);
            gnx_destroy(graph);
            graph = NULL;
            break;
        }
    }

    if (graph) {
        /* The graph is empty because filename does not contain a valid graph.
         */
        if (!graph->total_nodes) {
            gnx_destroy(graph);
            graph = NULL;
        }
    }

    return graph;
}

/**
 * @brief Writes a graph to file.
 *
 * You must ensure that the file does not exist before you write to it.
 * The function gnx_write() will refuse to write to an existing file.
 *
 * @param graph The graph to output to file.
 * @param filename Write the graph to this file.  This file must not already
 *        exist.  The file will specify the graph as per the gnx format.
 *        See the section <em>Detailed Description</em> for an explanation of
 *        the gnx format.
 * @return Nonzero if the graph was successfully written to file; zero
 *         otherwise.  We also return zero if the graph does not contain any
 *         node.  If @a filename is an existing file, we set @c errno to
 *         @c EEXIST and return zero.  If we cannot write to file, then
 *         @c errno is set to @c EIO and we return zero.
 */
int
gnx_write(GnxGraph *graph,
          const char *filename)
{
    errno = 0;
    gnx_i_check(graph);
    g_return_val_if_fail(filename, GNX_FAILURE);

    if (!graph->total_nodes)
        return GNX_FAILURE;
    if (g_file_test((const gchar *)filename, G_FILE_TEST_EXISTS)) {
        errno = EEXIST;
        return GNX_FAILURE;
    }

    if (graph->directed)
        return gnx_i_write_directed(graph, filename);
    return gnx_i_write_undirected(graph, filename);
}
