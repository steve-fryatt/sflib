# Makefile for SFLib

COMPONENT = SFLib

TARGET = SFLib

OBJS = colpick config dataxfer debug errors event general   \
       heap icons ihelp menus msgs resources saveas stack   \
       strdup string tasks templates url windows

CINCLUDES = -IC:,OSLib:

# Don't warn of non-ANSII includes in the <> path.
CFLAGS = -Wp

include CLibrary

# Include function names in the binary.
C_NO_FNAMES =

# Dynamic dependencies:
