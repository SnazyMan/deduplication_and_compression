#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "io.h"
#include "Matching.h"
#include "lzw.h"
#include "chunk_digest.h"

unsigned char *Input; // stores input in DRAM from SD Card
unsigned char *Output; // stores Output in DRAM
int *ChunkLength; // I think this has been deprecated
unsigned char *historytable; // hash table holding previous chunk digests
int LZWChunkNumber = 0; // statistics on how many chunks we compressed
int ChunkNumber = 0; // statistics on how many chunks total

int compress(unsigned char *Input, unsigned char *Output)
{
    int PreviousLength = 0; // accumulator to keep track of offset for lzw
    int chunk_acc = 0; // acuumulator to keep track of offset for chunking
    unsigned char digest[32]; // holds digest for current chunk
    int index = 0; // if chunk was a duplicate, holds index of chunk in historytable
    char deduplicate = 0; // flag from matching stage used to compress of add duplicate header
    int CompressedLength = 0; // Compressed length of a chunk from lzw
    int PreviousCompressedLength = 0; // Keeps track of where we are in the Output buffer across multiple iterations
    int chunk_length = 0; // holds length of chunk
//    unsigned char digest_tb[32] = {0xcd, 0xb2, 0xc3, 0xb8, 0x73, 0x3d, 0xa5, 0xf4, 0x4a, 0xb7,
//    		0x15, 0x20, 0x10, 0xb3, 0xa2, 0xd9, 0x48, 0x38, 0xd9, 0xba, 0xc3, 0x10, 0x4, 0x6f,
//			0x9c, 0xc8, 0x9e,
//   		0x77, 0x7e, 0xdd, 0x77, 0x13};

//    int good = 0;

    // Precalculate a chunk so that chunking/digest to enable concurrency
	chunk_digest(Input + chunk_acc, digest, &chunk_length);
	chunk_acc = chunk_length;

    while (chunk_acc < (INPUT_SIZE - WINDOW_SIZE - 1)) {
	    
    	// Stage 1 & 2
    	// Start chunk/sha on (next chunk)
#pragma SDS async(1);
    	chunk_digest(Input + chunk_acc, digest, &chunk_length);

#if(0)
    	int k;
    	for (k = 0; k < 32;  k++) {
    		if (digest[k] == digest_tb[k]) {
    			good = 1;
    		}
    		else {
    			good = 0;
    			return 1;
    		}
    	}

    	if (good == 1) {
    		return 0;
    	}
#endif
        // Stage 3 : Matching stage
    	// Start Matching/Compression on (previous) chunk
    	// Potential timing issue on digest it could be updated with next chunk before it starts, (I think unlikely)
        Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);
        
        // If the chunk come out from matching stage is not a duplicate
        if (deduplicate == 0) {
            lzw(Input + PreviousLength, Output + PreviousCompressedLength,
                chunk_length, &CompressedLength);
            PreviousCompressedLength += CompressedLength;
        }
        else {
            //if it's duplicate, add the header to the duplicate chunk
            unsigned char *temp = Output + PreviousCompressedLength;
            index <<= 1;
            
            index |= 1;
            
            temp[0] = (unsigned char) (index & 0x000000FF);
            temp[1] = (unsigned char)((index & 0x0000FF00) >> 8);
            temp[2] = (unsigned char)((index & 0x00FF0000) >> 16);
            temp[3] = (unsigned char)((index & 0xFF000000) >> 24);
            PreviousCompressedLength += 4;

            // Reset deduplicate for next chunk
            deduplicate = 0;
        }

        // keep track of where we are in the input buffer
        PreviousLength += chunk_length;
	
        // For statistics on content defined chunking
        ChunkNumber++;

        // Synchronization , make sure both chunk/sha & Matching/LZW finish before starting again
#pragma SDS wait(1);

        // At this point chunk/sha is guarenteed to be finished, update Input position
    	chunk_acc += chunk_length;
    }
    
    //Preform final Matching
    Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);

    // If the chunk come out from matching stage is not a duplicate
    if (deduplicate == 0) {
    	lzw(Input + PreviousLength, Output + PreviousCompressedLength,
    			chunk_length, &CompressedLength);
    	PreviousCompressedLength += CompressedLength;
    }
    else {
    	//if it's duplicate, add the header to the duplicate chunk
        unsigned char *temp = Output + PreviousCompressedLength;
        index <<= 1;

        index |= 1;

        temp[0] = (unsigned char) (index & 0x000000FF);
        temp[1] = (unsigned char)((index & 0x0000FF00) >> 8);
        temp[2] = (unsigned char)((index & 0x00FF0000) >> 16);
        temp[3] = (unsigned char)((index & 0xFF000000) >> 24);
        PreviousCompressedLength += 4;

        // Reset deduplicate for next chunk
        deduplicate = 0;
    }

    return PreviousCompressedLength;
}

int main()
{

    /********************          Allocate Memory        **************************/
#ifdef __SDSCC__	
    Input = (unsigned char*)sds_alloc(INPUT_SIZE);
    Output = (unsigned char*)sds_alloc(OUTPUT_SIZE);
    if (Input == NULL || Output == NULL) {
        puts("Memory allocation error");
        return -1;
    }
    
    int MaxChunkNumber = INPUT_SIZE / MIN_CHUNK_SIZE;
    ChunkLength = (int*)sds_alloc(MaxChunkNumber);
    // 34 bytes wide (32 byte digest, 2 byte index + valid bit) * (2 ^15) entries
    int hisTableSize = 1114112;
    historytable = (unsigned char*)sds_alloc(hisTableSize);
    
#else
    Input = (unsigned char*)malloc(INPUT_SIZE);
    Output = (unsigned char*)malloc(OUTPUT_SIZE);
    if (Input == NULL || Output == NULL) {
        puts("Memory allocation error");
        return -1;
    }
    
    int MaxChunkNumber = INPUT_SIZE / MIN_CHUNK_SIZE;
    ChunkLength = (int*)malloc(MaxChunkNumber);
    // 34 bytes wide (32 byte digest, 2 byte index + valid bit) * (2 ^15) entries)
    int hisTableSize = 1114112;
    historytable = (unsigned char*)malloc(hisTableSize);
    
#endif
    memset(historytable, 0, hisTableSize);
    memset(Output, 0, OUTPUT_SIZE);

    /***********************           Loading Data             *********************/
   
// Mount FAT filesystem on SD card
#ifdef __SDSCC__
    FATFS FS;
    int ret = f_mount(&FS, "0:/", 0);
    if (ret != FR_OK) {
	printf("Could not mount SD-card\n");
	return ret;
    }
#endif
	
    load_data(Input);

    /***********************            Compress               *********************/

    int compressedLength = 0;
    unsigned long long start = sds_clock_counter();
    compressedLength = compress(Input, Output);
    unsigned long long end = sds_clock_counter();
    printf("cycles = %llu\n", end - start);

#if(0)
    if (compressedLength == 1) {
    	return 1;
    }
    else if (compressedLength == 0) {
    	return 0;
    }
#endif
    printf("The total number of chunks is %d\n", ChunkNumber);
    printf("The number of LZW chunks is %d\n", LZWChunkNumber);
    printf("the number of duplicate chunks is %d\n", ChunkNumber - LZWChunkNumber);

    /***********************           Storing Data             *********************/
    
    store_data("OUT.bin", Output, compressedLength);

#ifdef __SDSCC__
    sds_free(Input);
    sds_free(Output);
    sds_free(ChunkLength);
    sds_free(historytable);
#else
    free(Input);
    free(Output);
    free(ChunkLength);
    free(historytable);
#endif
    
    return 0;
}

