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

#ifndef GNX_BASE_H
#define GNX_BASE_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* create and destroy */
void gnx_destroy(GnxGraph *graph);
GnxGraph* gnx_new(void);
GnxGraph* gnx_new_full(const GnxBool directed,
                       const GnxBool selfloop,
                       const GnxBool weighted);

/* query the graph properties */
int gnx_allows_selfloop(const GnxGraph *graph);
unsigned int gnx_degree(const GnxGraph *graph,
                        const unsigned int *v);
double gnx_edge_weight(const GnxGraph *graph,
                       const unsigned int *u,
                       const unsigned int *v);
int gnx_has_edge(const GnxGraph *graph,
                 const unsigned int *u,
                 const unsigned int *v);
int gnx_has_node(const GnxGraph *graph,
                 const unsigned int *v);
unsigned int gnx_indegree(const GnxGraph *graph,
                          const unsigned int *v);
int gnx_is_directed(const GnxGraph *graph);
int gnx_is_weighted(const GnxGraph *graph);
unsigned int gnx_outdegree(const GnxGraph *graph,
                           const unsigned int *v);

/* modify the graph structure */
int gnx_add_edge(GnxGraph *graph,
                 const unsigned int *u,
                 const unsigned int *v);
int gnx_add_edgew(GnxGraph *graph,
                  const unsigned int *u,
                  const unsigned int *v,
                  const double *w);
int gnx_add_node(GnxGraph *graph,
                 const unsigned int *v);
int gnx_delete_edge(GnxGraph *graph,
                    const unsigned int *u,
                    const unsigned int *v);
int gnx_delete_node(GnxGraph *graph,
                    const unsigned int *v);

#endif  /* GNX_BASE_H */
