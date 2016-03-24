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

# Makefile for GCC-based SF Lib C Library
#
# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.
#
# Set VERSION to build using a version number and not an SVN revision.

.PHONY: all library headers documentation clean release install backup

# Target Details

ARCHIVE ?= sflib
PACKAGE ?=
LIBRARY := libSFLib32.a
INCFOLDER := sflib
README := ReadMe,fff
LICENSE ?= Licence,fff
ADDITIONS ?=
EXTRASRC ?=

# Set up the source files.

MANSRC := Source
MANSRCNC := Norcroft
MANSPR := ManSprite
PKGCTRL ?= Control

OBJS := colpick.o config.o dataxfer.o debug.o errors.o event.o		\
	general.o heap.o icons.o ihelp.o menus.o msgs.o resources.o	\
	stack.o tasks.o saveas.o string.o templates.o url.o windows.o

# Set up the various build directories.

SRCDIR := src
MENUDIR := menus
MANUAL := manual
OBJDIR := obj
OUTDIR := build
PKGDIR := package
HDRDIR := sflib
NORCROFTDIR := norcroft
STRDUP := strdup

# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include

# Build Tools

ECHO := echo
PRINTF := printf
MKDIR := mkdir -p
RM := rm -rf
CP := cp
CD := cd
CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)
AR := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*ar)
INSTALL := install
ZIP := $(GCCSDK_INSTALL_ENV)/bin/zip
MANTOOLS := $(SFTOOLS_BIN)/mantools
BINDHELP := $(SFTOOLS_BIN)/bindhelp
TEXTMERGE := $(SFTOOLS_BIN)/textmerge
MENUGEN := $(SFTOOLS_BIN)/menugen
TOKENIZE := $(SFTOOLS_BIN)/tokenize
GETPKGREV := $(SFTOOLS_BIN)/getpackagerev
MAKECONTROL := $(SFTOOLS_BIN)/makecontrol

# Build Flags

CCFLAGS := -mlibscl -mhard-float -static -mthrowback -Wall -O2 -fno-strict-aliasing
ZIPFLAGS := -x "*/.svn/*" -r -, -9
PKGZIPFLAGS := -x "*/.svn/*" -r -, -9
SRCZIPFLAGS := -x "*/.svn/*" -r -9
BUZIPFLAGS := -x "*/.svn/*" -r -9
BINDHELPFLAGS := -f -r -v
MENUGENFLAGS := -d -m
TOKENIZEFLAGS +=  -verbose -warn pV -swi $(SWIDEFS)

# Set up escape sequences for colouring terminal output.

