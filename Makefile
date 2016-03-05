# Copyright 2010-2016, Stephen Fryatt (info@stevefryatt.org.uk)
#
# This file is part of SFLib:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.1 only (the "Licence");
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

.PHONY: all library headers documentation clean release install backup

# The build date.

BUILD_DATE := $(shell date "+%d %b %Y")
HELP_DATE := $(shell date "+%-d %B %Y")

# Construct version or revision information.

ifeq ($(VERSION),)
  RELEASE := $(shell svnversion --no-newline)
  VERSION := r$(RELEASE)
  RELEASE := $(subst :,-,$(RELEASE))
  HELP_VERSION := ----
else
  RELEASE := $(subst .,,$(VERSION))
  HELP_VERSION := $(VERSION)
endif

$(info Building with version $(VERSION) ($(RELEASE)) on date $(BUILD_DATE))

# The archive to assemble the release files in.  If $(RELEASE) is set, then the file can be given
# a standard version number suffix.

ZIPFILE := sflib$(RELEASE).zip
SRCZIPFILE := sflib$(RELEASE)src.zip
BUZIPFILE := sflib$(shell date "+%Y%m%d").zip

# Build Tools

CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)
AR := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*ar)

MKDIR := mkdir
RM := rm -rf
CP := cp
INSTALL := install

ZIP := $(GCCSDK_INSTALL_ENV)/bin/zip

MANTOOLS := $(SFTOOLS_BIN)/mantools
BINDHELP := $(SFTOOLS_BIN)/bindhelp
TEXTMERGE := $(SFTOOLS_BIN)/textmerge
MENUGEN := $(SFTOOLS_BIN)/menugen


# Build Flags

CCFLAGS := -mlibscl -mhard-float -static -mthrowback -Wall -O2 -fno-strict-aliasing
ZIPFLAGS := -x "*/.svn/*" -r -, -9
SRCZIPFLAGS := -x "*/.svn/*" -r -9
BUZIPFLAGS := -x "*/.svn/*" -r -9
BINDHELPFLAGS := -f -r -v
MENUGENFLAGS := -d


# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include


# Set up the various build directories.

SRCDIR := src
MANUAL := manual
OBJDIR := obj
OUTDIR := build
HDRDIR := sflib
NORCROFTDIR := norcroft
STRDUP := strdup


# Set up the named target files.

RUNIMAGE := libSFLib32.a
INCFOLDER := sflib
README := ReadMe,fff


# Set up the source files.

MANSRC := Source
MANSRCNC := Norcroft
MANSPR := ManSprite

OBJS := colpick.o config.o dataxfer.o debug.o errors.o event.o		\
	general.o heap.o icons.o ihelp.o menus.o msgs.o resources.o	\
	stack.o tasks.o saveas.o string.o templates.o url.o windows.o


# Build everything, but don't package it for release.

all: $(OUTDIR)/$(README) library norcroft
# Was all: documentation $(OUTDIR)/$(RUNIMAGE)

library: headers $(OUTDIR)/$(RUNIMAGE)

headers: $(OUTDIR)/$(HDRDIR) $(addprefix $(OUTDIR)/$(HDRDIR)/, $(OBJS:.o=.h))

# Assemble the Norcroft version of the library

norcroft: $(NORCROFTDIR)/c $(NORCROFTDIR)/h $(NORCROFTDIR)/$(README) $(addprefix $(NORCROFTDIR)/c/, $(OBJS:.o=) $(STRDUP)) $(addprefix $(NORCROFTDIR)/h/, $(OBJS:.o=) $(STRDUP))

$(NORCROFTDIR)/c/%: $(SRCDIR)/%.c
	$(CP) $< $@

$(NORCROFTDIR)/h/%: $(SRCDIR)/%.h
	$(CP) $< $@

$(NORCROFTDIR)/c:
	$(MKDIR) $(NORCROFTDIR)/c

$(NORCROFTDIR)/h:
	$(MKDIR) $(NORCROFTDIR)/h

# Build the complete !RunImage from the object files.

OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

$(OUTDIR)/$(RUNIMAGE): $(OBJS) $(OBJDIR)
	$(AR) -rcuv $(OUTDIR)/$(RUNIMAGE) $(OBJS)

# Create a folder to hold the object files.

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

# Build the object files, and identify their dependencies.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	@$(CC) -MM $(CCFLAGS) $(INCLUDES) $< > $(@:.o=.d)
	@mv -f $(@:.o=.d) $(@:.o=.d).tmp
	@sed -e 's|.*:|$@:|' < $(@:.o=.d).tmp > $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp

# Update the headers

$(OUTDIR)/$(HDRDIR):
	$(MKDIR) $(OUTDIR)/$(HDRDIR)

$(OUTDIR)/$(HDRDIR)/%.h: $(SRCDIR)/%.h
	$(CP) $< $@

# Build the documentation

documentation:
	doxygen $(MANUAL)/Doxyfile

$(OUTDIR)/$(README): $(MANUAL)/$(MANSRC)
	$(MANTOOLS) -MTEXT -I$(MANUAL)/$(MANSRC) -O$(OUTDIR)/$(README) -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'

$(NORCROFTDIR)/$(README): $(MANUAL)/$(MANSRCNC)
	$(MANTOOLS) -MTEXT -I$(MANUAL)/$(MANSRCNC) -O$(NORCROFTDIR)/$(README) -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'


# Build the release Zip file.

release: clean all documentation
	$(RM) ../$(ZIPFILE)
	(cd $(OUTDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) $(RUNIMAGE) $(HDRDIR) Licence,fff $(README))
	(cd $(MANUAL) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) html)
	$(RM) ../$(SRCZIPFILE)
	$(ZIP) $(SRCZIPFLAGS) ../$(SRCZIPFILE) $(OUTDIR) $(NORCROFTDIR) $(SRCDIR) $(MANUAL)/$(MANSRC) $(MANUAL)/$(MANSRCNC) $(MANUAL)/Doxyfile Makefile


# Build a backup Zip file

backup:
	$(RM) ../$(BUZIPFILE)
	$(ZIP) $(BUZIPFLAGS) ../$(BUZIPFILE) *


# Install the finished version in the GCCSDK, ready for use.

install: clean all
	$(RM) $(GCCSDK_INSTALL_ENV)/lib/$(RUNIMAGE)
	$(RM) $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER)
	$(MKDIR) $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER)
	$(INSTALL) -t $(GCCSDK_INSTALL_ENV)/lib $(OUTDIR)/$(RUNIMAGE)
	$(INSTALL) -t $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER) $(SRCDIR)/*.h


# Clean targets

clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(OUTDIR)/$(RUNIMAGE)
	$(RM) $(OUTDIR)/$(HDRDIR)/*
	$(RM) $(OUTDIR)/$(README)
	$(RM) $(NORCROFTDIR)/c/*
	$(RM) $(NORCROFTDIR)/h/*
	$(RM) $(NORCROFTDIR)/$(README)

