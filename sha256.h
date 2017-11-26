#include <stdint.h>
#include <stddef.h>

/*
 * Initial implementation referenced from:
 * https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
 * https://github.com/openssl/openssl/blob/master/crypto/sha/sha256.c
 *
 * Tested with test vectors from: https://www.di-mgt.com.au/sha_testvectors.html
 *
 * Naming convention mostly follows openssl hash implementations
 */

#define SHA256_BLOCK_SIZE 32 // 32 Byte digest

struct sha256_ctx {
	uint8_t data[64]; 
	uint32_t datalen;
	unsigned long long bitlen;
	uint32_t nl;
	uint32_t nh;
	uint32_t h[8]; // h0-h7 : h0 || h1 ... || h7 = digest
};	
	
void sha256(unsigned char input_chunk[8192], unsigned int chunk_length, unsigned char digest[32]);
void sha256_init(struct sha256_ctx *ctx);
void sha256_update(struct sha256_ctx *ctx, const unsigned char data[8192], size_t len);
void sha256_transform(struct sha256_ctx *ctx, const unsigned char data[64]);
void sha256_final(struct sha256_ctx *ctx, unsigned char hash[32]);
