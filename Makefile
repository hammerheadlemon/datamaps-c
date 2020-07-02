CC = gcc
EXE = datamap
CFLAGS = -Wall -g -std=c99 -Wpedantic -O0
LDFLAGS = -lsqlite3 -lxlsxio_read

.PHONY: all clean

all: $(EXE)

$(EXE): reader.o main.o
	$(CC) reader.o main.o -o datamaps $(CFLAGS) $(LDFLAGS)

clean:
	rm reader.o main.o datamaps test.db
