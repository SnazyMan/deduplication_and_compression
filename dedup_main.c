#include "sha256.h"
#include "lzw.h"
#include "ContentDefinedChunk.h"
#include "io.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

//#define SHA_TEST 
//#define ChunkDefine_TEST 
//#define LZW_TEST

int main()
{
	int ret = 0;

// Mount FAT filesystem on SD card
#ifdef __SDSCC__
	FATFS FS;
	ret = f_mount(&FS, "0:/", 0);
	if (ret != FR_OK) {
		printf("Could not mount SD-card\n");
		return -1;
	}
#endif

#ifdef ChunkDefine_TEST
    int InputLength = 300000;
    FILE * testfile;
    testfile = fopen("/Users/koutsutomushiba/Desktop/chunktest/testfile.xml","r");
    FILE * ChunkLengthtest;
    unsigned char Input[InputLength];
    fread(Input,sizeof(unsigned char),InputLength,testfile);
    int chunklength[1000];
    ChunkLengthtest = fopen("/Users/koutsutomushiba/Desktop/chunktest/ChunkLength.xml","wt");
    ContentDefinedChunk(Input,chunklength);
    /*
    for(int i=0;i<100;i++){
        fprintf(ChunkLengthtest,"%d\n",chunklength[i]);
        printf("%d\n", chunklength[i]);
    }
     */
#endif
	
#ifdef SHA_TEST
	int pass = 1;
	unsigned char hash1[SHA256_BLOCK_SIZE] = {
		0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,
		0x5d,0xae,0x22,0x23,
		0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,
		0xf2,0x00,0x15,0xad
	};
	unsigned char *chunk1 = {"abc"};
	unsigned char digest_out[SHA256_BLOCK_SIZE];
	ret = sha256(chunk1, strlen(chunk1), digest_out);
	pass = pass && !memcmp(hash1, digest_out, SHA256_BLOCK_SIZE);
	printf("SHA-256 tests: %s\n", pass ? "SUCCEEDED" : "FAILED");		
#endif	

#ifdef LZW_TEST
//    int input_length = 300000;
//    FILE * test_file;
//    test_file = fopen("./test.txt","r");
//    FILE * lzw_test;
//    unsigned char input[input_length];
//    unsigned char output[input_length];
//    fread(input,sizeof(unsigned char),input_length,test_file);
	unsigned char *input = "the rain in Spain falls mainly on the plain";
	unsigned char output[100];
	memset(output, 0, 100);
	lzw(input, output, strlen(input));

#ifdef __SDSCC__

	store_data("OUT.bin", output, 46);

#else	
	int fd = open("compressed.txt", O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("Couldnt open file\n");
		return -1;
	}

	int wbytes = write(fd, output, 46 * sizeof(char));
	if (wbytes != (46 * sizeof(char))) {
		printf("Couldn't write file");
		return -1;
	}
#endif
#endif
	return ret;

}
