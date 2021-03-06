#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#
SS = hex/shader_256.hex \
	hex/shader_512.hex \
	hex/shader_1k.hex \
	hex/shader_2k.hex \
	hex/shader_4k.hex \
	hex/shader_8k.hex \
	hex/shader_16k.hex \
	hex/shader_32k.hex \
	hex/shader_64k.hex \
	hex/shader_128k.hex \
	hex/shader_256k.hex \
	hex/shader_512k.hex \
	hex/shader_1024k.hex \
	hex/shader_2048k.hex \
	hex/shader_4096k.hex

# define the C compiler to use
CC = gcc#g++

# define any compile-time flags
CFLAGS = -Wall -g

# define any directories containing header files other than /usr/include
#
INCLUDES = -I./include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L./lib

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lfftw3 -lm -lrt -ldl -lliquid -lwiringPi

# define the C source files
FFTLIBS = lib/fftfiles
S = $(FFTLIBS)/($SS)
SRCS = main.c lib/frequencyprocessing.c lib/globals.c lib/timeprocessing.c lib/ads.c

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# Output Directory
ODIR = build

OBJ = $(patsubst %,$(ODIR)/%,$(OBJS))

# define the executable file
MAIN = ams

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all: $(MAIN)
	@echo Built ACE Antenna Measurement System

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(ODIR)/$(MAIN) $(OBJ) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $(ODIR)/$@

clean:
	$(RM) $(ODIR)/*.o $(ODIR)/*~ $(ODIR)/$(MAIN) $(ODIR)/lib/*.o *.txt

depend: $(SRCS)
	makedepend $(INCLUDES) $^

