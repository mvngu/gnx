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

/* Test the functions in the module src/io.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gnx.h>

#include "routine.h"

/**************************************************************************
 * prototypes of helper functions
 *************************************************************************/

/* read graph */
static void read_advogato(void);
static void read_advogato_ignore_weight(void);
static void read_comment(void);
static void read_error(void);
static void read_from_directory(void);
static void read_non_existing_file(void);
static void read_not_available(void);
static void read_valid_graphs(void);

/**************************************************************************
 * read graph from file
 *************************************************************************/

static void
read(void)
{
    read_advogato();
    read_advogato_ignore_weight();
    read_comment();
    read_error();
    read_from_directory();
    read_non_existing_file();
    read_not_available();
    read_valid_graphs();
}

/* Read in a graph of the Advogato trust network.
 */
static void
read_advogato(void)
{
    GnxGraph *graph;

    graph = gnx_read("data/network/advogato.csv",
                     GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(graph);
    assert(gnx_is_weighted(graph));
    assert(14018 == graph->total_nodes);
    assert(51482 == graph->total_edges);

    gnx_destroy(graph);
}

/* If the data file has edge weights, we can ignore the weights by specifying
 * that we want an unweighted version of the graph in the data file.
 */
static void
read_advogato_ignore_weight(void)
{
    GnxGraph *graph;

    graph = gnx_read("data/network/advogato.csv",
                     GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(!gnx_is_weighted(graph));
    assert(14018 == graph->total_nodes);
    assert(51482 == graph->total_edges);

    gnx_destroy(graph);
}

/* Read a graph file that has comments.
 */
static void
read_comment(void)
{
    GnxGraph *graph;
    const unsigned int u = 1;
    const unsigned int v = 42;

    graph = gnx_read("data/io/graph_comment.csv",
                     GNX_DIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(gnx_has_edge(graph, &u, &v));

    gnx_destroy(graph);
}

/* Test that the function gnx_read() correctly handles various types of errors.
 */
static void
read_error(void)
{
    GnxGraph *graph;

    /**********************************************************************
     * nodes
     *********************************************************************/

    graph = gnx_read("data/io/graph_empty.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_blank_line.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_node_minus_zero.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_node_plus_zero.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_node_repeat.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_no_comma.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_multiple_edges.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_negative_node_id.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_non_digit.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);


    /**********************************************************************
     * edge weight
     *********************************************************************/

    graph = gnx_read("data/io/graph_no_weight.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_no_weight_no_comma.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_non_digit.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_two_periods.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_multiple_minus.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_plus_zero.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_leading_space.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_leading_tab.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);


    /**********************************************************************
     * overflow
     *********************************************************************/

    graph = gnx_read("data/io/graph_overflow_int.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_overflow_long.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_overflow.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);

    graph = gnx_read("data/io/graph_weight_underflow.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);
}

/* A graph file cannot be a directory.
 */
static void
read_from_directory(void)
{
    char *dirname;
    GnxGraph *graph;

    dirname = random_template();
    dirname = g_mkdtemp_full(dirname, S_IRWXU);
    assert(dirname);

    graph = gnx_read(dirname, GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);
    assert(EISDIR == errno);

    assert(0 == g_rmdir(dirname));
    free(dirname);
}

/* Cannot read from a file that does not exist.
 */
static void
read_non_existing_file(void)
{
    GnxGraph *graph;

    graph = gnx_read("data/io/phantom.csv",
                     GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);
    assert(ENOENT == errno);
}

/* Cannot read from a file if we do not have read permission.
 */
static void
read_not_available(void)
{
    char *name;
    FILE *f;
    GnxGraph *graph;

    /* Create a temporary file with write-only permission. */
    name = random_template();
    f = fopen(name, "w");
    assert(f);
    fclose(f);
    assert(0 == g_chmod(name, S_IWUSR));

    graph = gnx_read(name, GNX_DIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(!graph);
    assert(EIO == errno);

    assert(0 == g_remove(name));
    free(name);
}

/* Read in various graphs.
 */
static void
read_valid_graphs(void)
{
    GnxGraph *graph;
    const unsigned int largeid = 1048576;  /* 2^20 */

    graph = gnx_read("data/io/graph_undirected.csv",
                     GNX_UNDIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(7 == graph->total_nodes);
    assert(4 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_directed.csv",
                     GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(7 == graph->total_nodes);
    assert(5 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_node.csv",
                     GNX_DIRECTED, GNX_NO_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(1 == graph->total_nodes);
    assert(0 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_large_node_id.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    assert(gnx_has_node(graph, &largeid));
    gnx_destroy(graph);


    /* edge weights */

    graph = gnx_read("data/io/graph_ignore_weight.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_ignore_weight_comma.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_UNWEIGHTED);
    assert(graph);
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_zero_weight.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(graph);
    assert(gnx_is_weighted(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_weight_minus_zero.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(graph);
    assert(gnx_is_weighted(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_positive_weight.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(graph);
    assert(gnx_is_weighted(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);

    graph = gnx_read("data/io/graph_negative_weight.csv",
                     GNX_UNDIRECTED, GNX_SELFLOOP, GNX_WEIGHTED);
    assert(graph);
    assert(gnx_is_weighted(graph));
    assert(2 == graph->total_nodes);
    assert(1 == graph->total_edges);
    gnx_destroy(graph);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/io/read", read);

    return g_test_run();
}
