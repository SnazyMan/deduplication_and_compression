CC=gcc
CXXFLAGS=-I. -std=c++11 -g
OBJ = app_main.o sha256.o ContentDefinedChunk.o lzw.o io.o  

%.o: %.c
	$(CXX) -c -g -Wall -Wextra -o $@ $< $(CXXFLAGS)

dedup: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) -lm

clean:
	rm -rf *.c~ *.h~ *.o dedup
