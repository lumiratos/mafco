#!/bin/bash

# Clean binary files
make clean -f Makefile.linux
make clean -f Makefile.win32
make clean -f Makefile.win64

# Create Linux/Mac binaries
make -f Makefile.linux

# Create Windows 32-bits binaries
make -f Makefile.win32

# Create Windows 64-bits binaries
make -f Makefile.win64

echo "All done!"

