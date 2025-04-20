/******************************************************************************/
/* This code was developed by another author and made available for free use. */
/* I am only utilizing a part of it in my program.                            */
/* For the original source, refer to the following link:                      */
/* Source: https://github.com/smuellerDD/xdrbg                                */
/******************************************************************************/

// Client-server API(PICO)        //
// DRBG Algorithm 		          //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/******************************************************************************/  
/*                      MODIFICATIONS (Nikita Kuropatkin)                     */  
/*                                                                            */  
/*   #Removed all test code and the main function for usage.                  */  
/*   #Deleted `alpha` and `alphalen`, as they are not used in our system.     */  
/*     The following functions were altered:                                  */  
/*     - `lc_xdrbg256_encode`                                                 */  
/*     - `lc_xdrbg256_drng_fke_init_ctx`                                      */  
/*     - `lc_xdrbg256_drng_generate`                                          */  
/*     - `lc_xdrbg256_drng_seed`                                              */  
/*   #Deleted the `partial` field in the `lc_sha3_256_state` structure and    */  
/*     replaced it with a local definition in `lc_xdrbg256_drng_seed` and     */  
/*     `lc_xdrbg256_drng_generate`. The `partial` field is now of type        */  
/*     `uint64_t`. We needed that modification due to problems with memory    */  
/*     alignment on a processor with an ARM Cortex-M0+ architecture. The      */  
/*     original implementation accessed misaligned memory by casting the      */  
/*     `uint8_t partial` field to `uint64_t`.                                 */
/*   #In `keccak_absorb`, I removed all functions that handled input data     */  
/*     exceeding the state size. I also adjusted the pointer logic for the    */  
/*     new type of `local_partial`.                                           */  
/*   #In `keccak_squeeze`, I added `local_partial` as an argument and         */  
/*     adjusted pointer operations accordingly. Other functions also received */  
/*     the new `local_partial` argument.                                      */  
/*   #Refer to the description and look for "NK" in the comments to find      */  
/*     my modifications.                                                      */  
/*   #Moved the macro and structure definitions to `xdrbg.h` (line 560)       */  
/*     to make them accessible throughout the code. I also created function   */  
/*     prototypes for `lc_xdrbg256_drng_seed` and `lc_xdrbg256_drng_generate`.*/  
/*   #Added wiping of the seed value from the stack after initialization      */  
/*     in `lc_xdrbg256_drng_seed`. Also wiping `partial` at the end of        */  
/*     `lc_xdrbg256_drng_generate` due to security concerns.                  */  
/******************************************************************************/ 

  

/*
 * That code is released under Public Domain
 * (https://creativecommons.org/share-your-work/public-domain/cc0/).
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "include/xdrbg.h"
#include "include/monocypher.h"

static inline size_t min_size(size_t a, size_t b)
{
	return a < b ? a : b;
}

static inline uint64_t rol(uint64_t x, int n)
{
	return ((x << (n & (64 - 1))) | (x >> ((64 - n) & (64 - 1))));
}

static inline void memset_secure(void *s, int c, size_t n)
{
	memset(s, c, n);
	__asm__ __volatile__("" : : "r" (s) : "memory");
}

/*********************************** Keccak ***********************************/
/* state[x + y*5] */
#define A(x, y) (x + 5 * y)
#define RHO_ROL(t) (((t + 1) * (t + 2) / 2) % 64)

