SFLib
=====

RISC OS Wimp Libraries for C.


Introduction
------------

SFLib is a set of C libraries for use when developing RISC OS software. Supplied mainly to allow my other software to compile, they can also be used in their own right as part of software licensed under the EUPL or one of its "compatible licences" (such as the GPL v2).

As supplied, the library is intended to be built into an ELF binary using the GCCSDK. When built, it assembles a collection of files in the norcroft folder which can be used to build an AOF binary.

Documentation for the libraries is provided within the header and source files, as well as in the form of output from Doxygen within the HTML folder.


Installation
------------

To install and use SFLib, it will be necessary to have suitable Linux system with a working installation of the [GCCSDK](http://www.riscos.info/index.php/GCCSDK).

It will also be necessary to ensure that the `SFTOOLS_BIN` and `$SFTOOLS_MAKE` variables are set to a suitable location within the current environment. For example

	export SFTOOLS_BIN=/home/steve/sftools/bin
	export SFTOOLS_MAKE=/home/steve/sftools/make

where the path is changed to suit your local settings and installation requirements. Finally, you will also need to have installed the Shared Makefiles, ManTools, PackTools and FlexLib.

To install SFLib, use

	make install

from the root folder of the project, which will compile the code copy the necessary files in to the GCCSDK environment.

If
	make documentation

is used, Docygen will be used to generate documentation within the manual folder.


Licence
-------

SFLib is licensed under the EUPL, Version 1.2 only (the "Licence"); you may not use this work except in compliance with the Licence.

You may obtain a copy of the Licence at <http://joinup.ec.europa.eu/software/page/eupl>.

Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on an "**as is**"; basis, **without warranties or conditions of any kind**, either express or implied.

See the Licence for the specific language governing permissions and limitations under the Licence.