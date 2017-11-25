#include <math.h>
#include "ContentDefinedChunk.h"

#define CHUNK_AVERAGE_SIZE 12 // 4KB
#define FINGERPRINT_BITS 64
#define CHUNK_AVERAGE_SIZE_BITS ((1 << CHUNK_AVERAGE_SIZE) - 1) // Masks rollhash
#define MODULUS ((1 << 63) - 1)
#define CONSTANT 153191 //stolen from pcomress which is stolen from Bulat Ziganshin's REP
#define MASK 0xffffffffffULL
#define POLY 0xbfe6b8a5bf378d83ULL //stolen from pcompress which was stolen from LBFS

// Will these synthesize into a BRAM or is this going to be in DRAM? 
unsigned long irp_table[256]; // pre-calculated irreducible polynomial table
unsigned long outbyte[256]; // pre-calculated outbytes
static int chunk_acc = 0; // keeps track of total amount of bytes processed

#pragma SDS data mem_attribute(Input:PHYSICAL_CONTIGUOUS)
#pragma SDS data access_pattern(Input:SEQUENTIAL, Output:SEQUENTIAL)
int ContentDefinedChunk(const unsigned char Input[MAX_CHUNK_SIZE], unsigned char Output[MAX_CHUNK_SIZE])
{
    unsigned long rollhash = 0;
    unsigned long pos_hash = 0;
    unsigned int chunklength = WINDOW_SIZE;
    chunk_acc += WINDOW_SIZE;

    // Shift register of sorts (I think?)
    unsigned char window_buf[WINDOW_SIZE];
#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

    // Write first window of values (should be min chunk, less calculations, more throughput)
    int j;
    for (j = 0; j < WINDOW_SIZE; j++) {
    	window_buf[j] = Input[j];
    	Output[j] = window_buf[j];
    }
    
    int i;
    for (i = 0; i < (MAX_CHUNK_SIZE - WINDOW_SIZE); i++) {

    	// Compute the hash on the current window
        rollhash = (rollhash * CONSTANT) & MASK;
        rollhash += window_buf[WINDOW_SIZE - 1];
	rollhash -= outbyte[window_buf[0]];
	pos_hash = rollhash ^ irp_table[window_buf[0]];

	// Take bottom 12 bits to get average chunk size of 4KB
	// this can just be wired (routed)
        int lowerbits = pos_hash & CHUNK_AVERAGE_SIZE_BITS;

        // Write the output to next stage (sha256 in this case)
        Output[i + WINDOW_SIZE] = Input[i + WINDOW_SIZE];

        // shift the window
        int k;
        for (k = 0; k < (WINDOW_SIZE - 1); k++) {
#pragma HLS unroll
        	window_buf[k] = window_buf[k + 1];
        }

	// We processed another byte
	chunklength++;
	chunk_acc++;
	
        // End the chunk at max size, content marker, or at end of file
        if (((chunklength >= MIN_CHUNK_SIZE) && (chunklength == MAX_CHUNK_SIZE || lowerbits == 0)) || chunk_acc == (INPUT_SIZE)) {
        	//signal that chunk is done
        	return chunklength;
        }

    	//grab an input
        window_buf[WINDOW_SIZE - 1] = Input[i + WINDOW_SIZE];
    }

    return chunklength;
}

// Pre-calculate a table of irreducible polynomials
// Also taken from pcompress
void fill_irp_table()
{
    unsigned int term, pow, j, i;
    unsigned long val, poly_pow;

    poly_pow = 1;
    for (j = 0; j < WINDOW_SIZE; j++) {
        poly_pow = (poly_pow * CONSTANT) & MASK;
    }

    for (j = 0; j < 256; j++) {
        term = 1;
	pow = 1;
	val = 1;
	outbyte[j] = (j * poly_pow) & MASK;
	for (i = 0; i < WINDOW_SIZE; i++) {
	    if (term & POLY) {
	        val += ((pow * j) & MASK);
	    }
	    pow = (pow * CONSTANT) & MASK;
	    term <<= 1;
	}
	irp_table[j] = val;
    }
}
