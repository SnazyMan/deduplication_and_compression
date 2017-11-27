#ifndef Matching_h
#define Matching_h

#include <stdio.h>

void Matching(unsigned char digest[32], unsigned char historytable[1114112], int *LZWChunkNumber, char *deduplicate, int *index);


unsigned int hash(unsigned char digest[32]);
unsigned int rehash(unsigned int curHash, unsigned char historytable[1114112], unsigned char digest[32],
		    char *deduplicate, int *index);

void insert(unsigned int curHash, unsigned char historytable[1114112], unsigned char digest[32], int *LZWChunkNumber);
#endif /* Matching_h */

