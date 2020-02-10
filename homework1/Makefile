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

default: QuickSort

QuickSort: QuickSort.o
	gcc -o QuickSort QuickSort.o -pthread

QuickSort.o: QuickSort.c
	gcc -c QuickSort.c -pthread

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean:
	rm -f QuickSort *.o *~