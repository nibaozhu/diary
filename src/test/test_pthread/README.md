A.out
===============

#### Create the build directory

    ~/test_pthread$ mkdir build
    ~/test_pthread$ cd build

#### Select build type

Selecting a *release* version (no debugging symbols, messages, enable some
optimizations, etc):

    ~/test_pthread/build$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/ ../

If you'd like to enable optimiations but still use a debugger, use this instead:

    ~/test_pthread/build$ cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/tmp/ ../

To disable optimizations and build a more debugging-friendly version:

    ~/test_pthread/build$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/tmp/ ../

#### Build A.out

    ~/test_pthread/build$ make

This will generate a few binaries:

 - `a.out`:

