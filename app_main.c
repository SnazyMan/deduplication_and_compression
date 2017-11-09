#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sha256.h"
#include "ContentDefinedChunk.h"
#include "Matching.h"
#include "io.h"
#include "lzw.h"



int main() {
    /***********************         Loading Data             *********************/
    
    unsigned char Input[300000];
    unsigned char Output[300000];
    load_data(Input);
    
    /***********************          4 Stages             *********************/

    
    int ChunkLength[1000];
    int ChunkNumber =0;
    int PreviousLength = 0;
    unsigned char digest[32];
    unsigned char historytable[100000];
    int index=0;
    int LZWChunkNumber=0;
    int deduplicate =0;
    int CompressedLength=0;
    int PreviousCompressedLength=0;
    
    //Stage 1
    
    ContentDefinedChunk(Input, ChunkLength, &ChunkNumber);
    
    //Stage 2-4, sent the chunks one by one to other stages
    
    for(int k=0;k<LZWChunkNumber;k++){
        
        //Stage 2 : SHA stage
        
        sha256(Input+PreviousLength,ChunkLength[k], digest);
        printf("k=%d\n",k);
        for(int l=0;l<32;l++){
            printf("%d",digest[l]);
            printf("\n");
        }
        
        //Stage 3 : Matching stage
        
        Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);
        
        //if the chunk come out from matching stage is not a duplicate
        
        if(deduplicate==0){
        lzw(Input+PreviousLength, Output+PreviousCompressedLength, ChunkLength[k],&CompressedLength);
        PreviousCompressedLength = PreviousCompressedLength+CompressedLength;
        }
        
        //if it's duplicate
        else{
            //add the header to the duplicate chunk
        }
        
        PreviousLength +=ChunkLength[k];

    }


     /***********************         Storing Data             *********************/

    
    store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml",Output, PreviousCompressedLength);

    
    
     /***************************************************   Ending ***********************************************************************/
    
    
    /***********************      separated test for LZW             *********************/
    /*
    char output[100000];
    //char output1[100000];
    int compressed_length = 0;
    //int compressed_length1 = 0;

    //printf("%c\n",Input[0]);
    //char *input="politics to the end of his life. In 1748 he sold his business in order to get leisure for study, having now acquired comparative wealth; and in a few years he had made discoveries that gave him a reputation with the learned throughout Europe.";
     //char *input1 = "In politics he proved very able both as an administrator and as a controversialist; but his record as an office-holder is stained by the use he made of his position to advance his relatives. His most notable service in home politics was his reform of the postal system; but his fame as a statesman rests chiefly on his services in connection with the relations of the Colonies with Great Britain, and later with France. In 1757 he was sent to England to protest against the influence of the Penns in the government of the colony, and for five years he remained there, striving to enlighten the people and the ministry of England as to Colonial conditions. On his return to America he played an honorable part in the Paxton affair, through which he lost his seat in the Assembly; but in 1764 he was again despatched to England as agent for the colony, this time to petition the King to resume the government from the hands of the proprietors. In";
    //lzw(input, output,strlen(input),&compressed_length);
    //lzw(input1, output1, strlen(input1),&compressed_length1);
   //lzw(input, output,strlen(input),&compressed_length);

    lzw(Input+200, output,200,&compressed_length);
    printf("%d\n",compressed_length);
    
    //lzw(Input+100, output+compressed_length,100,&compressed_length1);

    //for(int l=0;l<300;l++){
    //    printf("%c",input[l]);
    //}
    //printf("\n");

    //printf("%d\n",compressed_length1);

    printf("%s\n","lalalalal");

    for(int i=0;i<400;i++){
        if(output[i]==0){
            printf("%d\n",i);
        }
    }
    

    printf("Hello, World!\n");
    store_data("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml",output, compressed_length);
    //FILE * fd;
    //fd = fopen("/Users/koutsutomushiba/Desktop/chunktest/compressed.xml","wt+");
    //fwrite(output,sizeof(char),compressed_length, fd);
    
   /* //test matching stage
   
   Matching(digest, historytable, &LZWChunkNumber, &deduplicate, &index);
    printf("%d\n",LZWChunkNumber);
    printf("%d\n",deduplicate);
    printf("%d\n",index);
    for(int i=0;i<32;i++){
        printf("%d\n",historytable[i]);
    }
    */
   
    return 0;
}
