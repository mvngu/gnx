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

#ifndef GNX_UTIL_H
#define GNX_UTIL_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* comparing floating-point numbers */
int gnx_double_cmp_eq(const double *a,
                      const double *b);
int gnx_double_cmp_le(const double *a,
                      const double *b);

/* graph functions */
void gnx_undirected_edge_order(const unsigned int *u,
                               const unsigned int *v,
                               unsigned int *a,
                               unsigned int *b);

#endif  /* GNX_UTIL_H */
