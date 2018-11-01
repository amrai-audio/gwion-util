CC ?=gcc
YACC ?=yacc
LEX ?=flex
PREFIX ?=/usr/local

CFLAGS += -Iinclude -std=c99 -O3 -mtune=native -fstrict-aliasing -D_GNU_SOURCE
CFLAGS += -Wall -Wextra

USE_DOUBLE    = 0
USE_COVERAGE ?= 0
USE_MEMCHECK ?= 0
USE_LTO      ?= 1

ifeq (${USE_DOUBLE}, 1)
CFLAGS +=-DUSE_DOUBLE -DSPFLOAT=double
else
CFLAGS+=-DSPFLOAT=float
endif

ifeq (${USE_COVERAGE}, 1)
CFLAGS += -ftest-coverage -fprofile-arcs
LDFLAGS += --coverage
endif

ifeq (${USE_MEMCHECK}, 1)
CFLAGS += -g -Og
endif

ifeq (${USE_LTO}, 1)
CFLAGS += -flto
LDFLAGS += -flto
endif

#ifeq (${USE_COLOR}, 1)
#CFLAGS+= -DCOLOR
#endif