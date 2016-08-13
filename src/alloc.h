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

#ifndef GNX_ALLOC_H
#define GNX_ALLOC_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include <stddef.h>

/* wrappers for library functions */
void* __real_calloc(size_t nobject,
                    size_t size);
void* __real_malloc(size_t nbyte);
void* __real_realloc(void *ptr,
                     size_t new_size);
void* __wrap_calloc(size_t nobject,
                    size_t size);
void* __wrap_malloc(size_t nbyte);
void* __wrap_realloc(void *ptr,
                     size_t new_size);

/* modify the allocation limit */
void gnx_alloc_reset_limit(void);
void gnx_alloc_set_limit(int count);

#endif  /* GNX_ALLOC_H */
