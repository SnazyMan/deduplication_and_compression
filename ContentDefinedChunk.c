#include <math.h>
#include "ContentDefinedChunk.h"

#define PRIME (23)
#define CHUNK_AVERAGE_SIZE (12)
#define MIN_CHUNK_SIZE (512) // Bytes
#define FINGERPRINT_BITS (64)
#define CHUNK_AVERAGE_SIZE_BITS ((1 << CHUNK_AVERAGE_SIZE) - 1)
#define MODULUS ((1 << 63) - 1)
#define CONSTANT 153191
#define MASK (0xffffffffffULL)

#pragma SDS data mem_attribute(Input:PHYSICAL_CONTIGUOUS)
#pragma SDS data access_pattern(Input:SEQUENTIAL, Output:SEQUENTIAL)
int ContentDefinedChunk(const unsigned char Input[MAX_CHUNK_SIZE], unsigned char Output[MAX_CHUNK_SIZE])
{
	//long Modulus = (long)pow(2, FINGERPRINT_BITS);
    //long POW = ((unsigned long long)pow(PRIME, WINDOW_SIZE)) & MODULUS;
    long rollhash = 0;
    unsigned int chunklength = WINDOW_SIZE;

    // Shift register of sorts (I think?)
    unsigned char window_buf[WINDOW_SIZE];
#pragma HLS ARRAY_PARTITION variable=window_buf=complete dim=0

    
    // Write first window of values (should be min chunk, less calculations, more throughput)
    int j;
    for (j = 0; j < WINDOW_SIZE; j++) {
    	window_buf[j] = Input[j];
    	Output[j] = window_buf[j];
    }
    
    int i;
    for (i = 0; i < (MAX_CHUNK_SIZE - WINDOW_SIZE); i++) {

    	// Compute the hash
        rollhash = (rollhash * CONSTANT) & MASK;
        rollhash += window_buf[WINDOW_SIZE - 1];
		rollhash -= window_buf[0];

		// take bottom 12 bits to get average chunk size of 4KB
        chunklength++;
        long lowerbits = rollhash & CHUNK_AVERAGE_SIZE_BITS;

        //write the output
        Output[i + WINDOW_SIZE] = Input[i + WINDOW_SIZE];

        // shift the window
        int k;
        for (k = 0; k < (WINDOW_SIZE - 1); k++) {
#pragma HLS unroll
        	window_buf[k] = window_buf[k + 1];
        }

        // End the chunk at max size, content marker, or at end of file
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || i == (INPUT_SIZE - WINDOW_SIZE)) {
        	//signal that chunk is done
        	return chunklength;
        }

    	//grab an input
        window_buf[WINDOW_SIZE - 1] = Input[i + WINDOW_SIZE];
    }

    return chunklength;
}
