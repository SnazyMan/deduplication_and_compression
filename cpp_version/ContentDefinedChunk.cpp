#include <stdio.h>
#include <math.h>
#include "ContentDefinedChunk.h"

#define WINDOW_SIZE (48)
#define PRIME (2)
#define CHUNK_AVERAGE_SIZE (12)
#define MAX_CHUNK_SIZE 8192 // Bytes
#define MIN_CHUNK_SIZE (2048) // Bytes
#define FINGERPRINT_BITS (64)

void ContentDefinedChunk(const unsigned char *Input, int *ChunkLength,
			 int *ChunkNumber, int in_len)
{
    long Modulus = (long)pow(2, FINGERPRINT_BITS);
    long POW = (long)(pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int j = 0;

    unsigned int chunklength = MIN_CHUNK_SIZE-1-64+WINDOW_SIZE;
    for(int i =MIN_CHUNK_SIZE-1-64; i< (in_len-WINDOW_SIZE);i++){
        
        rollhash = (rollhash * PRIME + Input[i + WINDOW_SIZE] - Input[i] * POW) % Modulus;
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, CHUNK_AVERAGE_SIZE) - 1);
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || i == (in_len - WINDOW_SIZE - 1)) {
            ChunkLength[j++] = chunklength;
            printf("chunklength%d\n",chunklength);
            (*ChunkNumber)++;
            chunklength = MIN_CHUNK_SIZE-1-64+WINDOW_SIZE;
            rollhash=0;
            i = i+MIN_CHUNK_SIZE-1-64+WINDOW_SIZE;
        }

    }
}
