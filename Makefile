# Makefile for SFLib
#
# Copyright 2010, Stephen Fryatt
#
# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.

.PHONY: all clean documentation release install backup


# The archive to assemble the release files in.  If $(RELEASE) is set, then the file can be given
# a standard version number suffix.

ZIPFILE := sflib$(RELEASE).zip
BUZIPFILE := sflib$(shell date "+%Y%m%d").zip


# The build date.

BUILD_DATE := $(shell date "+%d %b %Y")


# Build Tools

CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)
AR := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*ar)

MKDIR := mkdir
RM := rm -rf
CP := cp

ZIP := /home/steve/GCCSDK/env/bin/zip

SFBIN := /home/steve/GCCSDK/sfbin
SFLIB := /home/steve/GCCSDK/libs/SFLib

TEXTMAN := $(SFBIN)/textman
STRONGMAN := $(SFBIN)/strongman
HTMLMAN := $(SFBIN)/htmlman
DDFMAN := $(SFBIN)/ddfman
BINDHELP := $(SFBIN)/bindhelp
TEXTMERGE := $(SFBIN)/textmerge
MENUGEN := $(SFBIN)/menugen


# Build Flags

CCFLAGS := -mlibscl -mhard-float -mthrowback -Wall -O2 -fno-strict-aliasing
ZIPFLAGS := -x "*/.svn/*" -r -, -9
BUZIPFLAGS := -x "*/.svn/*" -r -9
BINDHELPFLAGS := -f -r -v
MENUGENFLAGS := -d


# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include -I$(GCCSDK_LIBS)/OSLib/ -I$(GCCSDK_LIBS)/FlexLib/


# Set up the various build directories.

SRCDIR := src
MANUAL := manual
OBJDIR := obj
OUTDIR := build


# Set up the named target files.

RUNIMAGE := libSFLib32.a
README := ReadMe


# Set up the source files.

MANSRC := Source
MANSPR := ManSprite

OBJS := colpick.o config.o debug.o errors.o event.o general.o heap.o	\
	icons.o menus.o msgs.o resources.o stack.o tasks.o string.o	\
	transfer.o url.o windows.o


# Build everything, but don't package it for release.

all: $(OUTDIR)/$(RUNIMAGE)
# Was all: documentation $(OUTDIR)/$(RUNIMAGE)

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


# Build the documentation

documentation:
	doxygen $(MANUAL)/Doxyfile

#documentation: $(OUTDIR)/$(README)
#
#$(OUTDIR)/$(README): $(MANUAL)/$(MANSRC)
#	$(TEXTMAN) $(MANUAL)/$(MANSRC) $(OUTDIR)/$(README)
#
#MakeCHelp.MakeCLibSH ADFS::Iyonix.$.C.Libraries.SFLib.sflib ADFS::Iyonix.$.!Boot.Resources.!Manuals.Root.c.SFLib -b


# Build the release Zip file.

release: clean all
	$(RM) ../$(ZIPFILE)
	(cd $(OUTDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) $(RUNIMAGE))
	(cd $(SRCDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) *.h)


# Build a backup Zip file

backup:
	$(RM) ../$(BUZIPFILE)
	$(ZIP) $(BUZIPFLAGS) ../$(BUZIPFILE) *


# Install the finished version in the GCCSDK, ready for use.

install: clean all
	$(RM) $(SFLIB)/*
	$(CP) $(OUTDIR)/$(RUNIMAGE) $(SFLIB)
	$(MKDIR) $(SFLIB)/sflib
	$(CP) $(SRCDIR)/*.h $(SFLIB)/sflib


# Clean targets

clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(OUTDIR)/$(RUNIMAGE)
#	$(RM) $(OUTDIR)/$(README)

