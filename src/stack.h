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

#ifndef GNX_STACK_H
#define GNX_STACK_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include "type.h"

/* create and destroy */
void gnx_destroy_stack(GnxStack *stack);
GnxStack* gnx_init_stack(void);
GnxStack* gnx_init_stack_full(const unsigned int *capacity,
                              const GnxBool destroy);

/* insert and delete */
unsigned int* gnx_stack_pop(GnxStack *stack);
int gnx_stack_push(GnxStack *stack,
                   unsigned int *elem);

/* query */
unsigned int* gnx_stack_peek(const GnxStack *stack);

#endif  /* GNX_STACK_H */
