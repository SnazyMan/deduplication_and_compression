CC=gcc
CFLAGS=-I.
OBJ = dedup_main.o sha256.o ContentDefinedChunk.o

%.o: %.c
	$(CC) -c -g -o $@ $< $(CFLAGS)

dedup: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.c~ *.h~ *.o dedup
