#include "Matching_2.h"

void Matching(unsigned char *digest, unsigned char *historytable, int *LZWChunkNumber, int *deduplicate, int *index)
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
    		*index = (historytable[curHash + j] | historytable[curHash + j + 1]) >> 1;
    		*deduplicate = 1;
    	}
    	else { // linear probe to find empty entry
    		curHash = rehash(curHash, historytable);
		
    		// Space is empty, add to table
    		insert(curHash, historytable, digest, LZWChunkNumber);
    		*deduplicate = 0;
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
unsigned int hash(unsigned char *digest)
{
    unsigned int hash = 0;
    // this loop can get unrolled/pipelined for HLS
    // function may be revisted, it does operate on all bits of input key which is good
    // shifting like this may be unbalanced
    // decent amount of computational complexity for software here...
    for (int i = 0; i < 16; i++) {
        hash |= (digest[i] ^ digest[32 - i]) << i;
    }

    hash = hash % 1114117;

    return hash;
}

//  Perform a linear probe until an empty entry is found
unsigned int rehash(unsigned int curHash, unsigned char *historytable)
{
    int rehash = curHash;

    // problem here if table is full, need victimize logic?
    while (1) {
        // check the next valid bit (entries are 34 bytes wide)
    	rehash += 34;
    	rehash = rehash % 1114117;
    	if (historytable[rehash + 33] & 1) {
    		// Entry already exists here
    	}
    	else {
    		// Found free space
    		return rehash;
    	}
    }
}

void insert(unsigned int curHash, unsigned char *historytable, unsigned char *digest, int *LZWChunkNumber)
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
