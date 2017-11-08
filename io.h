#ifdef __SDSCC__
#include <ff.h>
#include <sds_lib.h>
#endif


int load_data(unsigned char *data);
int store_data(const char *filename, unsigned char *data, unsigned int size);
