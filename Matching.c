//
//  Matching.c
//  testfile
//
//  Created by 吱吱吱呀 on 2017/11/7.
//  Copyright © 2017年 RenzhiHuang. All rights reserved.
//

#include "Matching.h"

void Matching(unsigned char * digest, unsigned char * historytable, int *LZWChunkNumber, int *deduplicate,int *index)
{
    int currentLZWChunkNumber = *LZWChunkNumber;
    for(int i=0;i<= currentLZWChunkNumber;i++){
        int equal=0;
        for(int j=0;j<32;j++){
            if(digest[j]==historytable[i*32+j]){
                equal++;
            }
        }
        if(equal==32){
            *index = i;
            *deduplicate = 1;
        }
        else{
            *deduplicate = 0;
            for(int k = 0;k<32;k++){
                historytable[*LZWChunkNumber*32+k]=digest[k];
            }
            *LZWChunkNumber=1;
        }

    }
   
   // *LZWChunkNumber=1;

}
