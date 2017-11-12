#include <math.h>
#include "ContentDefinedChunk.h"

#define WINDOW_SIZE (16)
#define PRIME (23)
#define CHUNK_AVERAGE_SIZE (12)
#define MAX_CHUNK_SIZE 8192 //bytes
#define MIN_CHUNK_SIZE (512)
#define FINGERPRINT_BITS (64)

void ContentDefinedChunk(const unsigned char *Input, unsigned int *ChunkLength,
			 unsigned int *ChunkNumber, int in_len)
{

    long Modulus = (long)pow(2, FINGERPRINT_BITS);
    long POW = ((long)pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int j = 0;
    unsigned int chunklength = WINDOW_SIZE;
    
    for (int i = 0; i < (in_len - WINDOW_SIZE); i++) {
        rollhash = (rollhash * PRIME + Input[i + WINDOW_SIZE] - Input[i] * POW) % Modulus;
	
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, CHUNK_AVERAGE_SIZE) - 1);

	// End the chunk at max size, at content marker, or when out of data
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || i == (in_len - WINDOW_SIZE - 1)) {
            ChunkLength[j++] = chunklength;
            (*ChunkNumber)++;
            chunklength = 0;
        }
    }
}
