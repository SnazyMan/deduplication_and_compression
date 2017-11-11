//
//
//  ESE532
//
//  Created by RenzhiHuang on 2017/11/4.
//  Copyright © 2017年 RenzhiHuang. All rights reserved.
//
#include <math.h>
#include "ContentDefinedChunk.h"

#define WindowSize (16)
#define FingerprintBits (64)
#define PRIME (23)
#define ChunkAverageSize (12)
#define MaxChunkSize (8192)
#define MinChunkSize (512)

void ContentDefinedChunk(const unsigned char *Input, int *ChunkLength, int *ChunkNumber,
			 int in_len)
{
    long Modulus = (long)pow(2, 64);
    long POW = ((long)pow(PRIME, WindowSize)) % Modulus;
    long rollhash = 0;
    int j = 0;
    int chunklength = 0;
    
    for (int i = 0; i < in_len - WindowSize; i++) {
        rollhash = (rollhash * PRIME + Input[i + 16] - Input[i] * POW) % Modulus;
	
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, ChunkAverageSize) - 1);
	
        if ((chunklength >= MinChunkSize) &&(chunklength == MaxChunkSize || lowerbits == 0 || i == (in_len - WindowSize - 1))) {
            ChunkLength[j++] = chunklength;
            (*ChunkNumber)++;
            chunklength=0;
        }
    }
}
