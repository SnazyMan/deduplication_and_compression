#ifdef __SDSCC__
#include <ff.h>
#include <sds_lib.h>
#endif
#define INPUT_SIZE 200000000
#define OUTPUT_SIZE 200000000
//#define INPUT_SIZE 399305
//#define OUTPUT_SIZE 399305

int load_data(unsigned char *data);
int store_data(const char *filename, unsigned char *data, unsigned int size);
