CC = c99
EXE = datamap
CFLAGS = -Wall -g -std=c99
LDFLAGS = -lsqlite3 -lxlsxio_read

.PHONY: all clean

all: $(EXE)

$(EXE): reader.o main.o
	$(CC) reader.o main.o -o datamaps $(CFLAGS) $(LDFLAGS)

clean:
	rm reader.o main.o datamaps test.db
