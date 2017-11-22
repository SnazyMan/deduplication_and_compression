#include <math.h>
#include "ContentDefinedChunk.h"

#define PRIME (23)
#define CHUNK_AVERAGE_SIZE (12)
#define MAX_CHUNK_SIZE 8192 // Bytes
#define MIN_CHUNK_SIZE (512) // Bytes
#define FINGERPRINT_BITS (64)

#pragma SDS data mem_attribute(Input:PHYSICAL_CONTIGUOUS)
#pragma SDS data access_pattern(Input:SEQUENTIAL)
int ContentDefinedChunk(const unsigned char Input[INPUT_SIZE])
{
    long Modulus = (long)pow(2, FINGERPRINT_BITS);
    long POW = ((long)pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int chunklength = WINDOW_SIZE;
    
    for (int i = 0; i < (MAX_CHUNK_SIZE - WINDOW_SIZE); i++) {
        rollhash = (rollhash * PRIME + Input[i + WINDOW_SIZE] - Input[i] * POW) % Modulus;
	
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, CHUNK_AVERAGE_SIZE) - 1);

        // End the chunk at max size, content marker, or at end of file
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || i == (INPUT_SIZE - WINDOW_SIZE)) {
        	return chunklength;
        }
    }

    return chunklength;
}
