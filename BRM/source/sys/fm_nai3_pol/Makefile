#
#
# Copyright (c) 2002, 2017, Oracle and/or its affiliates. All rights reserved.
#
# This material is the confidential property of Oracle Corporation
# or its licensors and may be used, reproduced, stored or transmitted
# only in accordance with a valid Oracle license or sublicense agreement.
#

OS=linux
PIN_VERSION=7.5

##########

PINDIR=../../..
INCDIR=$(PINDIR)/include

###########

CC_solaris = cc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC_linux = gcc
CC = $(CC_$(OS))

CFLAGS_solaris= -g
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS_linux= -fPIC
CFLAGS= $(CFLAGS_$(OS)) -DFLIST_HEAP

CPPFLAGS = -I$(INCDIR)


LDFLAGS_solaris= -G
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -brtl -lc
LDFLAGS_linux= -shared -L/usr/lib
LDFLAGS = $(LDFLAGS_$(OS))

SL_EXT_solaris= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a
SL_EXT_linux= so
SL_EXT= $(SL_EXT_$(OS))

OS_LIBS_solaris= -lm
OS_LIBS_hpux= -lsec -lm
OS_LIBS_hpux_ia64= -lm
OS_LIBS_aix= -lm
OS_LIBS_linux= -lm
OS_LIBS= $(OS_LIBS_$(OS))

####

INCFILES= $(INCDIR)/pcm.h \
        $(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h \
        $(INCDIR)/pin_flds.h

FILES= fm_nai_config.c \
        fm_nai_commit_customer.c \
        fm_nai_get_available_res.c \
				fm_nai_act_rate.c

OBJECTS= fm_nai_config.o \
        fm_nai_commit_customer.o \
        fm_nai_get_available_res.o \
				fm_nai_act_rate.o

#

LIBORDER=fm_nai_custom.$(SL_EXT)


DEP_linux = $(FILES)
DEP = $(DEP_$(OS))

###########

all: $(LIBORDER)

clean:
				rm -f $(OBJECTS) core

clobber: clean
				rm -f $(LIBORDER)

lint:
				lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBORDER): $(OBJECTS) $(INCFILES) Makefile $(DEP)
				$(LD) -o $(LIBORDER) $(LDFLAGS) $(OBJECTS) $(OS_LIBS)

$(OBJECTS): $(INCFILES) Makefile
				$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:
