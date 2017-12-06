dog
===============

#### Create the build directory

    ~/test_pthread$ mkdir build
    ~/test_pthread$ cd build

#### Select build type

Selecting a *release* version (no debugging symbols, messages, enable some
optimizations, etc):

    ~/test_pthread/build$ cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=Release

If you'd like to enable optimiations but still use a debugger, use this instead:

    ~/test_pthread/build$ cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=RelWithDebInfo

To disable optimizations and build a more debugging-friendly version:

    ~/test_pthread/build$ cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=Debug

#### Build dog

    ~/test_pthread/build$ make

This will generate a few binaries:

 - `dog`:

