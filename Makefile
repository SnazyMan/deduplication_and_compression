CC=gcc
CFLAGS=-I.
OBJ = dedup_main.o sha256.o ContentDefinedChunk.o lzw.o io.o

%.o: %.c
	$(CC) -c -g -Wall -Wextra -o $@ $< $(CFLAGS)

dedup: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.c~ *.h~ *.o dedup
