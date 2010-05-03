# SFLib - makefile
#
# (C) Stephen Fryatt, 2003

# Flags

AR = $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*ar)
CC = $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)
CCFlags = -mlibscl -mhard-float -mthrowback -Wall -O1 -fno-strict-aliasing
Include = -I$(GCCSDK_INSTALL_ENV)/include -I$(GCCSDK_LIBS)/OSLib/ -I$(GCCSDK_LIBS)/SFLib/ -I$(GCCSDK_LIBS)/FlexLib/


# Final Target

OBJS = colpick.o config.o debug.o errors.o event.o general.o heap.o	\
	icons.o menus.o msgs.o resources.o stack.o tasks.o string.o	\
	transfer.o url.o windows.o

../libSFLib32.a: $(OBJS)
		$(AR) -rcuv ../libSFLib32.a $(OBJS)
#		libfile -c ^..oSFLib $(OBJS)

# Individual source files

colpick.o: colpick.h colpick.c
	$(CC) $(CCFlags) $(Include) -o colpick.o -c colpick.c

config.o: config.h config.c
	$(CC) $(CCFlags) $(Include) -o config.o -c config.c

debug.o: debug.h debug.c
	$(CC) $(CCFlags) $(Include) -o debug.o -c debug.c

errors.o: errors.h msgs.h errors.c
	$(CC) $(CCFlags) $(Include) -o errors.o -c errors.c

event.o: event.h event.c
	$(CC) $(CCFlags) $(Include) -o event.o -c event.c

general.o: general.h general.c
	$(CC) $(CCFlags) $(Include) -o general.o -c general.c

heap.o: heap.h heap.c
	$(CC) $(CCFlags) $(Include) -o heap.o -c heap.c

icons.o: icons.h icons.c
	$(CC) $(CCFlags) $(Include) -o icons.o -c icons.c

menus.o: menus.h menus.c
	$(CC) $(CCFlags) $(Include) -o menus.o -c menus.c

msgs.o: msgs.h msgs.c
	$(CC) $(CCFlags) $(Include) -o msgs.o -c msgs.c

resources.o: resources.h resources.c
	$(CC) $(CCFlags) $(Include) -o resources.o -c resources.c

string.o: string.h string.c
	$(CC) $(CCFlags) $(Include) -o string.o -c string.c

stack.o: stack.h stack.c
	$(CC) $(CCFlags) $(Include) -o stack.o -c stack.c

tasks.o: tasks.h errors.h types.h tasks.c
	$(CC) $(CCFlags) $(Include) -o tasks.o -c tasks.c

transfer.o: transfer.h transfer.c
	$(CC) $(CCFlags) $(Include) -o transfer.o -c transfer.c

url.o: url.h url.c
	$(CC) $(CCFlags) $(Include) -o url.o -c url.c

windows.o: windows.c windows.h types.h
	$(CC) $(CCFlags) $(Include) -o windows.o -c windows.c
