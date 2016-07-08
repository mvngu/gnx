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

#ifndef GNX_CONSTANT_H
#define GNX_CONSTANT_H

/**************************************************************************
 * singular #define
 *************************************************************************/

/* The number of allocations that is required to initialize an array.  Any
 * non-negative integer below this number would result in an out-of-memory
 * error.
 */
#define GNX_ALLOC_ARRAY_SIZE (2)

/* The number of allocations that is required to initialize a bucket for a set
 * or dictionary.  Any non-negative integer below this number would result in
 * an out-of-memory error.
 */
#define GNX_ALLOC_BUCKET_SIZE (2)

#endif  /* GNX_CONSTANT_H */
