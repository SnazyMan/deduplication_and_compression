#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sha256.h"
#include "ContentDefinedChunk.h"
#include "Matching.h"
#include "io.h"
#include "lzw.h"

int main()
{
	int ret;
	
    /***********************         Loading Data             *********************/

// Mount FAT filesystem on SD card
#ifdef __SDSCC__
	FATFS FS;
	ret = f_mount(&FS, "0:/", 0);
	if (ret != FR_OK) {
		printf("Could not mount SD-card\n");
		return -1;
	}
#endif

    unsigned char Input[INPUT_SIZE];
    unsigned char Output[OUTPUT_SIZE];
    memset(Output, 0, OUTPUT_SIZE);
    load_data(Input);

    
    /***********************          4 Stages             *********************/
    
    int ChunkLength[10000];
    int ChunkNumber = 0;
    int PreviousLength = 0;
    unsigned char digest[32];
    unsigned char historytable[300000];
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
        //sha256(Input + PreviousLength, ChunkLength[k], digest);
        
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
            unsigned char * temp = Output+PreviousCompressedLength;
            index <<= 1;
            
            index |= 1;
            
            temp[0] = (unsigned char)(index & 0x000000FF);
            temp[1] = (unsigned char)((index & 0x0000FF00) >> 8);
            temp[2] = (unsigned char)((index & 0x00FF0000) >> 16);
            temp[3] = (unsigned char)((index & 0xFF000000) >> 24);
            PreviousCompressedLength = 4+PreviousCompressedLength;
            
        }
        
        PreviousLength += ChunkLength[k];
    }


     /***********************         Storing Data             *********************/
    
    store_data("OUT.bin", Output, PreviousCompressedLength);
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, PreviousCompressedLength);

    return 0;
}
