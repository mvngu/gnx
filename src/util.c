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

#include <assert.h>

#include <mpfr.h>

#include "util.h"

/**
 * @file util.h
 * @brief Miscellaneous utility functions that do not belong anywhere else.
 */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_double_cmp(const double *a,
                            const double *b);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Compares two floating-point numbers.
 *
 * @param a A floating-point number.
 * @param b Another floating-point number.
 * @return A positive value if @f$a > b@f$, zero if @f$a = b@f$, and
 *         a negative value if @f$a < b@f$.
 */
static int
gnx_i_double_cmp(const double *a,
                 const double *b)
{
    int retval;
    mpfr_t ma, mb;

    g_return_val_if_fail(a, GNX_FAILURE);
    g_return_val_if_fail(b, GNX_FAILURE);

    mpfr_inits2(GNX_MPFR_PRECISION, ma, mb, (mpfr_ptr)NULL);

    (void)mpfr_set_d(ma, *a, MPFR_RNDN);
    (void)mpfr_set_d(mb, *b, MPFR_RNDN);
    retval = mpfr_cmp(ma, mb);

    mpfr_clears(ma, mb, (mpfr_ptr)NULL);
    mpfr_free_cache();

    return retval;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Determines whether one floating-point number is less than or
 *        equal to another floating-point number.
 *
 * @param a A floating-point number.
 * @param b Another floating-point number.
 * @return Nonzero if @a a compares less than or equal to @a b; zero otherwise.
 */
int
gnx_double_cmp_le(const double *a,
                  const double *b)
{
    int retval = gnx_i_double_cmp(a, b);

    return (retval <= 0) ? TRUE : FALSE;
}
