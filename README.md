# MAFCO #
MAFCO is a compression tool for MAF files.


## INSTALLATION ##
In order to compile the source code, you will need to install a GCC compiler on a Unix platform (Linux or OS X). If you are using Windows, it will be easy to use the pre-compiled binaries that are in folders "win32" and "win64".

### Linux ###
For Linux users, install the build-essentials package which contains GCC and other utilities in order to be able to compile the source code. To install the build-essentials package type:
<pre>sudo apt-get install build-essential</pre>
After that you only need to type:
<pre>make -f Makefile.linux</pre>
to create the binaries "MAFCOenc" (encoder) and "MAFCOdec" (decoder).

### OS X ###
For OS X users, it depends on which Xcode version is installed. For the most recent versions, you will need to install the "Command Line Tool" in order to have the "make" utility. It seems that the "Command Line Tools" are not installed by default anymore when you install Xcode. In order to install them, open Xcode, go to Preferences -> Downloads -> Components -> Command Line Tools. This also should install a GCC compiler as well. If you want a recent compiler you can install it using Homebrew by typing the following command in a Terminal:
<pre>brew install gcc48</pre>
After that, we need to make sure that the "CC" variable in the "Makefile.osx" file is linked to the GCC previously installed. The most recent versions of XCode come with a modified version of GCC known as LLVM. This tool was not tested using LLVM so it will probably not work if you try to compile the code using it. In order to generate the binaries just type:
<pre>make -f Makefile.osx</pre>
to create the binaries "MAFCOenc" (encoder) and "MAFCOdec" (decoder).

