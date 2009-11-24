CC := gcc
CFLAGS := -Wall

all: admin executor

admin: admin.c err.h err.o grammar.h grammar.o readwrite.h readwrite.o
	${CC} ${CFLAGS} admin.c err.o readwrite.o grammar.o -o admin

executor: executor.c grammar.o err.h err.o readwrite.h readwrite.o
	${CC} ${CFLAGS} executor.c grammar.o err.o readwrite.o -o executor

err.o: err.h err.c
	${CC} ${CFLAGS} -c err.c -o err.o

grammar.o: grammar.h grammar.c err.h err.c
	${CC} ${CFLAGS} -c grammar.c -o grammar.o

readwrite.o: readwrite.h readwrite.c
	${CC} ${CFLAGS} -c readwrite.c -o readwrite.o

clean:
	@rm -f *.o *~

.PHONY: all clean
