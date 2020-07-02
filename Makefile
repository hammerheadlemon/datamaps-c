CC = gcc
EXE = datamap
T_READER_EXE = test_reader 
CFLAGS = -Wall -g -std=c99 -Wpedantic -O0
LDFLAGS = -lsqlite3 -lxlsxio_read

.PHONY: all clean

all: $(EXE) $(T_READER_EXE)

$(EXE): reader.o main.o
	$(CC) reader.o main.o -o datamaps $(CFLAGS) $(LDFLAGS)

$(T_READER_EXE): reader_test.c
	bash -c "gcc -o reader_test reader_test.c `pkg-config --cflags --libs glib-2.0`"

clean:
	rm reader.o main.o datamaps test.db reader_test

