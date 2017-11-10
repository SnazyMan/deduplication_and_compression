
#include "Matching.h"

void Matching(unsigned char * digest, unsigned char * historytable, int *LZWChunkNumber, int *deduplicate,int *index)
{
    int currentLZWChunkNumber = (*LZWChunkNumber);
    
    if (currentLZWChunkNumber == 0) {
        *deduplicate = 0;
        for (int k = 0;k<32;k++) {
            historytable[currentLZWChunkNumber * 32 + k] = digest[k];
        }
        (*LZWChunkNumber)++;
    }
    else{
        
    for(int i=0;i < currentLZWChunkNumber;i++)
    {
        int equal = 0;
        for (int j = 0; j < 32; j++) {
            if (digest[j] == historytable[i * 32 + j]) {
                equal++;
            }
        }
        if (equal == 32) {
            *index = i;
            *deduplicate = 1;
        }
    }
    if(*deduplicate == 0){
    *deduplicate = 0;
    for (int k = 0; k < 32; k++) {
        historytable[currentLZWChunkNumber * 32 + k] = digest[k];
    }
    (*LZWChunkNumber)++;
    }
    }
    //printf("currentLZWnumber=%d\n",currentLZWChunkNumber);
    //printf("LZWnumber=%d\n",*LZWChunkNumber);
}
