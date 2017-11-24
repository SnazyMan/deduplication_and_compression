
#ifndef ContentDefinedChunk_h
#define ContentDefinedChunk_h
#include "io.h"

#define MAX_CHUNK_SIZE 8192 // Bytes
#define MIN_CHUNK_SIZE (512) // Bytes

#define WINDOW_SIZE (16)

int ContentDefinedChunk(const unsigned char Input[MAX_CHUNK_SIZE] , unsigned char Output[MAX_CHUNK_SIZE]);

#endif /* ContentDefinedChunk_h */
