#include <math.h>
#include "ContentDefinedChunk.h"

#define WindowSize (16)
#define FingerprintBits (64)
#define PRIME (23)
#define ChunkAverageSize (12)
#define MaxChunkSize (8192)
#define MinChunkSize (512)

void ContentDefinedChunk(const unsigned char *Input, int *ChunkLength, int *ChunkNumber,
			 int in_len)
{
    long Modulus = (long)pow(2, FingerprintBits);
    long POW = ((long)pow(PRIME, WindowSize)) % Modulus;
    long rollhash = 0;
    int j = 0;
    int chunklength = WindowSize;
    
    for (int i = 0; i < in_len-WindowSize; i++) {
        rollhash = (rollhash * PRIME + Input[i + WindowSize] - Input[i] * POW) % Modulus;
	
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, ChunkAverageSize) - 1);
	
        if (((chunklength >= MinChunkSize) &&(chunklength == MaxChunkSize || lowerbits == 0)) || i == (in_len - WindowSize - 1)) {
        //if((chunklength == MaxChunkSize || lowerbits == 0 || i == (in_len - WindowSize - 1))) {
            ChunkLength[j] = chunklength;
            j++;
            (*ChunkNumber)++;
            chunklength=0;
        }
    }
}
