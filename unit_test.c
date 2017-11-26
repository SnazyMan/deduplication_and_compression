#include "sha256.h"
#include "Matching.h"
#include <string.h>

extern unsigned char *historytable;

int unit_test()
{
    int ret = 0;
    
    // Test sha256 - test inputs to known outputs
    unsigned char text1[] = {"abc"};
    unsigned char text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    unsigned char text3[] = {"aaaaaaaaaa"};
    unsigned char hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,
				     0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
	                             0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,
				     0xff,0x61,0xf2,0x00,0x15,0xad};
    
    unsigned char hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,
				     0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
	                             0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,
				     0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
    
    unsigned char hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,
					      0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
	                                 0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,
					      0x39,0xcc,0xc7,0x11,0x2c,0xd0};
    unsigned char buf[SHA256_BLOCK_SIZE];
    struct sha256_ctx ctx;
    int idx;
    int pass = 1;
    
    sha256_init(&ctx);
    sha256_update(&ctx, text1, strlen(text1));
    sha256_final(&ctx, buf);
    pass = pass && !memcmp(hash1, buf, SHA256_BLOCK_SIZE);
    
    sha256_init(&ctx);
    sha256_update(&ctx, text2, strlen(text2));
    sha256_final(&ctx, buf);
    pass = pass && !memcmp(hash2, buf, SHA256_BLOCK_SIZE);
    
    sha256_init(&ctx);
    for (idx = 0; idx < 100000; ++idx)
	    sha256_update(&ctx, text3, strlen(text3));
    sha256_final(&ctx, buf);
    pass = pass && !memcmp(hash3, buf, SHA256_BLOCK_SIZE);  	
    printf("SHA-256 tests: %s\n", pass ? "SUCCEEDED" : "FAILED");

    // Test matching
    int chunk_num = 0;
    char duplicate = 0;
    int index = 0;

    // First entry should not find match
    Matching(buf, historytable, &chunk_num, &duplicate, &index);
    if (duplicate == 1) {
        printf("Matching failed\n");
	return -1;
    }

    duplicate = 0;
    
    //Find Match at index 0
    Matching(buf, historytable, &chunk_num, &duplicate, &index);
    if (duplicate == 0 || index != 0) {
        printf("Matching failed\n");
	return -1;
    }

    duplicate = 0;
    
    // change digest slightly
    unsigned char buf2[SHA256_BLOCK_SIZE];
    memcpy(buf2, buf, SHA256_BLOCK_SIZE);
    buf2[3] ^= 0x55;
    buf2[4] &= 0x0F;

    // Shouldnt find match (index 1)
    Matching(buf2, historytable, &chunk_num, &duplicate, &index);
    if (duplicate == 1) {
        printf("Matching failed\n");
	return -1;
    }

    duplicate = 0;
    index = 5;
   
    // Find another match at index 0
    Matching(buf, historytable, &chunk_num, &duplicate, &index);
    if (duplicate == 0 || index != 0) {
        printf("Matching failed\n");
	return -1;
    }

    printf("Matching stage passes tests\n");
    
    return ret;
}
