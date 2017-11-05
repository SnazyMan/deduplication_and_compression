//
//
//  ESE532
//
//  Created by RenzhiHuang on 2017/11/4.
//  Copyright © 2017年 RenzhiHuang. All rights reserved.
//
#include <math.h>
#define WindowSize (16)
#define FingerprintBits (64)
#define PRIME (2)
#define InputLength (10000)
void ContentDefinedChunk(const unsigned char * Input, int * ChunkLength)
{
    long Modulus = (long)pow(2,64);
    long POW = ((long)pow(PRIME, WindowSize) )% Modulus;
    long rollhash = 0;
    int j = 0;
    int chunklength = 0;
    for(int i = 0; i < InputLength; i++){
        rollhash = (rollhash * PRIME + Input[i+16]-Input[i] * POW) % Modulus;
        chunklength++;
        long lowerbits = rollhash & (1 << 12);
        if(lowerbits == 0){
            ChunkLength[j]=chunklength;
            j++;
            chunklength=0;
        }
    }
}
