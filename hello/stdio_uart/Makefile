#
# Makefile for eCos tests
#

# Platform specific setups
include Make.params

# Simple build rules

.c.o:
	$(CC) -c $(ACTUAL_CFLAGS) -I$(PREFIX)/include $*.c

.o:
	$(CC) $(ACTUAL_LDFLAGS) -L$(PREFIX)/lib -Ttarget.ld $*.o -o $@

SRCS=hello.c
OBJS=${SRCS:%.c=%.o}
DST=hello

${DST}: ${OBJS}

