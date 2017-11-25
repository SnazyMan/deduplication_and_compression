#include "Matching.h"

#pragma SDS data mem_attribute(historytable:PHYSICAL_CONTIGUOUS)
void Matching(unsigned char digest[32], unsigned char historytable[1114112], int *LZWChunkNumber, char *deduplicate, int *index)
{
    // Calculate hash for incoming digest
    unsigned int curHash = hash(digest);

    // Check if valid bit is set 
    if (historytable[curHash + 33] & 1) {
        // check for collision
    	int equal = 0;
    	int j;
    	for (j = 0; j < 32; j++) {
    		if (digest[j] == historytable[curHash + j]) {
    			equal++;
    		}
    	}
    	// if there wasn't a collision, return the index
    	if (equal == 32) {
    		*index = (((int)historytable[curHash + j] << 8) | historytable[curHash + j + 1]) >> 1;
    		*deduplicate = 1;
    	}
    	else { // linear probe to find empty entry or return match
    		curHash = rehash(curHash, historytable, digest, deduplicate, index);
		
    		// If match wasnt found in rehash, insert the chunk
    		if (*deduplicate == 0) {
    			insert(curHash, historytable, digest, LZWChunkNumber);
    		}
    	}
    }
    else {
        // Space is empty, add to table
    	insert(curHash, historytable, digest, LZWChunkNumber);
    	*deduplicate = 0;
    }
}

/*
 * Compute hash on digest 
 * For now, the hash function will be bitwise xor
 * 256 LSB xor MSB 
 * mod by the table size by 1 entry to make sure it fits in the table
 */
unsigned int hash(unsigned char digest[32])
{
    unsigned int hash = 0;
    // this loop can get unrolled/pipelined for HLS
    // function may be revisted, it does operate on all bits of input key which is good
    // shifting like this may be unbalanced
    // decent amount of computational complexity for software here...
    int i;
    for (i = 0; i < 16; i++) {
        hash ^= (digest[i] ^ digest[31 - i]) << i;
    }

    hash = hash % 32768;
    hash *= 34;

    return hash;
}

//  Perform a linear probe until an empty entry is found
unsigned int rehash(unsigned int curHash, unsigned char historytable[1114112], unsigned char digest[32], char *deduplicate, int *index)
{
    int rehash = curHash;

    // problem here if table is full, need victimize logic?
    while (1) {
        // check the next valid bit (entries are 34 bytes wide)
    	rehash += 34;
    	rehash = rehash % 1114112;
    	if (historytable[rehash + 33] & 1) {
    	    // Entry already exists here, check if chunk in question
    		int equal = 0;
    		int j;
    		for (j = 0; j < 32; j++) {
    			if (digest[j] == historytable[rehash + j]) {
    				equal++;
    			}
    		}
    		// if chunk was found, return the index
    		if (equal == 32) {
    			*index = (((int)historytable[rehash + j] << 8) | historytable[rehash + j + 1]) >> 1;
    			*deduplicate = 1;
    			return rehash;
    		}
    	}
    	else {
    		// Found free space
    		*deduplicate = 0;
    		return rehash;
    	}
    }
}

void insert(unsigned int curHash, unsigned char historytable[1114112], unsigned char digest[32], int *LZWChunkNumber)
{
    int i;
    for (i = 0; i < 32; i++) {
        historytable[curHash + i] = digest[i];    	
    }

    // add LZW chunk number and valid bit
    historytable[curHash + i] = ((*LZWChunkNumber) & 0x0000FF00) >> 8;
    historytable[curHash + i + 1] = ((*LZWChunkNumber) & 0x000000FF) | 1;
    
    (*LZWChunkNumber)++;
}

