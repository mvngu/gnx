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

#ifndef GNX_SET_H
#define GNX_SET_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* create and destroy */
void gnx_destroy_set(GnxSet *set);
GnxSet* gnx_init_set(void);
GnxSet* gnx_init_set_full(const GnxBool destroy);

/* insert and remove */
int gnx_set_add(GnxSet *set,
                unsigned int *elem);
int gnx_set_delete(GnxSet *set,
                   const unsigned int *elem);

/* query */
unsigned int gnx_set_any(GnxSet *set);
unsigned int* gnx_set_has(const GnxSet *set,
                          const unsigned int *elem);

/* iterator */
void gnx_set_iter_init(GnxSetIter *iter,
                       GnxSet *set);
int gnx_set_iter_next(GnxSetIter *iter,
                      gnxptr *elem);

#endif  /* GNX_SET_H */
