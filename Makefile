all: datamaps
	
datamaps: src/reader.o src/main.o
	gcc src/reader.o src/main.o -o datamaps -g -std=c11 -Wall

reader.o:  
	gcc -c src/reader.c src/reader.h

main.o:
	gcc -c src/main.c

clean:
	rm src/reader.o src/main.o datamaps
