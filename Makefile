HOMEDIR = .
DEBUG_FLAGS =
#DEBUG_FLAGS +=	-g #-D__DEBUG -DKT_DEBUG

CC			= 	g++
LINK		= 	g++
DO_SUFFIX	=	dll.o
DO_CFLAGS	=	-fPIC -m64
CFLAGS		= 	-c -O5 -Wall -m64 -D__64BIT
CFLAGS      +=  -Wno-write-strings
CFLAGS      +=  -fno-strict-aliasing
CFLAGS2		=	-fPIC -m64 -D__64BIT
DO_CFLAGS2	=	-fPIC -m64
LD_PATH		= 	-L$(LIBOUTDIR) -L/usr/lib
LFLAGS		= 	-O5  -m64
SO_FLAGS	=	-shared -m64
LIBS		=	-lpthread -lnsl -ldl -lpcre-gcc-linux64-intel
CMD_RANLIB	=	ranlib

CFLAGS		+= 	$(DEBUG_FLAGS) -c -O5 


########################################################

include	Makefile.ver
include	Makefile.nam.unix
include	Makefile.lst
include	Makefile.target.unix
include	Makefile.suffix.unix
include	Makefile.etc.unix



