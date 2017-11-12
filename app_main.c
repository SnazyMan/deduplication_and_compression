#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <string.h>
#include "sha256.h"
#include "ContentDefinedChunk.h"
#include "Matching.h"
#include "io.h"
#include "lzw.h"

//#define STAGES (4)
#define MAX_CHUNK_SIZE (8192)
#define MIN_CHUNK_SIZE (512)

int main()
{
    unsigned char *Input = (unsigned char*)malloc(INPUT_SIZE);
    unsigned char *Output = (unsigned char*)malloc(OUTPUT_SIZE);
    if (Input == NULL || Output == NULL) {
        puts("Memory allocation error");
        return -1;
    }
    
    int MaxChunkNumber=INPUT_SIZE / MIN_CHUNK_SIZE;
    int *ChunkLength = (int*)malloc(MaxChunkNumber);
    int hisTableSize= INPUT_SIZE / MIN_CHUNK_SIZE * 32;
    unsigned char *historytable = (unsigned char*)malloc(hisTableSize);

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

    /***********************            4 Stages             *********************/
    
    int ChunkNumber = 0;
    int PreviousLength = 0;
    unsigned char digest[32];
    int index = 0;
    int LZWChunkNumber = 0;
    int deduplicate = 0;
    int CompressedLength = 0;
    int PreviousCompressedLength = 0;
    // Stage 1
    ContentDefinedChunk(Input, ChunkLength, &ChunkNumber, INPUT_SIZE);

    // Stage 2-4, sent the chunks one by one to other stages
    
    for (int k = 0; k < ChunkNumber; k++) {

        // Stage 2 : SHA stage
        sha256(Input + PreviousLength, ChunkLength[k], digest);
        
        //Stage 3 : Matching stage
        Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);
        
        //if the chunk come out from matching stage is not a duplicate
        if (deduplicate == 0) {
            lzw(Input + PreviousLength, Output + PreviousCompressedLength,
		ChunkLength[k], &CompressedLength);
	    PreviousCompressedLength += CompressedLength;
        }
        else {
            //if it's duplicate
            //add the header to the duplicate chunk
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
        
        PreviousLength += ChunkLength[k];
    }


     /***********************           Storing Data             *********************/
    
    //store_data("OUT.bin", Output, PreviousCompressedLength);
    store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, PreviousCompressedLength);
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/uncompressed.xml", Input, INPUT_SIZE);
    
    free(Input);
    free(Output);
    free(ChunkLength);
    free(historytable);
    Input =NULL;
    Output=NULL;
    ChunkLength=NULL;
    historytable=NULL;
    printf("The total number of chunks is %d\n", ChunkNumber);
    printf("The number of LZW chunks is %d\n", LZWChunkNumber);
    printf("the number of duplicate chunks is %d\n",ChunkNumber-LZWChunkNumber);
    puts("Application completed successfully.");

    return 0;
}
