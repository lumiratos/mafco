# MAFCO #
MAFCO is a compression tool for MAF files.


## INSTALLATION ##
The source code was developed in ANSI C so you will need to have a GCC compiler installed in a Unix platform (Linux or OS X). If you are using Windows, it will be easy to use the pre-compiled binaries that are in folders "win32" and "win64".

### Linux ###
For Linux users, install the build-essentials package which contains GCC and other utilities in order to be able to compile the source code. To install the build-essentials package type
<pre>sudo apt-get install build-essential</pre>
After that you only need to type
<pre>make -f Makefile.linux</pre>
to create the binaries "MAFCOenc" (encoder) and "MAFCOdec" (decoder).

### OS X ###
For OS X users, it depends on which Xcode version is installed. For the most recent versions, you will need to install the "Command Line Tool" in order to have the "make" utility. It seems that those "Command Line Tools" are not installed by default anymore when you install Xcode. In order to install them, open Xcode, go to Preferences -> Downloads -> Components -> Command Line Tools. This also should install a GCC compiler as well but if you want a more recent compiler you can install it using Homebrew by typing the following command in a Terminal:
<pre>brew install gcc48</pre>
After that, we need to make sure that the "CC" variable in the "Makefile.osx" file is linked to the GCC previously installed. The most recent versions of XCode come with a modified version of GCC known as LLVM. This tool was not tested using LLVM so it will probably not work if you try to compile the code using it. In order to generate the binaries just type
<pre>make -f Makefile.osx</pre>
to create the binaries "MAFCOenc" (encoder) and "MAFCOdec" (decoder).

### Windows ###
The source code of MAFCO is not prepared to be compiled in Windows without using a cross-compiler. The main reason to use a cross compiler instead a classic compiler is due to some functions that have different names in Windows (namely regarding the pthread library). You can get the Windows binaries in a Linux environment after installing the cross-compiler [MinGW-w64](http://mingw-w64.sourceforge.net). After installing MinGW-w64, just type
<pre>make -f Makefile.win32</pre>
to get the "MAFCOenc32.exe" (encoder) "MAFCOdec32.exe" (decoder) executables (32-bits architecture) and for the 64-bits architecture just type
<pre>make -f Makefile.win64</pre> 
to get the "MAFCOenc64.exe" (encoder) "MAFCOdec64.exe" (decoder) executables.
If you are not able to get the executables, just use the precompiled ones available in "win32" and "win64" folders. 
Because of the pthread library, the DLL files that are inside the "DLLs" folder must be in the same location of the executables in order to be able to run the encoder and the decoder. There are two DLL files. One for 32-bits and the other one for 64-bits operating system.

## Issues ##
At the time, there are no relevant issues detected but if you find one please let me know using the [Issues link](https://github.com/lumiratos/mafco/issues) at GitHub.

<!---
## Cite ##
If you use this software, please cite the follwing publication: 

MAFCO: a compression tool for MAF data (BIOINFORMATICS)
-->

## Copyright ##
Copyright (c) 2014 Luís M. O. Matos. See LICENSE.txt for further details.
