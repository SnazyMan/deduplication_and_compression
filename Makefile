CC=gcc
CFLAGS=-I.
OBJ = app_main.o sha256.o ContentDefinedChunk.o lzw.o io.o Matching.o chunk_digest.o unit_test.o

%.o: %.c
	$(CC) -c -g -Wall -Wextra -std=c99 -o $@ $< $(CFLAGS)

dedup: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lm

clean:
	rm -rf *.c~ *.h~ *.o dedup