static inline void keccakp_theta_rho_pi(uint64_t s[25])
{
	uint64_t C[5], D[5], t;

	/* Steps 1 + 2 */
	C[0] = s[A(0, 0)] ^ s[A(0, 1)] ^ s[A(0, 2)] ^ s[A(0, 3)] ^ s[A(0, 4)];
	C[1] = s[A(1, 0)] ^ s[A(1, 1)] ^ s[A(1, 2)] ^ s[A(1, 3)] ^ s[A(1, 4)];
	C[2] = s[A(2, 0)] ^ s[A(2, 1)] ^ s[A(2, 2)] ^ s[A(2, 3)] ^ s[A(2, 4)];
	C[3] = s[A(3, 0)] ^ s[A(3, 1)] ^ s[A(3, 2)] ^ s[A(3, 3)] ^ s[A(3, 4)];
	C[4] = s[A(4, 0)] ^ s[A(4, 1)] ^ s[A(4, 2)] ^ s[A(4, 3)] ^ s[A(4, 4)];

	D[0] = C[4] ^ rol(C[1], 1);
	D[1] = C[0] ^ rol(C[2], 1);
	D[2] = C[1] ^ rol(C[3], 1);
	D[3] = C[2] ^ rol(C[4], 1);
	D[4] = C[3] ^ rol(C[0], 1);

	/* Step 3 theta and rho and pi */
	s[A(0, 0)] ^= D[0];
	t = rol(s[A(4, 4)] ^ D[4], RHO_ROL(11));
	s[A(4, 4)] = rol(s[A(1, 4)] ^ D[1], RHO_ROL(10));
	s[A(1, 4)] = rol(s[A(3, 1)] ^ D[3], RHO_ROL(9));
	s[A(3, 1)] = rol(s[A(1, 3)] ^ D[1], RHO_ROL(8));
	s[A(1, 3)] = rol(s[A(0, 1)] ^ D[0], RHO_ROL(7));
	s[A(0, 1)] = rol(s[A(3, 0)] ^ D[3], RHO_ROL(6));
	s[A(3, 0)] = rol(s[A(3, 3)] ^ D[3], RHO_ROL(5));
	s[A(3, 3)] = rol(s[A(2, 3)] ^ D[2], RHO_ROL(4));
	s[A(2, 3)] = rol(s[A(1, 2)] ^ D[1], RHO_ROL(3));
	s[A(1, 2)] = rol(s[A(2, 1)] ^ D[2], RHO_ROL(2));
	s[A(2, 1)] = rol(s[A(0, 2)] ^ D[0], RHO_ROL(1));
	s[A(0, 2)] = rol(s[A(1, 0)] ^ D[1], RHO_ROL(0));
	s[A(1, 0)] = rol(s[A(1, 1)] ^ D[1], RHO_ROL(23));
	s[A(1, 1)] = rol(s[A(4, 1)] ^ D[4], RHO_ROL(22));
	s[A(4, 1)] = rol(s[A(2, 4)] ^ D[2], RHO_ROL(21));
	s[A(2, 4)] = rol(s[A(4, 2)] ^ D[4], RHO_ROL(20));
	s[A(4, 2)] = rol(s[A(0, 4)] ^ D[0], RHO_ROL(19));
	s[A(0, 4)] = rol(s[A(2, 0)] ^ D[2], RHO_ROL(18));
	s[A(2, 0)] = rol(s[A(2, 2)] ^ D[2], RHO_ROL(17));
	s[A(2, 2)] = rol(s[A(3, 2)] ^ D[3], RHO_ROL(16));
	s[A(3, 2)] = rol(s[A(4, 3)] ^ D[4], RHO_ROL(15));
	s[A(4, 3)] = rol(s[A(3, 4)] ^ D[3], RHO_ROL(14));
	s[A(3, 4)] = rol(s[A(0, 3)] ^ D[0], RHO_ROL(13));
	s[A(0, 3)] = rol(s[A(4, 0)] ^ D[4], RHO_ROL(12));
	s[A(4, 0)] = t;
}

