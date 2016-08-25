# gnx

gnx is a C library of algorithms for graphs and networks.

## Prerequisites

gnx requires the following libraries:

1. glib >= 2.44.1
2. gmp >= 6.1.0
3. mpfr >= 3.1.0

## Build and test

The directory `util/` contains various sample scripts to automate the
build and testing of the gnx library.  However, if you prefer to
manually build and test gnx, the process is usually as follows:

1. Generate the `configure` script: `./autogen.sh`.
2. Configure the build: `./configure`.  You can pass various options
   to the script.  To see the available options, pass the option
   `--help` to the script.
3. Build the gnx library: `make`.  Refer to the script `util/gxbuild`
   for a way to automate the build of gnx.
4. Generate the HTML version of the documentation: `make html`.  Refer
   to the script `util/gxdoc` for a way to automatically generate the
   documentation of gnx.
5. Run the test suite: `make check`.  Refer to the script
   `util/gxtest` for a way to automatically configure, build, and test
   the gnx library.

## Developers

This section is for people who will develop or maintain the library.
In addition to the prerequisites listed above, you will also require
the following:

1. Recent versions of GNU autotools, in particular autoconf, automake,
   and libtool.  The minimum required versions of GNU autotools should
   be documented in the file `configure.ac`.  GNU autotools are
   required to configure the build system.  Once the GNU autotools are
   in place, the build system can be bootstrapped by running the
   script `autogen.sh`, which will run the GNU autotools to generate a
   `configure` script and other files that are required for compiling
   the library.
2. A recent version of doxygen.  Doxygen is required to build the
   documentation of the library.  After the library is built, run the
   command `make html` to generate the HTML version of the
   documentation.
