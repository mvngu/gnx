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

/* Test the functions in the module src/util.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>

#include <glib.h>
#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* compare floating-point numbers */
static void compare_double_le(void);

/**************************************************************************
 * comparing floating-point numbers
 *************************************************************************/

static void
compare_double(void)
{
    compare_double_le();
}

/* Compare two floating-point numbers a and b for the relation a <= b. */
static void
compare_double_le(void)
{
    double a, b;
    unsigned int i;
    const double num[3] = {-100.0, 0.0, 100.0};
    const double low = num[0];
    const double mid = num[1];
    const double high = num[2];

    assert(gnx_double_cmp_le(&low, &mid));
    assert(gnx_double_cmp_le(&low, &high));
    assert(gnx_double_cmp_le(&mid, &high));

    i = (unsigned int)g_random_int_range(0, 3);
    assert(gnx_double_cmp_le(&num[i], &num[i]));

    a = g_random_double_range(low, mid);
    b = g_random_double_range(mid, high);
    assert(gnx_double_cmp_le(&a, &b));
    assert(gnx_double_cmp_le(&a, &a));
    assert(gnx_double_cmp_le(&b, &b));
    assert(!gnx_double_cmp_le(&b, &a));
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/util/compare-double", compare_double);

    return g_test_run();
}