### Windows ###
The source code of MAFCO is not prepared to be compiled in Windows without using a cross-compiler. The main reason to use a cross compiler instead of a classic compiler is due to some functions that have different names in Windows (namely regarding the pthread library). You can get the Windows binaries in a Linux environment after installing the cross-compiler [MinGW-w64](http://mingw-w64.sourceforge.net). After installing MinGW-w64, just type:
<pre>make -f Makefile.win32</pre>
to get the "MAFCOenc32.exe" (encoder) "MAFCOdec32.exe" (decoder) executables (32-bits architecture) and for the 64-bits architecture just type:
<pre>make -f Makefile.win64</pre> 
to get the "MAFCOenc64.exe" (encoder) and "MAFCOdec64.exe" (decoder) executables.
If you are not able to get the executables, just use the precompiled ones available in "win32" and "win64" folders. 
Because of the pthread library, the DLL files that are inside the "DLLs" folder must be in the same location of the executables in order to be able to run the encoder and the decoder. There are two DLL files. One for 32-bits and the other one for 64-bits operating system.

## Usage ##
### Encoding ###
The MAFCOenc/MAFCOenc32.exe/MAFCOenc64.exe programs have a very extensive interface because there are a wide variety of parameters that can be defined by the user. In the following you can find a description with the most relevant parameters available.

<pre>Usage: MAFCOenc [Options] ... [MAF File]</pre>

The most relevant options are:
<table align="center">
        <tr> 
          <td width="25%">-o [encodedFile]</td> 
          <td width="75%">If present, it writes the encoded data into file "encodedFile". If not present, the output file name is the same as the input file name with ".enc" appended.</td>
        </tr>
        <tr> 
          <td width="25%">-O [temporaryDir]</td> 
          <td width="75%">In the case of not having enough disk space in the current directory, this option allows the encoder to use the path specified by "temporaryDir" to create the temporary files that are a result of the splitting process.</td>
        </tr>
         <tr> 
          <td width="25%">-nt [nThreads]</td> 
          <td width="75%">Maximum number of threads during encoding. The user can define the number of parallel processes but the encoder can use a lower number of parallel processes due to the -ng flag. For instance, if the user specified a higher number of threads when compared to the number specified by the -ng flag, the encoder will set the number of threads to the number defined in the parameter ("nThreads" &le; "nGOBs"). If this flag is not present, the number of threads is by default 4 (equal to the number of Group Of Blocks in the -ng flag).</td>
        </tr>
        <tr> 
          <td width="25%">-ng [nGOBs]</td> 
          <td width="75%">The number of parts in which the MAF file is divided. Each part is then encoded independently from the other parts. Larger number of parts makes possible the parallelization increasing hability (see also the -nt flag). However, if these parts happen to be too small, the compression rate may decrease. By default, the encoder will split the input MAF file into 4 pieces (Group of Blocks).</td>
        </tr>
        <tr> 
          <td width="25%">-t [template]</td> 
          <td width="75%">When present it will define the template that will be used to encode the 2D DNA alignments. MAFCO provides a set of 5 templates that are represented by the letters 'A'-'E'. By default, the encoder uses the template 'C' with depth 10 as next:
          <!-- <p><a href="template.png" target="_blank"><img src="template.png" alt="Screenshot" style="max-width:20%;"></a></p> -->
          <p align="center"><a href="template.png" target="_blank"><img src="template.png" alt="Screenshot" width="200"></a></p>
          </td>
        </tr>
        <tr> 
          <td width="25%">-sm [ModelOrder]</td> 
          <td width="75%">When specified, will indicate the model order to use in the template indicated by the -t flag. By default the model order is 10 for template 'C'.</td>
        </tr>
  </table>

### Decoding ###
The MAFCOdec/MAFCOdec32.exe/MAFCOdec64.exe programs have the following interface:

<pre>Usage: MAFCOdec [Options] ... [Encoded MAF File]</pre>

The most relevant options are:
<table align="center">
    <tr> 
      <td width="25%">-o [decodedFile]</td> 
        <td width="75%">If present, it writes the decoded data into file "decodedFile". If not present, the output file name is the same as the input file name with ".dec" appended.</td>
    </tr>
    <tr> 
      <td width="25%">-O [temporaryDir]</td> 
        <td width="75%">In the case of not having enough disk space in the current directory, this option allows the decoder to use the path specified by "temporaryDir" to create the temporary files that are a result of the decoding process.</td>
    </tr>
    <tr> 
      <td width="25%">-nt [nThreads]</td> 
      <td width="75%">Maximum number of threads during decoding. The decoder can use a different number of parallel processes that were used by the encoder. This allows modest machines to be able to decompress data that was encoded in a powerfull computer with high parallel processing capability, using a single thread.</td>
    </tr>
    <tr> 
      <td width="25%">-ng [Gi:Gj]<br> -ng [Gi]</td> 
      <td width="75%">If present, this option allows the user to only decode a range [Gi - Gj] of GOBs (Group of Blocks) or a single one [Gi]. This is only possible if in the encoder the MAF file was splitted in several GOBs. By default it decodes all the GOBs (the entire MAF file).</td>
    </tr>
  </table>
      
### Examples ###
In the following, we will show some examples of how to use the MAFCO tool in a linux environment. It is important to emphasize that in order to be able to split the MAF file in several pieces, the encoder needs an input file in RAW format (no compression). 

Using the default parameters we can encode a MAF file by typing:
<pre>$ MAFCOenc chrM-multiz28way.maf</pre>
This will create the encoded file "chrM-multiz28way.maf.enc" that can be decoded using the following command:
<pre>$ MAFCOdec chrM-multiz28way.maf.enc</pre>
The decoder will create the file "chrM-multiz28way.maf.dec" that should be the same as the original file "chrM-multiz28way.maf". You can verify that by using the _diff_ or _cmp_ command:
<pre>$ cmp chrM-multiz28way.maf chrM-multiz28way.maf.dec
$ diff chrM-multiz28way.maf chrM-multiz28way.maf.dec</pre>

Lets say that you want to encode the MAF file using only 2 threads, splitting the original file into 8 GOBs (parts), using the 'C' template with order 8, and the output file should be "encodedFile.dat". In order to do that, you need to type:
<pre>$ MAFCOenc -nt 2 -ng 8 -t C -sm 8 -o encodedFile.dat chrM-multiz28way.maf</pre>
Now if you want to decode only the first 4 GOBs of the "encodedFile.dat", using a single thread and to put the decoded data in file "first4GOBsDecoded.maf" just type:
<pre>$ MAFCOdec -nt 1 -ng 1:4 -o first4GOBsDecoded.maf encodedFile.dat </pre>

## Issues ##
At the time, there are no relevant issues detected but if you find one please let me know using the [Issues link](https://github.com/lumiratos/mafco/issues) at GitHub.


## Cite ##
If you use this software, please cite the follwing publication: 
* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Diogo Pratas](http://sweet.ua.pt/pratas), and [Armando J. Pinho](http://sweet.ua.pt/ap), ["MAFCO: a compression tool for MAF files"](http://dx.doi.org/10.1371/journal.pone.0116082), [PLoS One](http://www.plosone.org/), volume 10, number 3, page e0116082, March 2015.

## Data sets ##
Some data set that can be used for evaluate this tool.
* [multiz28way](http://hgdownload-test.cse.ucsc.edu/goldenPath/hg18/multiz28way)
* [multiz28wayB](http://hgdownload-test.cse.ucsc.edu/goldenPath/hg18/multiz28way/multiz28wayAnno.tar.gz)
* [multiz46way](http://hgdownload-test.cse.ucsc.edu/goldenPath/hg19/multiz46way)
* [multiz100way](http://hgdownload-test.cse.ucsc.edu/goldenPath/hg19/multiz100way)

## Copyright ##
Copyright (c) 2014 Luís M. O. Matos. See [LICENSE.txt](https://github.com/lumiratos/saco/blob/master/LICENSE.txt) for further details.
