CC = gcc
CFLAGS = -Wall -g -std=c99
LDFLAGS = -lsqlite3
LDFLAGS += -lxlsxio_read

all: datamaps
	
datamaps: src/reader.o src/main.o
	gcc src/reader.o src/main.o -o datamaps $(CFLAGS) $(LDFLAGS)

reader.o:  
	gcc -c src/reader.c src/reader.h $(LDFLAGS)

main.o:
	gcc -c src/main.c

clean:
	rm src/reader.o src/main.o datamaps test.db
