# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
# CC      = /usr/bin/gcc
# CFLAGS  = -ansi -pedantic -Wall -O2
# LIBPATH = -L.
# LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
# CFDEBUG = -ansi -pedantic -Wall -g -DDEBUG $(LDFLAGS)
# RM      = /bin/rm -f

EXECS=QuickSort
MPICC?=mpicc

all: ${EXECS}

QuickSort.o: QuickSort.c QuickSort.h
	${MPICC} -c -O3 QuickSort.c

Utility.o: Utility.c QuickSort.h
	${MPICC} -c -O3 Utility.c

QuickSort: Utility.o QuickSort.o
	${MPICC} -o QuickSort Utility.o QuickSort.o

# QuickSort.o: QuickSort.c QuickSort.h
#	gcc -c -O3 QuickSort.c -pthread

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean:
	rm -f ${EXECS} *.o