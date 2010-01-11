AR = ar
CC = gcc
RM = rm -f
CXX = g++

CFLAGS_OPT = -Os -pipe -fomit-frame-pointer
CFLAGS_DBG = -O0 -ggdb

all: sched.a

test: test.cc sched.a
	$(CXX) $(CFLAGS_DBG) -o $@ test.cc sched.a

sched.a: sched.o sched_stack.o sched_queue.o err.o
	$(AR) -r $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS_DBG) -o $@ -c $<

clean:
	$(RM) sched.a sched.o sched_stack.o

.PHONY: all clean

