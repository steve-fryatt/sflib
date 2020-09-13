# Copyright 2010-2016, Stephen Fryatt (info@stevefryatt.org.uk)
#
# This file is part of SFLib:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.2 only (the "Licence");
# You may not use this work except in compliance with the
# Licence.
#
# You may obtain a copy of the Licence at:
#
#   http://joinup.ec.europa.eu/software/page/eupl
#
# Unless required by applicable law or agreed to in
# writing, software distributed under the Licence is
# distributed on an "AS IS" basis, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the Licence for the specific language governing
# permissions and limitations under the Licence.

# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.

# Target Details

ARCHIVE := sflib

LIBRARY := libSFLib32.a

INCFOLDER := sflib
HDRDIR := sflib

OBJS := colpick.o config.o dataxfer.o debug.o errors.o event.o		\
	general.o heap.o icons.o ihelp.o menus.o msgs.o resources.o	\
	stack.o tasks.o saveas.o string.o templates.o url.o windows.o

include $(SFTOOLS_MAKE)/CLib

