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

#ifndef GNX_SANITY_H
#define GNX_SANITY_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

void gnx_i_check(const GnxGraph *graph);
void gnx_i_check_array(const GnxArray *array);
void gnx_i_check_data_type(const GnxBool datatype);
void gnx_i_check_destroy_type(const GnxBool destroy);
void gnx_i_check_dict(const GnxDict *dict);
void gnx_i_check_heap(const GnxHeap *heap);
void gnx_i_check_node(const unsigned int *v);
void gnx_i_check_order(const GnxMethod order);
void gnx_i_check_properties(const GnxBool directed,
                            const GnxBool selfloop,
                            const GnxBool weighted);
void gnx_i_check_queue(const GnxQueue *queue);
void gnx_i_check_set(const GnxSet *set);
void gnx_i_check_stack(const GnxStack *stack);

#endif  /* GNX_SANITY_H */
