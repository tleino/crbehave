CFLAGS=-g -Wall

all: crbehave.o

.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm -f crbehave.o
