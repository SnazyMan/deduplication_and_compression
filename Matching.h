//
//  Matching.h
//  testfile
//
//  Created by 吱吱吱呀 on 2017/11/7.
//  Copyright © 2017年 RenzhiHuang. All rights reserved.
//

#ifndef Matching_h
#define Matching_h

#include <stdio.h>

void Matching(unsigned char *digest, unsigned char *historytable, int *LZWChunkNumber,
	      int *deduplicate, int *index);

unsigned int hash(unsigned char *digest);
unsigned int rehash(unsigned int curHash, unsigned char *historytable, unsigned char *digest,
		    int *deduplicate, int *index);

void insert(unsigned int curHash, unsigned char *historytable, unsigned char *digest,
	    int *LZWChunkNumber);

#endif /* Matching_h */

