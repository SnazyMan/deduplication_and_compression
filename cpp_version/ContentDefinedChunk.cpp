#include <stdio.h>
#include <math.h>
#include "ContentDefinedChunk.h"
#include "io.h"

#define WINDOW_SIZE (48)
#define PRIME (2)
#define CHUNK_AVERAGE_SIZE (12)
#define MIN_CHUNK_SIZE (2048) // Bytes
#define FINGERPRINT_BITS (64)

//int chunk_acc = 0;

#pragma SDS data mem_attribute(Input:PHYSICAL_CONTIGUOUS)
int ContentDefinedChunk(const unsigned char Input[8192], int* chunk_acc,unsigned char Output[8192], char *last)
{
    long Modulus = (long)pow(2, FINGERPRINT_BITS);
    long POW = (long)(pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int chunklength = MIN_CHUNK_SIZE - 1 - 64 + WINDOW_SIZE;
    (*chunk_acc) += chunklength;
    unsigned char window_buf[WINDOW_SIZE];
    // write first values guaranteed to be in the Chunk

    for (int j = 0; j < (chunklength - WINDOW_SIZE); j++) {
        Output[j] = Input[j];
    }

    // fill the window
    for (int k = (chunklength - WINDOW_SIZE); k < chunklength; k++) {
        window_buf[k] = Input[k];
	    Output[k] = window_buf[k];
    }
    
    for (int i = (MIN_CHUNK_SIZE - 1 - 64); i < (MAX_CHUNK_SIZE - WINDOW_SIZE); i++) {

#pragma HLS pipeline
    	// Calculate rolling hash
        rollhash = (rollhash * PRIME + window_buf[WINDOW_SIZE - 1] - window_buf[0] * POW) % Modulus;
	
        // write output byte to chunk
        Output[i + WINDOW_SIZE] = Input[i + WINDOW_SIZE];
		chunklength++;
		(*chunk_acc)++;

			// Check if Hash hits chunk marker, or out of bytes
        long lowerbits = rollhash & 4095;
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0))
        		|| (*chunk_acc) == (INPUT_SIZE - WINDOW_SIZE - 1)) {

        	if ((*chunk_acc) == (INPUT_SIZE - WINDOW_SIZE - 1)) {
        		*last = 1;
        	}
        	return chunklength;
		}

        //shift window
        for(int p=0;p<WINDOW_SIZE-1;p++){
#pragma HLS unroll
        	window_buf[p]=window_buf[p+1];
        }

        //grab another input byte
        window_buf[WINDOW_SIZE - 1] = Input[i + WINDOW_SIZE];
    }
    
    return chunklength;
}
