#include <stdio.h>
#include "io.h"

int load_data(unsigned char *data)
{
	unsigned int size = INPUT_SIZE;
	unsigned int bytes_read;
	
#ifdef __SDSCC__
	FIL file;
	
	FRESULT result = f_open(&file, "testfile.xml", FA_READ);
	if (result != FR_OK) {
		printf("Could not open input file.\n");
		return -1;
	}
	
	result = f_read(&file, data, size, &bytes_read);
	if (result != FR_OK || bytes_read != size) {
		printf("Could not read input file.");
		return -1;
	}
	
	result = f_close(&file);
	if (result != FR_OK) {
		printf("Could not close input file.");
		return -1;
	}
#else
	FILE *file = fopen("./testfile.xml", "r");
	if (file == NULL) {
		printf("Could not open input file.\n");
		return -1;
	}

	bytes_read = fread(data, 1, size, file);
	if (bytes_read != size) {
		printf("Could not read input data\n");
		return -1;
	}
	
	if (fclose(file) != 0) {
		printf("Could not close input file\n");
		return -1;
	}
#endif

	return 0;
}

int store_data(const char *filename, unsigned char *data, unsigned int size)
{
#ifdef __SDSCC__
	FIL file;
	unsigned int bytes_written;
	
	FRESULT result = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	if (result != FR_OK) {
		printf("Could not open output file.\n");
		return -1;
	}
	
	result = f_write(&file, data, size, &bytes_written);
	if (result != FR_OK || bytes_written != size) {
		printf("Could not write output file.");
		return -1;
	}
	
	result = f_close(&file);
	if (result != FR_OK) {
		printf("Could not close output file.\n");
		return -1;
	}
#else
	FILE *file = fopen(filename, "wb");
	if (file == NULL) {
		printf("Could not open input file.\n");
		return -1;
	}
	
	if (fwrite(data, 1, size, file) != size) {
		printf("Could not write output data\n");
		return -1;
	}
	
	if (fclose(file) != 0) {
		printf("Could not close output file\n");
		return -1;
	}
#endif

	return 0;
}
