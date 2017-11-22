#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "io.h"
#include "sha256.h"
#include "ContentDefinedChunk.h"
#include "Matching.h"
#include "lzw.h"

#define MAX_CHUNK_SIZE (8192)
#define MIN_CHUNK_SIZE (512)

unsigned char *Input;
unsigned char *Output;
int *ChunkLength;
unsigned char *historytable;
int LZWChunkNumber = 0;
int ChunkNumber = 0;

#pragma SDS data access_pattern(Input:SEQUENTIAL, Output:SEQUENTIAL)
#pragma SDS data mem_attribute(Input:PHYSICAL_CONTIGUOUS, Output:PHYSICAL_CONTIGUOUS)
int compress(unsigned char Input[INPUT_SIZE], unsigned char Output[OUTPUT_SIZE])
{

    int PreviousLength = 0;
    unsigned char digest[32];
    int index = 0;
    int deduplicate = 0;
    int CompressedLength = 0;
    int PreviousCompressedLength = 0;
    int chunk_length = 0;

    while (PreviousLength < (INPUT_SIZE - WINDOW_SIZE - 1)) {
#pragma HLS DATAFLOW
    	// Stage 1 : Content defined Chunk
    	chunk_length = ContentDefinedChunk(Input + PreviousLength);
	
        // Stage 2 : SHA stage
        sha256(Input + PreviousLength, chunk_length, digest);

        // Stage 3 : Matching stage
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

        // keep track of where we are in the input
        PreviousLength += chunk_length;
	
	// For statistics on content defined chunking
        ChunkNumber++;
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
    compressedLength = compress(Input, Output);

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

