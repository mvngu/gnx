# Process this file with autoconf to produce a configure script.

# Define the project version so we only need to change it here.
m4_define([GNX_MAJOR_VERSION], [0])
m4_define([GNX_MINOR_VERSION], [0])
m4_define([GNX_VERSION], [GNX_MAJOR_VERSION.GNX_MINOR_VERSION])

AC_PREREQ([2.69])
AC_INIT([gnx], [GNX_VERSION])
AM_INIT_AUTOMAKE([foreign])
LT_PREREQ([2.4.2])
LT_INIT
AC_CONFIG_SRCDIR([src/gnx.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CPP
AC_PROG_INSTALL

# Use doxygen to generate documentation.
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([gnx], [doxygen.cfg], [doc])

# At present, the full test suite of gnx requires Python and that we are
# compiling with GCC.
#
# Python is not required for building gnx.  However, Python is required
# for generating further test code for the test suite.
AM_PATH_PYTHON([2.7], [HAVE_PYTHON=yes], [HAVE_PYTHON=no])
#
# By default, we disable the full test suite.  We can enable the full test
# suite here.
AC_ARG_ENABLE(
    [full-testsuite],
    [AS_HELP_STRING(
        [--enable-full-testsuite],
        [Check gnx with the full test suite.  This requires Python.])])
AS_IF(
    [test x"$enable_full_testsuite" = x"yes"],
    [ENABLE_FULL_TEST=yes])
#
# If we enable the full test suite, then the module src/alloc.c will be
# compiled as part of the gnx shared library.  The purpose of the module
# src/alloc.c is to enable us to test that gnx correctly handles scenarios
# where the runtime system runs out of memory.  By default, we do not install
# the corresponding header file src/alloc.h when we install gnx, thus
# src/alloc.h would not be part of the public interface.  If for whatever
# reason we really need to have src/alloc.h as part of the installed public
# interface, then we can enable this feature here.
AC_ARG_ENABLE(
    [install-alloc],
    [AS_HELP_STRING(
        [--enable-install-alloc],
        [Install src/alloc.h as part of the public interface.  This requires
         the option --enable-full-testsuite.])])
AS_IF(
    [test x"$enable_install_alloc" = x"yes"],
    [ENABLE_INSTALL_ALLOC=yes],
    [ENABLE_INSTALL_ALLOC=no])

# Use the glib testing framework to automate the test suite.
GLIB_TESTS

# Checks for libraries.
#
# glib
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(
    [GLIB],
    [glib-2.0 >= 2.44.1],
    [HAVE_GLIB=yes],
    [HAVE_GLIB=no])
AS_IF(
    [test x"$HAVE_GLIB" = x"no"],
    [AC_MSG_ERROR(
        [*** glib >= 2.44.1 library and headers not found ***])])
#
# gmp
AC_CHECK_LIB([gmp],
    [__gmpz_init],
    [LIBS="-lgmp $LIBS"],
    [AC_MSG_ERROR(
        [*** libgmp headers not found ***])])
# Check for a recent GMP
AC_MSG_CHECKING(for recent GMP)
AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE(
        [[
#include "gmp.h"
#if (__GNU_MP_VERSION*100 + __GNU_MP_VERSION_MINOR*10 + __GNU_MP_VERSION_PATCHLEVEL < 610)
# error "Minimal GMP version is 6.1.0"
error
#endif
        ]])],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)
     AC_MSG_ERROR([GMP version >= 6.1.0 required])
    ])
#
# mpfr
AC_MSG_CHECKING(for MPFR)
LIBS="-lmpfr $LIBS"
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
        [[#include "mpfr.h"]],
            [[mpfr_t x;  mpfr_init(x) ; mpfr_clear(x);]])],
        [AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)
         AC_MSG_ERROR([*** libmpfr headers not found])
    ])
# Check for a recent MPFR
AC_MSG_CHECKING(for recent MPFR)
AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE(
        [[
#include "mpfr.h"
#if (MPFR_VERSION < MPFR_VERSION_NUM (2,4,2))
# error "Minimal MPFR version is 2.4.2"
error
#endif
        ]])],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)
     AC_MSG_ERROR([MPFR version >= 2.4.2 required])
    ])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CONFIG_FILES(
    [Makefile
     build/Makefile
     data/gnx.pc
     doc/Makefile
     src/Makefile
     tests/gentest.sh
     tests/Makefile])

AC_OUTPUT

# Assume that we are building with GCC.  If Python is found and we enable the
# full test suite, then we generate further test code for the test suite.
testdir=tests
if test x"$GCC" = x"yes"; then
    if test x"$HAVE_PYTHON" = x"yes" && test x"$ENABLE_FULL_TEST" = x"yes"; then
        if test -f "$testdir"/gentest.sh; then
            cd "$testdir"                                                  \
            && "$SHELL" gentest.sh --installoc "$ENABLE_INSTALL_ALLOC"     \
            && cd ../
        fi
    fi
fi

echo \
"------------------------------------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: ${prefix}
Compiler: ${CC} ${CFLAGS} ${CPPFLAGS}
Libraries: ${GLIB_LIBS} ${LIBS}
Python: ${PYTHON}
Doxygen: ${DX_DOXYGEN}

------------------------------------------------------------------------------"
