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

#ifndef GNX_DICT_H
#define GNX_DICT_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* create and destroy */
void gnx_destroy_dict(GnxDict *dict);
GnxDict* gnx_init_dict(void);
GnxDict* gnx_init_dict_full(const GnxBool free_key,
                            const GnxBool free_value);

/* add and remove */
int gnx_dict_add(GnxDict *dict,
                 unsigned int *key,
                 gnxptr value);
int gnx_dict_delete(GnxDict *dict,
                    const unsigned int *key);

/* query */
gnxptr gnx_dict_has(const GnxDict *dict,
                    const unsigned int *key);

/* iterator */
void gnx_dict_iter_init(GnxDictIter *iter,
                        GnxDict *dict);
int gnx_dict_iter_next(GnxDictIter *iter,
                       unsigned int *key,
                       gnxptr *value);

#endif  /* GNX_DICT_H */
