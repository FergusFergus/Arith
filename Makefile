# Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
# COMP 40 Homework 4: arith
# Makefile

CC = gcc #the compiler

IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64

LDLIBS = -l40locality -larith40 -lnetpbm -lcii40 -lm -lrt

INCLUDES = $(shell echo *.h)


all: 40image bitpack.o

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

40image: 40image.o a2plain.o uarray2.o YPbPr.o PixBlock.o DCTransform.o \
CodedImg.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)


clean:
	rm -f 40image *.o
