#include "sha256.h"
#include <stdio.h>
#include <string.h>

/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))


// Constants defined in section 4.2.2 of FIPS 180
static const uint32_t k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/* 
 * Implementation of the sha256 algorithm
 * Input is a chunk from a file that is sectioned with Rabin Fingerprint
 * Targeted for ZYNQ SoC, more specifically probably HLS
 * Note that ARM is little endian, which is somewhat important
 * for this algorithm
 */
#pragma SDS data access_pattern(input_chunk:SEQUENTIAL)
void sha256(unsigned char input_chunk[8192], unsigned int chunk_length, unsigned char digest[32])
{
	struct sha256_ctx ctx;
	
	// Compute the SHA256 
	sha256_init(&ctx);
	sha256_update(&ctx, input_chunk, chunk_length);
	sha256_final(&ctx, digest);
}

/*
 * Function to initialize the "h" of the algorithm
 * Sets initial hash values as determined in the FIPS specification
 */ 
void sha256_init(struct sha256_ctx *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->h[0] = 0x6a09e667;
	ctx->h[1] = 0xbb67ae85;
	ctx->h[2] = 0x3c6ef372;
	ctx->h[3] = 0xa54ff53a;
	ctx->h[4] = 0x510e527f;
	ctx->h[5] = 0x9b05688c;
	ctx->h[6] = 0x1f83d9ab;
	ctx->h[7] = 0x5be0cd19;
}

/*
 * This function reads in the chunk serially, creates blocks out of the chunks
 * This is OK for a single processor model, but this is an area
 * of optimization
 */ 
void sha256_update(struct sha256_ctx *ctx, const unsigned char *data, size_t len)
{
	int i;

	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

/* Function to preform compression function on message scheulde */ 
void sha256_transform(struct sha256_ctx *ctx, const unsigned char *data)
{
	uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
#pragma HLS unroll
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
#pragma HLS pipeline II=1
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->h[0];
	b = ctx->h[1];
	c = ctx->h[2];
	d = ctx->h[3];
	e = ctx->h[4];
	f = ctx->h[5];
	g = ctx->h[6];
	h = ctx->h[7];

	for (i = 0; i < 64; ++i) {
#pragma HLS pipeline II=1
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->h[0] += a;
	ctx->h[1] += b;
	ctx->h[2] += c;
	ctx->h[3] += d;
	ctx->h[4] += e;
	ctx->h[5] += f;
	ctx->h[6] += g;
	ctx->h[7] += h;
}

/* Function to handle last partial block */
void sha256_final(struct sha256_ctx *ctx, unsigned char hash[32])
{
	uint32_t i;

	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);

		int k;
		for (k = 0; k < 56; k++) {
			ctx->data[k] = 0;
		}
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final h to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->h[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->h[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->h[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->h[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->h[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->h[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->h[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->h[7] >> (24 - i * 8)) & 0x000000ff;
	}
}
