# Test suite

Each time you update any `.c` file in this directory, make sure you do
the following:

1. Update the script `gentest.sh.in` as necessary.
1. Update as necessary any script that `gentest.sh.in` calls.  The
   callees are responsible for generating further test code and
   embedding the code into the various test modules.

Ensure that you do not check in any test code that is generated by
`gentest.sh.in`.