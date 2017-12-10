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
#include <string>
#include "chunk_digest.h"
#ifdef __SDSCC__
#include <ff.h>
#include <sds_lib.h>
#endif
#include <typeinfo>

//#define MAX_CHUNK_SIZE (8192)
//#define MIN_CHUNK_SIZE (1024)

int ChunkNumber = 0;
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
    //unsigned char *Output = (unsigned char*)sds_alloc(OUTPUT_SIZE);
    static unsigned char Output[OUTPUT_SIZE];
    if (Input == NULL) {
        puts("Memory allocation error");
      return -1;
    }
    
    int MaxChunkNumber = INPUT_SIZE / 1024;
    int *ChunkLength = (int*)sds_alloc(MaxChunkNumber);
    int hisTableSize = INPUT_SIZE / 1024 * 32;
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
#ifdef __SDSCC__

    unsigned long long start=sds_clock_counter();
#endif
    int PreviousLength = 0;
    unsigned char digest[32];
    int index = 0;
    char deduplicate = 0;
    int CompressedLength = 0;
    int PreviousCompressedLength = 0;
    char last = 0;
    int chunk_acc = 0;
    int chunk_length = 0;
    unsigned long long chunkdigesttime=0;
    unsigned long long LZWtime=0;
    unsigned long long matchtime=0;

    // Precalculate a digest for the chunk;
    unsigned long long start1=sds_clock_counter();
    chunk_digest(Input, &chunk_acc, digest, &last, &chunk_length);
    chunkdigesttime+=sds_clock_counter()-start1;
    //chunk_acc += chunk_length;

    int k=0;
    int lzwtrue = 0;

    while (1) {
    	k++;
    	//printf("the ieration time is %d\n",k);


	// Stage 1 & 2 (calculate next chunk)
    	int temp1=chunk_length;
    	//printf("the temp is %d\n",temp);
    	//printf("the chunklength is %d\n",chunk_length);
    	//printf("the chunk_acc is %d\n",chunk_acc);
    	//printf("the last is %d\n",last);



//#pragma SDS async(1)


        // Stage 4: Compression
        //if the chunk come out from matching stage is not a duplicate
        if (deduplicate == 0) {
            unsigned long long start2=sds_clock_counter();
#pragma SDS async(1)
            lzw(Input + PreviousLength, Output + PreviousCompressedLength, chunk_length, &CompressedLength);
            LZWtime += sds_clock_counter()-start2;
            PreviousCompressedLength += CompressedLength;
            lzwtrue = 1;
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
        unsigned long long start3=sds_clock_counter();
        chunk_digest(Input + chunk_acc,&chunk_acc, digest, &last, &chunk_length);
        chunkdigesttime += sds_clock_counter()-start3;
        // Stage 3: Matching stage

        unsigned long long start4 = sds_clock_counter();
        std::string digest_str(digest, digest + (sizeof(digest) / sizeof(digest[0])));

        cppMatching(digest_str, &deduplicate, &index);
        matchtime += sds_clock_counter() - start4;
        
        PreviousLength += temp1;
        ChunkNumber++;
	
	// wait for both stages to finish (synchrnoization) before proceeding
        if(lzwtrue == 1){
       #pragma SDS wait(1)
        	lzwtrue = 0;
        }

	//chunk_acc += chunk_length;
	
		if (last == 1) {
			break;
		}
    }

    // Finish the pipeline
    // Stage 3: Matching stage
    std::string digest_str(digest, digest + (sizeof(digest) / sizeof(digest[0])));
    cppMatching(digest_str, &deduplicate, &index);
    
    // Stage 4: Compression
    //if the chunk come out from matching stage is not a duplicate
    if (deduplicate == 0) {
    	unsigned long long start5 = sds_clock_counter();
        lzw(Input + PreviousLength, Output + PreviousCompressedLength, chunk_length, &CompressedLength);
        LZWtime += sds_clock_counter()-start5;
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

     /***********************           Storing Data             *********************/
#ifdef __SDSCC__
    unsigned long long duration = sds_clock_counter() - start;
#endif
    store_data("OUT.bin", Output, PreviousCompressedLength);
    
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, PreviousCompressedLength);
    //store_data("/Users/koutsutomushiba/Desktop/chunktest/uncompressed.xml", Input, INPUT_SIZE);

#ifdef __SDSCC__
    sds_free(Input);
    //sds_free(Output);
    sds_free(ChunkLength);
    sds_free(historytable);
#else
    free(Input);
    free(Output);
    free(ChunkLength);
    free(historytable);
#endif
    
    Input =NULL;
    //Output=NULL;
    ChunkLength=NULL;
    historytable=NULL;
    printf("The total number of chunks is %d\n", ChunkNumber);
    printf("The number of LZW chunks is %d\n", LZWChunkNumber);
    printf("the number of duplicate chunks is %d\n",ChunkNumber-LZWChunkNumber);
#ifdef __SDSCC__

    printf("The duration is %llu \n",duration);
    printf("The duration for chunkdigest is %llu \n",chunkdigesttime);

    printf("The duration for match is %llu \n",matchtime);

    printf("The duration for LZW is %llu \n",LZWtime);


#endif
    puts("Application completed successfully.");

/*
    int CompressedLength;
    lzw(Input, Output,INPUT_SIZE, &CompressedLength);
    store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml", Output, CompressedLength);
 */
    return 0;
}
