# Compilation Instructions for Syft using CMake

These instructions were tested on Ubuntu 24.04.
They also work on RHEL 8, and should work on most linux distributions (with minor adjustments).
Before installing, please make sure that g++, libtool, automake and cmake are installed, otherwise install them using ``apt``.

On Ubuntu 24.04, you can also just run ``build.sh``.

==== Install CUDD ====

0.1 Make sure CUDD is installed. 
   CUDD can be found in the directory ``external/cudd``, from which the build instructions can be executed.
   Otherwise, it can also be cloned from the repository ``https://github.com/KavrakiLab/cudd.git``.
0.2 Install CUDD:
    ``./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=[install location]``
    ``sudo make install``

    If you get an error about aclocal, this might be due to either
    a. Not having automake:
       sudo apt-get install automake
    b. Needing to reconfigure, do this before configuring:
       autoreconf -i

   If you get an error about libtool missing, install it using ``apt install libtool-bin``

Please take note of the path where CUDD is installed (the prefix, if you set that variable), you might need it later for configuring Syft.

==== Install FLEX, BISON ====

0.3 Install flex and bison:
    sudo apt install flex bison

==== Install MONA ====

0.4 You probably want MONA if you are using Syft:
    sudo apt install mona

If you do not install MONA but, e.g., compile it from source and do not install it, make sure that the MONA executable is in your ``$PATH`` environment variable, as Syft calls MONA using that. We package the MONA version we used for testing in the ``external/``folder, but on ubuntu it is possible to just install it from the package manager.

==== INSTALL BOOST === 

0.5 Syft requires the Boost C++ libraries. 
   sudo apt install libboost-all-dev

To build and install Syft, start in the ``Syft``directory (this directory).

==== Install Syft ====

1. Make build folder so your directory is not flooded with build files:

   mkdir build && cd build

2. Run CMake to generate the makefile:

   cmake .. 

You might have to hint the CUDD location to CMake, if CUDD is not found. 
To do this, run cmake with the additional flag ``cmake .. -DCUDD_ROOT=path_to_cudd_install``.

3. Compile using the generated makefile:

   cmake --build .

# Running tests 

The tests can be executed (i.e. for ``ms0``) by running ``python3 runTests-cross.py mso`` from the ``scripts/runTests`` directory.