static const uint64_t keccakp_iota_vals[] = {
	0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
	0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
	0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
	0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
	0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
	0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
	0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
	0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static inline void keccakp_chi_iota(uint64_t s[25], unsigned int round)
{
	uint64_t t0[5], t1[5];

	t0[0] = s[A(0, 0)];
	t0[1] = s[A(0, 1)];
	t0[2] = s[A(0, 2)];
	t0[3] = s[A(0, 3)];
	t0[4] = s[A(0, 4)];

	t1[0] = s[A(1, 0)];
	t1[1] = s[A(1, 1)];
	t1[2] = s[A(1, 2)];
	t1[3] = s[A(1, 3)];
	t1[4] = s[A(1, 4)];

	s[A(0, 0)] ^= ~s[A(1, 0)] & s[A(2, 0)];
	s[A(0, 0)] ^= keccakp_iota_vals[round];
	s[A(0, 1)] ^= ~s[A(1, 1)] & s[A(2, 1)];
	s[A(0, 2)] ^= ~s[A(1, 2)] & s[A(2, 2)];
	s[A(0, 3)] ^= ~s[A(1, 3)] & s[A(2, 3)];
	s[A(0, 4)] ^= ~s[A(1, 4)] & s[A(2, 4)];

	s[A(1, 0)] ^= ~s[A(2, 0)] & s[A(3, 0)];
	s[A(1, 1)] ^= ~s[A(2, 1)] & s[A(3, 1)];
	s[A(1, 2)] ^= ~s[A(2, 2)] & s[A(3, 2)];
	s[A(1, 3)] ^= ~s[A(2, 3)] & s[A(3, 3)];
	s[A(1, 4)] ^= ~s[A(2, 4)] & s[A(3, 4)];

	s[A(2, 0)] ^= ~s[A(3, 0)] & s[A(4, 0)];
	s[A(2, 1)] ^= ~s[A(3, 1)] & s[A(4, 1)];
	s[A(2, 2)] ^= ~s[A(3, 2)] & s[A(4, 2)];
	s[A(2, 3)] ^= ~s[A(3, 3)] & s[A(4, 3)];
	s[A(2, 4)] ^= ~s[A(3, 4)] & s[A(4, 4)];

	s[A(3, 0)] ^= ~s[A(4, 0)] & t0[0];
	s[A(3, 1)] ^= ~s[A(4, 1)] & t0[1];
	s[A(3, 2)] ^= ~s[A(4, 2)] & t0[2];
	s[A(3, 3)] ^= ~s[A(4, 3)] & t0[3];
	s[A(3, 4)] ^= ~s[A(4, 4)] & t0[4];

	s[A(4, 0)] ^= ~t0[0] & t1[0];
	s[A(4, 1)] ^= ~t0[1] & t1[1];
	s[A(4, 2)] ^= ~t0[2] & t1[2];
	s[A(4, 3)] ^= ~t0[3] & t1[3];
	s[A(4, 4)] ^= ~t0[4] & t1[4];
}

static inline void keccakp_1600(uint64_t s[25])
{
	unsigned int round;

	for (round = 0; round < 24; round++) {
		keccakp_theta_rho_pi(s);
		keccakp_chi_iota(s, round);
	}
}

/******************************** SHA / SHAKE *********************************/

#define LC_SHA3_SIZE_BLOCK(bits) ((1600 - 2 * bits) >> 3)
#define LC_SHA3_STATE_WORDS 25

#define LC_SHA3_256_SIZE_DIGEST_BITS 256
#define LC_SHA3_256_SIZE_DIGEST (LC_SHA3_256_SIZE_DIGEST_BITS >> 3)
#define LC_SHA3_256_SIZE_BLOCK LC_SHA3_SIZE_BLOCK(LC_SHA3_256_SIZE_DIGEST_BITS)

/*
#NK  
Deleted the `partial` variable from the state. Now, functions like  
`lc_xdrbg256_drng_seed` and `lc_xdrbg256_drng_generate` define it locally  
and pass it to other functions. Additionally, the `partial` variable is now  
of type `uint64_t` for performance optimization.
*/
struct lc_sha3_256_state {
	uint64_t state[LC_SHA3_STATE_WORDS];
	size_t msg_len;
	size_t digestsize;
	size_t offset;
	unsigned int r;
	unsigned int rword;
	uint8_t padding;
	uint8_t squeeze_more : 1;
};

static inline void sha3_ctx_init(void *_state)
{
	/*
	 * All lc_sha3_*_state are equal except for the last entry, thus we use
	 * the largest state.
	 */
	struct lc_sha3_256_state *ctx = _state;
	unsigned int i;

	/*
	 * Zeroize the actual state which is required by some implementations
	 * like ARM-CE.
	 */
	for (i = 0; i < LC_SHA3_STATE_WORDS; i++)
		ctx->state[i] = 0;

	ctx->msg_len = 0;
	ctx->squeeze_more = 0;
	ctx->offset = 0;
}

static inline void sha3_state_init(uint64_t state[LC_SHA3_STATE_WORDS])
{
	unsigned int i;

	for (i = 0; i < LC_SHA3_STATE_WORDS; i++)
		state[i] = 0;
}

static void shake_256_init_common(void *_state)
{
	struct lc_sha3_256_state *ctx = _state;

	if (!ctx)
		return;

	sha3_ctx_init(_state);
	ctx->r = LC_SHA3_256_SIZE_BLOCK;
	ctx->rword = LC_SHA3_256_SIZE_BLOCK / sizeof(uint64_t);
	ctx->digestsize = 0;
	ctx->padding = 0x1f;
}

static void shake_256_init(void *_state)
{
	struct lc_sha3_256_state *ctx = _state;

	if (!ctx)
		return;

	shake_256_init_common(_state);
	sha3_state_init(ctx->state);
}

#define LC_SHAKE_256_SIZE_DIGEST_BITS 256
#define LC_SHAKE_256_SIZE_BLOCK                                                \
	LC_SHA3_SIZE_BLOCK(LC_SHAKE_256_SIZE_DIGEST_BITS)

#define _lc_swap64(x) (uint64_t) __builtin_bswap64((uint64_t)(x))

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define le_bswap64(x) _lc_swap64(x)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define le_bswap64(x) ((uint64_t)(x))
#else
#error "Endianess not defined"
#endif

static inline uint32_t ptr_to_le32(const uint8_t *p)
{
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
	       (uint32_t)p[3] << 24;
}

static inline uint64_t ptr_to_le64(const uint8_t *p)
{
	return (uint64_t)ptr_to_le32(p) | (uint64_t)ptr_to_le32(p + 4) << 32;
}

static inline void le32_to_ptr(uint8_t *p, const uint32_t value)
{
	p[0] = (uint8_t)(value);
	p[1] = (uint8_t)(value >> 8);
	p[2] = (uint8_t)(value >> 16);
	p[3] = (uint8_t)(value >> 24);
}

static inline void le64_to_ptr(uint8_t *p, const uint64_t value)
{
	le32_to_ptr(p + 4, (uint32_t)(value >> 32));
	le32_to_ptr(p, (uint32_t)(value));
}

static void shake_set_digestsize(struct lc_sha3_256_state *ctx,
				 size_t digestsize)
{
	ctx->digestsize = digestsize;
}

static inline void sha3_fill_state_aligned(struct lc_sha3_256_state *ctx,
					   const uint64_t *in)
{
	unsigned int i;

	for (i = 0; i < ctx->rword; i++) {
		ctx->state[i] ^= le_bswap64(*in);
		in++;
	}
}

/*
#NK  
Removed all functions that handled input data exceeding the state size,  
as our implementation only performs a single seeding.  
The seed size cannot exceed the state size, so those functions were unnecessary.  
Additionally, I added `local_partial` as an argument 
(see the explanation at the beginning).
Since `local_partial` is now of type `uint64_t`, I adjusted the pointer logic  
that previously worked with `uint8_t` for this variable.
*/
static void keccak_absorb(struct lc_sha3_256_state *ctx, const uint8_t *in,
			  size_t inlen, uint64_t *local_partial)
{
	size_t partial;
	if (!ctx)
		return;

	partial = ctx->msg_len % ctx->r;
	ctx->squeeze_more = 0;
	ctx->msg_len += inlen;

	/* Sponge absorbing phase */
	/* Check if we have a partial block stored */
	if (partial) {
		memcpy((uint8_t*)local_partial + partial, in, inlen);
		return;
	}

	/* If we have data left, copy it into the partial block buffer */
	memcpy((uint8_t*)local_partial, in, inlen);

}

/*
#NK
I added `local_partial` as an argument (see the explanation at the beginning).
Since `local_partial` is now of type `uint64_t`, I adjusted the pointer logic  
that previously worked with `uint8_t` for this variable.
*/
static void keccak_squeeze(struct lc_sha3_256_state *ctx, uint8_t *digest,
						   uint64_t *local_partial)
{
	size_t i, digest_len;
uint32_t part;
volatile uint32_t *part_p;

if (!ctx || !digest)
    return;

digest_len = ctx->digestsize;

if (!ctx->squeeze_more) {
    size_t partial = ctx->msg_len % ctx->r;

    /* Final round in sponge absorbing phase */

    /* Fill the unused part of the partial buffer with zeros */
    memset((uint8_t*)local_partial + partial, 0, ctx->r - partial);

    /* Add the padding bits and the 01 bits for the suffix. */
    ((uint8_t*)local_partial)[partial] = ctx->padding;
    ((uint8_t*)local_partial)[ctx->r - 1] |= 0x80;

    ctx->squeeze_more = 1;
/*
#NK  
Changed `sha3_fill_state` to `sha3_fill_state_aligned` due to the change  
in the type of the `partial` variable (`local_partial`).  
This adjustment allows us to use the function on our MCU.
*/
	sha3_fill_state_aligned(ctx, (uint64_t *)local_partial);
	}

	while (digest_len) {
		size_t todo_64, todo_32;

		/* How much data can we squeeze considering current state? */
		size_t todo = ctx->r - ctx->offset;

		/* Limit the data to be squeezed by the requested amount. */
		todo = (digest_len > todo) ? todo : digest_len;

		digest_len -= todo;

		if (ctx->offset) {
			/*
			 * Access requests when squeezing more data that
			 * happens to be not aligned with the block size of
			 * the used SHAKE algorithm are processed byte-wise.
			 */
			size_t word, byte;

			for (i = ctx->offset; i < todo + ctx->offset;
			     i++, digest++) {
				word = i / sizeof(ctx->state[0]);
				byte = (i % sizeof(ctx->state[0])) << 3;

				*digest = (uint8_t)(ctx->state[word] >> byte);
			}

			/* Advance the offset */
			ctx->offset += todo;
			/* Wrap the offset at block size */
			ctx->offset %= ctx->r;
			continue;
		}

		/*
		 * Access to obtain blocks without offset are implemented
		 * with streamlined memory access.
		 */

		/* Generate new keccak block */
		keccakp_1600(ctx->state);

		/* Advance the offset */
		ctx->offset += todo;
		/* Wrap the offset at block size */
		ctx->offset %= ctx->r;

		/* How much 64-bit aligned data can we obtain? */
		todo_64 = todo >> 3;

		/* How much 32-bit aligned data can we obtain? */
		todo_32 = (todo - (todo_64 << 3)) >> 2;

		/* How much non-aligned do we have to obtain? */
		todo -= ((todo_64 << 3) + (todo_32 << 2));

		/* Sponge squeeze phase */

		/* 64-bit aligned request */
		for (i = 0; i < todo_64; i++, digest += 8)
			le64_to_ptr(digest, ctx->state[i]);

		if (todo_32) {
			/* 32-bit aligned request */
			le32_to_ptr(digest, (uint32_t)(ctx->state[i]));
			digest += 4;
			part = (uint32_t)(ctx->state[i] >> 32);
		} else {
			/* non-aligned request */
			part = (uint32_t)(ctx->state[i]);
		}

		for (i = 0; i < todo << 3; i += 8, digest++)
			*digest = (uint8_t)(part >> i);
	}

	/* Zeroization */
	part_p = &part;
	*part_p = 0;
}

/****************************** XDRBG Definitions *****************************/

/*
NK: moved to xdrbg.h for usage within my code
#define LC_XDRBG256_DRNG_KEYSIZE 64

struct lc_xdrbg256_drng_state {
	uint8_t initially_seeded;
	uint8_t v[LC_XDRBG256_DRNG_KEYSIZE];
};
*/
#define LC_XDRBG256_DRNG_ENCODE_N(x) (x * 85)

/* maxout as defined in XDRBG specification */
#define LC_XDRBG256_DRNG_MAX_CHUNK (LC_SHAKE_256_SIZE_BLOCK * 2)

/******************************** XDRBG Helper ********************************/

static inline void lc_xdrbg256_shake_final(struct lc_sha3_256_state *shake_ctx,
					   uint8_t *digest, size_t digest_len, 
					   uint64_t *local_partial)
{
	shake_set_digestsize(shake_ctx, digest_len);
	keccak_squeeze(shake_ctx, digest, local_partial);
}

/* Maximum size of the input data to calculate the encode value */
#define LC_XDRBG256_DRNG_ENCODE_LENGTH 84

/*
 * The encoding is based on the XDRBG paper appendix B.2 with the following
 * properties:
 *
 *   * length of the hash is set to be equal to |V|
 */
/*
#NK  
Added `local_partial` as an argument to this function.
*/
static void lc_xdrbg256_encode(struct lc_sha3_256_state *shake_ctx,
			       const uint8_t n, uint64_t *local_partial
				   )
{
	uint8_t encode;

	/*
	 * Only consider up to 84 left-most bytes of alpha. According to
	 * the XDRBG specification appendix B:
	 *
	 * """
	 * This encoding is efficient and flexible, but does require that the
	 * additional input string is no longer than 84 bytes–a constraint that
	 * seems very easy to manage in practice.
	 *
	 * For example, IPV6 addresses and GUIDs are 16 bytes long, Ethernet
	 * addresses are 12 bytes long, and the most demanding requirement for
	 * unique randomly-generated device identifiers can be met with a
	 * 32-byte random value. This is the encoding we recommend for XDRBG.
	 * """
	 */

	/* Encode the length. */
	encode = (uint8_t)(n);

	/* Insert alpha and encode into the hash context. */
	//keccak_absorb(shake_ctx, alpha, alphalen, local_partial);
	keccak_absorb(shake_ctx, &encode, 1, local_partial);

	/*
	 * Zeroization of encode is not considered to be necessary as alpha is
	 * considered to be known string.
	 */
}

/*
 * Fast-key-erasure initialization of the SHAKE context. The caller must
 * securely dispose of the initialized SHAKE context. Additional data
 * can be squeezed from the state using lc_hash_final.
 *
 * This function initializes the SHAKE context that can later be used to squeeze
 * random bits out of the SHAKE context. The initialization happens from the key
 * found in the state. Before any random bits can be created, the first 512
 * output bits that are generated is used to overwrite the key. This implies
 * an automatic backtracking resistance as the next round to generate random
 * numbers uses the already updated key.
 *
 * When this function completes, initialized SHAKE context can now be used
 * to generate random bits.
 */
 /*
 #NK  
 Added `local_partial` as an argument to this function.
 */
static void lc_xdrbg256_drng_fke_init_ctx(struct lc_xdrbg256_drng_state *state,
					  struct lc_sha3_256_state *shake_ctx, 
					  uint64_t *local_partial
					  )
{
	shake_256_init(shake_ctx);

	/* Insert V' into the SHAKE */
	keccak_absorb(shake_ctx, state->v, LC_XDRBG256_DRNG_KEYSIZE,local_partial);

	/* Insert alpha into the SHAKE state together with its encoding. */
	lc_xdrbg256_encode(shake_ctx, LC_XDRBG256_DRNG_ENCODE_N(2), local_partial);

	/* Generate the V to store in the state and overwrite V'. */
	lc_xdrbg256_shake_final(shake_ctx, state->v, LC_XDRBG256_DRNG_KEYSIZE, 
				local_partial);
}

/********************************** XDRB256 ***********************************/

/*
 * Generating random bits is performed by initializing a transient SHAKE state
 * with the key found in state. The initialization implies that the key in
 * the state variable is already updated before random bits are generated.
 *
 * The random bits are generated by performing a SHAKE final operation. The
 * generation operation is chunked to ensure that the fast-key-erasure updates
 * the key when large quantities of random bits are generated.
 *
 * This function implements the following functions from Algorithm 2  of the
 * XDRBG specification:
 *
 *   * GENERATE
 */
/*
#NK  
Created a local variable `local_partial`, which functions similarly to the 
`partial` field in the original state structure, except that its type has 
been changed to `uint64_t`.
*/
int lc_xdrbg256_drng_generate(struct lc_xdrbg256_drng_state *state,
				     uint8_t *out, size_t outlen
					)
{
	struct lc_sha3_256_state shake_ctx = { 0 };
	uint64_t local_partial[LC_SHA3_256_SIZE_BLOCK / sizeof(uint64_t)]; //#NK
	if (!state)
		return -EINVAL;

	while (outlen) {
		size_t todo = min_size(outlen, LC_XDRBG256_DRNG_MAX_CHUNK);

		/*
		 * Instantiate SHAKE with V', and alpha with its encoding,
		 * and generate V.
		 */
		lc_xdrbg256_drng_fke_init_ctx(state, &shake_ctx, local_partial);

		/* Generate the requested amount of output bits */
		lc_xdrbg256_shake_final(&shake_ctx, out, todo, local_partial);

		out += todo;
		outlen -= todo;
	}

	/* Clear the SHAKE state which is not needed any more. */
	memset_secure(&shake_ctx, 0, sizeof(shake_ctx));

	/* #NK Wiping `partial` due to security concerns */
	crypto_wipe(local_partial,LC_SHA3_256_SIZE_BLOCK / sizeof(uint64_t));

	return 0;
}

/*
 * The DRNG is seeded by initializing a fast-key-erasure SHAKE context and add
 * the key into the SHAKE state. The SHAKE final operation replaces the key in
 * state.
 *
 * This function implements the following functions from Algorithm 2 of the
 * XDRBG specification:
 *
 *  * INSTANTIATE: The state is empty (either freshly allocated or zeroized with
 *                 lc_xdrbg256_drng_zero). In particular state->initially_seeded
 *                 is 0.
 *
 *  * RESEED: The state contains a working XDRBG state that was seeded before.
 *            In this case, state->initially_seeded is 1.
 */
/*
#NK  
Created a local variable `local_partial`, which functions similarly to the 
`partial` field in the original state structure, except that its type has 
been changed to `uint64_t`.
*/

int lc_xdrbg256_drng_seed(struct lc_xdrbg256_drng_state *state,
				 uint8_t *seed,
				 size_t seedlen 
				 )
{
	uint8_t intially_seeded = state->initially_seeded;
	struct lc_sha3_256_state shake_ctx = { 0 };
	uint64_t local_partial[LC_SHA3_256_SIZE_BLOCK / sizeof(uint64_t)]; //#NK

	if (!state)
		return -EINVAL;

	shake_256_init(&shake_ctx);

	/*
	 * During reseeding, insert V' into the SHAKE state. During initial
	 * seeding, V' does not yet exist and thus is not considered.
	 */
	if (intially_seeded)
		keccak_absorb(&shake_ctx, state->v, LC_XDRBG256_DRNG_KEYSIZE, local_partial);
	else
		state->initially_seeded = 1;

	/* Insert the seed data into the SHAKE state. */
	keccak_absorb(&shake_ctx, seed, seedlen, local_partial);

	/* #NK Wiping seed from stack due to security concerns*/
	crypto_wipe(seed, seedlen);

	/* Insert alpha into the SHAKE state together with its encoding. */
	lc_xdrbg256_encode(&shake_ctx,
			   LC_XDRBG256_DRNG_ENCODE_N(intially_seeded), local_partial);

	/* Generate the V to store in the state and overwrite V'. */
	lc_xdrbg256_shake_final(&shake_ctx, state->v, LC_XDRBG256_DRNG_KEYSIZE, local_partial);

	/* Clear the SHAKE state which is not needed any more. */
	memset_secure(&shake_ctx, 0, sizeof(shake_ctx));

	return 0;
}