COLOUR_ACTION := \033[1;32m
COLOUR_END := \033[0m

# The build date.

BUILD_DATE = $(shell date "+%d %b %Y")
HELP_DATE = $(shell date "+%-d %B %Y")
FILE_DATE = $(shell date "+%Y%m%d")

# Construct version or revision information.

ifeq ($(VERSION),)
  RELEASE = $(shell svnversion --no-newline)
  VERSION := r$(RELEASE)
  RELEASE := $(subst :,-,$(RELEASE))
  HELP_VERSION := ----
ifneq ($(PACKAGE),)
  PACKAGE := $(PACKAGE)Unstable
  PKG_VERSION = $(shell $(GETPKGREV) --index unstable --package $(PACKAGE) --revision $(VERSION))
endif
else
  RELEASE := $(subst .,,$(VERSION))
  HELP_VERSION := $(VERSION)
ifneq ($(PACKAGE),)
  PKG_VERSION = $(shell $(GETPKGREV) --index stable --package $(PACKAGE) --revision $(VERSION))
endif
endif

# The archives to assemble the release files, sources and backups in.
# If $(RELEASE) is set, then the file can be given a version number suffix.

ZIPFILE := $(ARCHIVE)$(RELEASE).zip
SRCZIPFILE := $(ARCHIVE)$(RELEASE)src.zip
PKGZIPFILE := $(PACKAGE)_$(PKG_VERSION).zip
BUZIPFILE := $(ARCHIVE)$(FILE_DATE).zip

# Provide a means of reporting progress.

show-stage = $(PRINTF) "$(COLOUR_ACTION)*%18s: %s$(COLOUR_END)\n" "$(1)" "$(2)"

# Inform the user what we're about to do.

$(info *** Building with version $(VERSION) ($(RELEASE)) on date $(BUILD_DATE) ***)

# Build everything, but don't package it for release.

all: $(OUTDIR)/$(README) library norcroft

# Build the library and its supporting binary files.

library: headers $(OUTDIR)/$(LIBRARY)

# Copy all of the header files.

headers: $(OUTDIR)/$(HDRDIR) $(addprefix $(OUTDIR)/$(HDRDIR)/, $(OBJS:.o=.h))

# Assemble the Norcroft version of the library

norcroft: $(NORCROFTDIR)/c $(NORCROFTDIR)/h $(NORCROFTDIR)/$(README) $(addprefix $(NORCROFTDIR)/c/, $(OBJS:.o=) $(STRDUP)) $(addprefix $(NORCROFTDIR)/h/, $(OBJS:.o=) $(STRDUP))

$(NORCROFTDIR)/c/%: $(SRCDIR)/%.c
	@$(call show-stage,COPY NORCROFT,$@)
	@$(MKDIR) $(@D)
	@$(CP) $< $@

$(NORCROFTDIR)/h/%: $(SRCDIR)/%.h
	@$(call show-stage,COPY NORCROFT,$@)
	@$(MKDIR) $(@D)
	@$(CP) $< $@

# Build the complete Library file from the object files.

OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

$(OUTDIR)/$(LIBRARY): $(OBJS)
	@$(call show-stage,ARCHIVING,$(OUTDIR)/$(LIBRARY))
	@$(AR) -rcuv $(OUTDIR)/$(LIBRARY) $(OBJS)

# Build the object files, and identify their dependencies.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(EXTRASRCPREREQ)
	@$(call show-stage,COMPILING,$<)
	@$(MKDIR) $(@D)
	@$(CC) -c $(CCFLAGS) -D'BUILD_VERSION="$(VERSION)"' -D'BUILD_DATE="$(BUILD_DATE)"' $(INCLUDES) $< -o $@
	@$(CC) -MM $(CCFLAGS) -D'BUILD_VERSION="$(VERSION)"' -D'BUILD_DATE="$(BUILD_DATE)"' $(INCLUDES) $< > $(@:.o=.d)
	@mv -f $(@:.o=.d) $(@:.o=.d).tmp
	@sed -e 's|.*:|$@:|' < $(@:.o=.d).tmp > $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp

# Update the headers

$(OUTDIR)/$(HDRDIR)/%.h: $(SRCDIR)/%.h
	@$(call show-stage,COPY HEADER,$@)
	@$(MKDIR) $(@D)
	@$(CP) $< $@

# Build the documentation

documentation:
	doxygen $(MANUAL)/Doxyfile

$(OUTDIR)/$(README): $(MANUAL)/$(MANSRC)
	@$(call show-stage,TEXT MANUAL,$(OUTDIR)/$(README))
	@$(MANTOOLS) -MTEXT -I$(MANUAL)/$(MANSRC) -O$(OUTDIR)/$(README) \
			-D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'

$(NORCROFTDIR)/$(README): $(MANUAL)/$(MANSRCNC)
	@$(call show-stage,TEXT MANUAL,$(NORCROFTDIR)/$(README))
	@$(MANTOOLS) -MTEXT -I$(MANUAL)/$(MANSRCNC) -O$(NORCROFTDIR)/$(README) \
			-D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'


# Build the release Zip file.

release: clean all documentation
	@$(call show-stage,ZIPPING RELEASE,$(ZIPFILE))
	@$(RM) ../$(ZIPFILE)
	@(cd $(OUTDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) $(LIBRARY) $(HDRDIR) $(README) $(LICENSE) $(ADDITIONS))
	@(cd $(MANUAL) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) html)
ifneq ($(PACKAGE),)
	@$(call show-stage,ZIPPING PACKAGE,$(PKGZIPFILE))
	@$(RM) ../$(PKGZIPFILE)
	@$(RM) $(OUTDIR)/package/Apps/Misc/*
	@($(CD) $(OUTDIR) ; rsync -av --exclude=*.svn* $(APP) package/Apps/Misc/ )
	@$(MAKECONTROL) --template $(PKGDIR)/$(PKGCTRL) --control $(OUTDIR)/package/RiscPkg/Control --version $(PKG_VERSION)
	@($(CD) $(OUTDIR)/package ; $(ZIP) $(PKGZIPFLAGS) ../../../$(PKGZIPFILE) Apps RiscPkg Sprites SysVars)
endif
	@$(call show-stage,ZIPPING SOURCE,$(SRCZIPFILE))
	@$(RM) ../$(SRCZIPFILE)
	@$(ZIP) $(SRCZIPFLAGS) ../$(SRCZIPFILE) Makefile $(OUTDIR) $(NORCROFTDIR) $(SRCDIR) $(MANUAL)/$(MANSRC) $(MANUAL)/$(MANSRCNC) $(MANUAL)/Doxyfile $(EXTRASRC)


# Build a backup Zip file

backup:
	@$(call show-stage,ZIPPING BACKUP,$(BUZIPFILE))
	@$(RM) ../$(BUZIPFILE)
	@$(ZIP) $(BUZIPFLAGS) ../$(BUZIPFILE) *


# Install the finished version in the GCCSDK, ready for use.

install: clean all
	@$(call show-stage,INSTALLING,$(GCCSDK_INSTALL_ENV))
	@$(RM) $(GCCSDK_INSTALL_ENV)/lib/$(LIBRARY)
	@$(RM) $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER)
	@$(MKDIR) $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER)
	@$(INSTALL) -t $(GCCSDK_INSTALL_ENV)/lib $(OUTDIR)/$(LIBRARY)
	@$(INSTALL) -t $(GCCSDK_INSTALL_ENV)/include/$(INCFOLDER) $(SRCDIR)/*.h


# Clean targets

clean::
	@$(call show-stage,CLEANING,)
	@$(RM) $(OBJDIR)/*
	@$(RM) $(OUTDIR)/$(LIBRARY)
	@$(RM) $(OUTDIR)/$(HDRDIR)/*
	@$(RM) $(OUTDIR)/$(README)
	@$(RM) $(NORCROFTDIR)/c/*
	@$(RM) $(NORCROFTDIR)/h/*
	@$(RM) $(NORCROFTDIR)/$(README)

