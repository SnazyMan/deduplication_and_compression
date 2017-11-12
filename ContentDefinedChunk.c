//
//
//  ESE532
//
//  Created by RenzhiHuang on 2017/11/4.
//  Copyright © 2017年 RenzhiHuang. All rights reserved.
//
#include <math.h>
#include "ContentDefinedChunk.h"

#define WINDOW_SIZE (16)
#define PRIME (23)
#define CHUNK_AVERAGE_SIZE (12)
#define MAX_CHUNK_SIZE 8192 //bytes

void ContentDefinedChunk(const unsigned char *Input, unsigned int *ChunkLength,
			 unsigned int *ChunkNumber, int in_len)
{
    long Modulus = (long)pow(2, 64);
    long POW = ((long)pow(PRIME, WINDOW_SIZE)) % Modulus;
    long rollhash = 0;
    unsigned int j = 0;
    unsigned int chunklength = 0;
    
    for (int i = 0; i < (in_len - WINDOW_SIZE); i++) {
        rollhash = (rollhash * PRIME + Input[i + 16] - Input[i] * POW) % Modulus;
	
        chunklength++;
        long lowerbits = rollhash & ((long)pow(2, CHUNK_AVERAGE_SIZE) - 1);

	// End the chunk at max size, at content marker or when out of data
        if (chunklength == MAX_CHUNK_SIZE || lowerbits == 0 || i == (in_len - WINDOW_SIZE - 1)) {
            ChunkLength[j++] = chunklength;
            (*ChunkNumber)++;
            chunklength=0;
        }
    }
}
