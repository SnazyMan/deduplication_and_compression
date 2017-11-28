#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <string.h>
#include "sha256.h"
#include "ContentDefinedChunk.h"
//#include "Matching.h"
#include "io.h"
#include "lzw.h"
#include <unordered_map>

#define MAX_CHUNK_SIZE (8192)
#define MIN_CHUNK_SIZE (512)

int LZWChunkNumber = 0;
std::unordered_map<std::string, int> chunkMap;

void cppMatching(std::string digest, char *deduplicate, int *index)
{
    // Search map to Check if the digest exits
   std::unordered_map<std::string, int>::iterator got = chunkMap.find(digest);

    // insert if the iterator got to the element one past the map end
    if (got == chunkMap.end()) {
	    chunkMap.insert({digest, LZWChunkNumber});
	    LZWChunkNumber++;
	    *deduplicate = 0;
    }
    else { // return the second value in the iterator
	    *index = got->second;
	    *deduplicate = 1;
    }
}

int main()
{

    /********************          Allocate Memory        **************************/
#ifdef __SDSCC__	
    unsigned char *Input = (unsigned char*)sds_alloc(INPUT_SIZE);
    unsigned char *Output = (unsigned char*)sds_alloc(OUTPUT_SIZE);
    if (Input == NULL || Output == NULL) {
        puts("Memory allocation error");
        return -1;
    }
    
    int MaxChunkNumber = INPUT_SIZE / MIN_CHUNK_SIZE;
    int *ChunkLength = (int*)sds_alloc(MaxChunkNumber);
    int hisTableSize = INPUT_SIZE / MIN_CHUNK_SIZE * 32;
    unsigned char *historytable = (unsigned char*)sds_alloc(hisTableSize);
    
#else
    unsigned char *Input = (unsigned char*)malloc(INPUT_SIZE);
    unsigned char *Output = (unsigned char*)malloc(OUTPUT_SIZE);
    if (Input == NULL || Output == NULL) {
        puts("Memory allocation error");
        return -1;
    }
    
    int MaxChunkNumber = INPUT_SIZE / MIN_CHUNK_SIZE;
    int *ChunkLength = (int*)malloc(MaxChunkNumber);
    int hisTableSize = INPUT_SIZE / MIN_CHUNK_SIZE * 32;
    unsigned char *historytable = (unsigned char*)malloc(hisTableSize);
    
#endif
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

    int ChunkNumber = 0;
    int PreviousLength = 0;
    unsigned char digest[32];
    int index = 0;

    char deduplicate = 0;
    int CompressedLength = 0;
    int PreviousCompressedLength = 0;

    // Stage 1
    ContentDefinedChunk(Input, ChunkLength, &ChunkNumber, INPUT_SIZE);

    // Stage 2-4, sent the chunks one by one to other stages
    for (int k = 0; k < ChunkNumber; k++) {

        // Stage 2 : SHA stage
        sha256(Input + PreviousLength, ChunkLength[k], digest);
        
        //Stage 3 : Matching stage
        //Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);

	std::string digest_str(digest, digest + (sizeof(digest) / sizeof(digest[0])));
	cppMatching(digest_str, &deduplicate, &index);
	
        //if the chunk come out from matching stage is not a duplicate
        if (deduplicate == 0) {
            lzw(Input + PreviousLength, Output + PreviousCompressedLength,ChunkLength[k], &CompressedLength);
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

    store_data("OUT.bin", Output, PreviousCompressedLength);
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, PreviousCompressedLength);
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/uncompressed.xml", Input, INPUT_SIZE);

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
    
    Input =NULL;
    Output=NULL;
    ChunkLength=NULL;
    historytable=NULL;
    printf("The total number of chunks is %d\n", ChunkNumber);
    printf("The number of LZW chunks is %d\n", LZWChunkNumber);
    printf("the number of duplicate chunks is %d\n",ChunkNumber-LZWChunkNumber);
    puts("Application completed successfully.");
/*
    int CompressedLength;
    lzw(Input, Output,INPUT_SIZE, &CompressedLength);
    store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, CompressedLength);
 */
    return 0;
}
