#include <stdio.h>
#include <math.h>
#include "ContentDefinedChunk.h"

#define WINDOW_SIZE (32)
#define PRIME (3)
#define CHUNK_AVERAGE_SIZE (12)
#define MAX_CHUNK_SIZE 8192 // Bytes
#define MIN_CHUNK_SIZE (512) // Bytes
#define FINGERPRINT_BITS (64)

void ContentDefinedChunk(const unsigned char *Input,  int *ChunkLength,
			 int *ChunkNumber, int in_len)
{
    long Modulus = (long)pow(2, FINGERPRINT_BITS);
    long POW = ((long)pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int j = 0;
    //unsigned int chunklength = MIN_CHUNK_SIZE-1;
    unsigned int chunklength =WINDOW_SIZE;
    
    long long coeffient[WINDOW_SIZE];
    for(int t=0;t<WINDOW_SIZE;t++){
        coeffient[t]=(long long)pow(PRIME, WINDOW_SIZE-1-t);
    }
    
    for (int i = 0; i < (in_len - WINDOW_SIZE); i++) {
        for(int l=0;l<WINDOW_SIZE;l++){
            rollhash += (coeffient[l]*Input[i+l]) % Modulus;
        }
        
	
        chunklength++;
        long lowerbits = rollhash & (long)((pow(2,CHUNK_AVERAGE_SIZE))-1);

	// End the chunk at max size, at content marker, or when out of data
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || i == (in_len - WINDOW_SIZE - 1)) {
            ChunkLength[j++] = chunklength;
            printf("chunklength=%d\n",chunklength);
            (*ChunkNumber)++;
            chunklength = 0;
        }
    }
}
