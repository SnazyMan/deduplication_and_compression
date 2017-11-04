#include "sha256.h"
#include <stdio.h>

/* 
 * Implementation of the sha256 algorithm
 * Input is a chunk from a file that is sectioned with Rabin Fingerprint
 * Targeted for ZYNQ SoC, more specifically probably HLS
 * Note that ARM is little endian, which is somewhat important
 * for this algorithm
 */
int sha256(char *input_chunk, int chunk_length, char *digest)
{
	int ret;
	struct sha256_ctx ctx;
	
	// Valid parameter check
	if ((input_chunk == 0) || (digest == 0) || (chunk_length == 0)) {
		printf("Invalid parameter to SHA256 entry\n");
		return -1;
	}

	int sha256_init();
	
	// Pre-processing
	
	
	// input chunk expansion into message schedule

	// Compression

	// Populate output

	return ret;
}

/*
 * Function to initialize the "state" of the algorithm
 * Sets initial hash values as determined in the FIPS specification
 * 
 */ 
int sha256_init(struct sha256_ctx *ctx)
{
	int ret;

	return ret;
}
