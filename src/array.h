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

#ifndef GNX_ARRAY_H
#define GNX_ARRAY_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* create and destroy */
void gnx_destroy_array(GnxArray *array);
GnxArray* gnx_init_array(void);
GnxArray* gnx_init_array_full(const unsigned int *capacity,
                              const GnxBool destroy);

/* insert and remove */
int gnx_array_append(GnxArray *array,
                     int *elem);
int gnx_array_delete(GnxArray *array,
                     const unsigned int *i);
int gnx_array_delete_tail(GnxArray *array);

/* query */
int gnx_array_has(const GnxArray *array,
                  const int *elem);

#endif  /* GNX_ARRAY_H */
