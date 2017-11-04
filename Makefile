CC=gcc
CFLAGS=-I.
OBJ = dedup_main.o sha256.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

dedup: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.c~ *.h~ *.o dedup
