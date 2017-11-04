#include <stdint.h>

struct sha256_ctx {
	uint8_t data[64]; 
	uint32_t data_len;
	uint32_t nl;
	uint32_t nh;
	uint32_t state[8]; // h0-h7 : h0 || h1 ... || h7 = digest
};	
	
int sha256(char *input_chunk, int chunk_length, char *digest);
int sha256_init(struct sha256_ctx *ctx);
