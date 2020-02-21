//
//
//#pragma once 


#include "mtp.h"
//#include "sha3/sph_blake.h"
#include <stdlib.h>
#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h> /* For SecureZeroMemory */
#define _ALIGN(x) __attribute__ ((aligned(x)))
#endif

#include <ios>
#include <stdio.h>
#include <iostream>
#include "../../logging.h"
#if defined __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#include <emmintrin.h> 
#include <immintrin.h>
#include <cstdint>
#include <cstring>


#include "../argon2ref/core.h"
#include "../argon2ref/argon2.h"
#include "../argon2ref/thread.h"
#include "../argon2ref/blake2.h"
#include "../argon2ref/blake2-impl.h"
#include "../argon2ref/blamka-round-opt.h"

//#include "compat/bblake/bblake2b.h"

#ifndef INLINE
#ifdef __GNUC__
#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif
#elif defined(_MSC_VER)
#define INLINE __forceinline
#elif (defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#define bswap_32(x) ((((x) << 24) & 0xff000000u) | (((x) << 8) & 0x00ff0000u) \
                   | (((x) >> 8) & 0x0000ff00u) | (((x) >> 24) & 0x000000ffu))


#define memcost 4*1024*1024
static const unsigned int d_mtp = 1;
static const uint8_t L = 16;
static const unsigned int memory_cost = memcost;

//const int8_t L = 64;
const unsigned T_COST = 1;
const unsigned M_COST = 1024 * 1024 * 4;
const unsigned LANES = 4;


uint32_t index_beta(const argon2_instance_t *instance,
	const argon2_position_t *position, uint32_t pseudo_rand,
	int same_lane) {
	
	uint32_t reference_area_size;
	uint64_t relative_position;
	uint32_t start_position, absolute_position;

	if (0 == position->pass) {
		/* First pass */
		if (0 == position->slice) {
			/* First slice */
			reference_area_size =
				position->index - 1; /* all but the previous */
		}
		else {
			if (same_lane) {
				/* The same lane => add current segment */
				reference_area_size =
					position->slice * instance->segment_length +
					position->index - 1;
			}
			else {
				reference_area_size =
					position->slice * instance->segment_length +
					((position->index == 0) ? (-1) : 0);
			}
		}
	}
	else {
		/* Second pass */
		if (same_lane) {
			reference_area_size = instance->lane_length -
				instance->segment_length + position->index -
				1;
		}
		else {
			reference_area_size = instance->lane_length -
				instance->segment_length +
				((position->index == 0) ? (-1) : 0);
		}
	}

	/* 1.2.4. Mapping pseudo_rand to 0..<reference_area_size-1> and produce
	* relative position */
	relative_position = pseudo_rand;
	relative_position = relative_position * relative_position >> 32;
	relative_position = reference_area_size - 1 -
		(reference_area_size * relative_position >> 32);

	/* 1.2.5 Computing starting position */
	start_position = 0;

	if (0 != position->pass) {
		start_position = (position->slice == ARGON2_SYNC_POINTS - 1)
			? 0
			: (position->slice + 1) * instance->segment_length;
	}

	/* 1.2.6. Computing absolute position */
	absolute_position = (start_position + relative_position) %
		instance->lane_length; /* absolute position */
	return absolute_position;
}

void StoreBlock(void *output, const block *src)
{
	for (unsigned i = 0; i < ARGON2_QWORDS_IN_BLOCK; ++i) {
		store64(static_cast<uint8_t*>(output)
			+ (i * sizeof(src->v[i])), src->v[i]);
	}
}


void compute_blake2b(const block& input,
	uint8_t digest[MERKLE_TREE_ELEMENT_SIZE_B])
{
	blake2b_state state;
	blake2b_init(&state, MERKLE_TREE_ELEMENT_SIZE_B);
	blake2b_4r_update(&state, input.v, ARGON2_BLOCK_SIZE);
	blake2b_4r_final(&state, digest, MERKLE_TREE_ELEMENT_SIZE_B);
}





void getblockindex(uint32_t ij, argon2_instance_t *instance, uint32_t *out_ij_prev, uint32_t *out_computed_ref_block)
{
	uint32_t ij_prev = 0;
	if (ij%instance->lane_length == 0)
		ij_prev = ij + instance->lane_length - 1;
	else
		ij_prev = ij - 1;

	if (ij % instance->lane_length == 1)
		ij_prev = ij - 1;

	uint64_t prev_block_opening = instance->memory[ij_prev].v[0];
	uint32_t ref_lane = (uint32_t)((prev_block_opening >> 32) % instance->lanes);

	uint32_t pseudo_rand = (uint32_t)(prev_block_opening & 0xFFFFFFFF);

	uint32_t Lane = ((ij) / instance->lane_length);
	uint32_t Slice = (ij - (Lane * instance->lane_length)) / instance->segment_length;
	uint32_t posIndex = ij - Lane * instance->lane_length - Slice * instance->segment_length;


	uint32_t rec_ij = Slice*instance->segment_length + Lane *instance->lane_length + (ij % instance->segment_length);

	if (Slice == 0)
		ref_lane = Lane;


	argon2_position_t position = { 0, Lane , (uint8_t)Slice, posIndex };

	uint32_t ref_index = index_beta(instance, &position, pseudo_rand, ref_lane == position.lane);

	uint32_t computed_ref_block = instance->lane_length * ref_lane + ref_index;

	*out_ij_prev = ij_prev;
	*out_computed_ref_block = computed_ref_block;
}




unsigned int trailing_zeros(char str[64]) {


    unsigned int i, d;
    d = 0;
    for (i = 63; i > 0; i--) {
        if (str[i] == '0') {
            d++;
        }
        else {
            break;
        }
    }
    return d;
}


unsigned int trailing_zeros_little_endian(char str[64]) {
	unsigned int i, d;
	d = 0;
	for (i = 0; i < 64; i++) {
		if (str[i] == '0') {
			d++;
		}
		else {
			break;
		}
	}
	return d;
}

unsigned int trailing_zeros_little_endian_uint256(uint256 hash) {
	unsigned int i, d;
	std::string temp = hash.GetHex();
	d = 0;
	for (i = 0; i < temp.size(); i++) {
		if (temp[i] == '0') {
			d++;
		}
		else {
			break;
		}
	}
	return d;
}

static void store_block(void *output, const block *src) {
    unsigned i;
    for (i = 0; i < ARGON2_QWORDS_IN_BLOCK; ++i) {
        store64((uint8_t *)output + i * sizeof(src->v[i]), src->v[i]);
    }
}

void copy_block(block *dst, const block *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}
void copy_blockS(blockS *dst, const blockS *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}
void copy_blockS(blockS *dst, const block *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}


#define VC_GE_2005(version) (version >= 1400)

void  secure_wipe_memory(void *v, size_t n) {
#if defined(_MSC_VER) && VC_GE_2005(_MSC_VER)
	SecureZeroMemory(v, n);
#elif defined memset_s
	memset_s(v, n, 0, n);
#elif defined(__OpenBSD__)
	explicit_bzero(v, n);
#else
	static void *(*const volatile memset_sec)(void *, int, size_t) = &memset;
	memset_sec(v, 0, n);
#endif
}

/* Memory clear flag defaults to true. */

void clear_internal_memory(void *v, size_t n) {
	if (FLAG_clear_internal_memory && v) {
		secure_wipe_memory(v, n);
	}
}


void free_memory(const argon2_context *context, uint8_t *memory,
	size_t num, size_t size) {
	size_t memory_size = num*size;
	clear_internal_memory(memory, memory_size);
	if (context->free_cbk) {
		(context->free_cbk)(memory, memory_size);
	}
	else {
		free(memory);
	}
}

void mtp_verify(const char* input, const uint8_t hash_root_mtp[16] /*, uint32_t nonce */, 
const uint64_t block_mtp[MTP_L * 2][128], const uint8_t nProofMTP[MTP_L * 3 * 353], uint8_t *mtpHashValue)
{

	debuglog("***mtp_verify : checking proof of work***");
	const int8_t L = MTP_L;
	const unsigned T_COST = 1;
	const unsigned M_COST = 1024 * 1024 * 4;
	const unsigned LANES = 4;
	int32_t mtpVersion = 0x1000;
	MerkleTree::Buffer root;
	MerkleTree::Elements proof_blocks[L * 3];
	block blocks[L * 2];
	root.insert(root.begin(), &hash_root_mtp[0], &hash_root_mtp[16]);
	uint8_t digest[16];
	for (int i = 0; i < MTP_L * 3; i++) {
		uint8_t numberOfProofBlocks;
		numberOfProofBlocks = nProofMTP[i*353]; // should be 22 :D 
		for (uint8_t j = 0; j < numberOfProofBlocks; j++) {
			std::memset(digest,0,16);
			std::memcpy(digest,nProofMTP + (353*i+16*j+1),16);
			proof_blocks[i].emplace_back(digest, digest + sizeof(digest));
		}
	}


	for (int i = 0; i < (L * 2); ++i) {
		std::memcpy(blocks[i].v, block_mtp[i],
			sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
	}

#define TEST_OUTLEN 32
#define TEST_PWDLEN 80
#define TEST_SALTLEN 80
#define TEST_SECRETLEN 0
#define TEST_ADLEN 0

/*
			printf("The nonce = %08x \n  Input = ", ((uint32_t*)input)[19]);
			for (int i = 0; i<21; i++)
				printf(" %08x ", ((uint32_t*)input)[i]);

			printf("\n");
*/




	uint32_t nonce = ((uint32_t*)input)[19];
	((uint32_t*)input)[19] = ((uint32_t*)input)[20]; 
	unsigned char out[TEST_OUTLEN];
	unsigned char pwd[TEST_PWDLEN];
	std::memcpy(pwd, input, TEST_PWDLEN);
	unsigned char salt[TEST_SALTLEN];
	std::memcpy(salt, input, TEST_SALTLEN);



	argon2_context context_verify;
	context_verify.out = out;
	context_verify.outlen = TEST_OUTLEN;
	context_verify.version = ARGON2_VERSION_NUMBER;
	context_verify.pwd = pwd;
	context_verify.pwdlen = TEST_PWDLEN;
	context_verify.salt = salt;
	context_verify.saltlen = TEST_SALTLEN;
	context_verify.secret = NULL;
	context_verify.secretlen = TEST_SECRETLEN;
	context_verify.ad = NULL;
	context_verify.adlen = TEST_ADLEN;
	context_verify.t_cost = T_COST;
	context_verify.m_cost = M_COST;
	context_verify.lanes = LANES;
	context_verify.threads = LANES;
	context_verify.allocate_cbk = NULL;
	context_verify.free_cbk = NULL;
	context_verify.flags = ARGON2_DEFAULT_FLAGS;

#undef TEST_OUTLEN
#undef TEST_PWDLEN
#undef TEST_SALTLEN
#undef TEST_SECRETLEN
#undef TEST_ADLEN

	uint32_t memory_blocks = context_verify.m_cost;
	if (memory_blocks < (2 * ARGON2_SYNC_POINTS * context_verify.lanes)) {
		memory_blocks = 2 * ARGON2_SYNC_POINTS * context_verify.lanes;
	}
	uint32_t segment_length = memory_blocks / (context_verify.lanes * ARGON2_SYNC_POINTS);
	memory_blocks = segment_length * (context_verify.lanes * ARGON2_SYNC_POINTS);

	argon2_instance_t instance;
	instance.version = context_verify.version;
	instance.memory = NULL;
	instance.passes = context_verify.t_cost;
	instance.memory_blocks = context_verify.m_cost;
	instance.segment_length = segment_length;
	instance.lane_length = segment_length * ARGON2_SYNC_POINTS;
	instance.lanes = context_verify.lanes;
	instance.threads = context_verify.threads;
	instance.type = Argon2_d;
	if (instance.threads > instance.lanes) {
		instance.threads = instance.lanes;
	}

	// step 7
	uint256 y[L + 1];
	std::memset(&y[0], 0, sizeof(y));

	blake2b_state state_y0;
	blake2b_init(&state_y0, 32); // 256 bit
	blake2b_update(&state_y0, pwd, 80);  // header + mtpversion (not nonce)
	blake2b_update(&state_y0, hash_root_mtp, MERKLE_TREE_ELEMENT_SIZE_B);
	blake2b_update(&state_y0, &nonce, sizeof(unsigned int));
	blake2b_final(&state_y0, &y[0], sizeof(uint256));

	// get hash_zero
	uint8_t h0[ARGON2_PREHASH_SEED_LENGTH];
	initial_hash(h0, &context_verify, instance.type);

	// step 8
	for (uint32_t j = 1; j <= L; ++j) {
		// compute ij
		std::string s = "0x" + y[j - 1].GetHex();
//		boost::multiprecision::uint256_t t(s);
//		uint32_t ij = numeric_cast<uint32_t>(t % M_COST);
//		uint32_t ij = (((uint32_t*)(&Y[j - 1]))[0]) % M_COST;

		uint32_t ij = ((uint32_t*)&y[j-1])[0] % M_COST;
		// retrieve x[ij-1] and x[phi(i)] from proof
		block prev_block, ref_block, t_prev_block, t_ref_block;
		std::memcpy(t_prev_block.v, block_mtp[(j * 2) - 2],
			sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
		std::memcpy(t_ref_block.v, block_mtp[j * 2 - 1],
			sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
		copy_block(&prev_block, &t_prev_block);
		copy_block(&ref_block, &t_ref_block);
		clear_internal_memory(t_prev_block.v, ARGON2_BLOCK_SIZE);
		clear_internal_memory(t_ref_block.v, ARGON2_BLOCK_SIZE);

		//prev_index
		//compute
		uint32_t memory_blocks_2 = M_COST;
		if (memory_blocks_2 < (2 * ARGON2_SYNC_POINTS * LANES)) {
			memory_blocks_2 = 2 * ARGON2_SYNC_POINTS * LANES;
		}

		uint32_t segment_length_2 = memory_blocks_2 / (LANES * ARGON2_SYNC_POINTS);
		uint32_t lane_length = segment_length_2 * ARGON2_SYNC_POINTS;
		uint32_t ij_prev = 0;
		if ((ij % lane_length) == 0) {
			ij_prev = ij + lane_length - 1;
		}
		else {
			ij_prev = ij - 1;
		}
		if ((ij % lane_length) == 1) {
			ij_prev = ij - 1;
		}

		//hash[prev_index]
		uint8_t digest_prev[MERKLE_TREE_ELEMENT_SIZE_B];
		compute_blake2b(prev_block, digest_prev);
		MerkleTree::Buffer hash_prev(digest_prev,
			digest_prev + sizeof(digest_prev));
		if (!MerkleTree::checkProofOrdered(proof_blocks[(j * 3) - 2],
			root, hash_prev, ij_prev + 1)) {
			debuglog("error : checkProofOrdered in x[ij_prev]\n");
			std::memset(mtpHashValue,0xff,32);
			return;
		}

		//compute ref_index
		uint64_t prev_block_opening = prev_block.v[0];
		uint32_t ref_lane = static_cast<uint32_t>((prev_block_opening >> 32) % LANES);
		uint32_t pseudo_rand = static_cast<uint32_t>(prev_block_opening & 0xFFFFFFFF);
		uint32_t lane = ij / lane_length;
		uint32_t slice = (ij - (lane * lane_length)) / segment_length_2;
		uint32_t pos_index = ij - (lane * lane_length)
			- (slice * segment_length_2);
		if (slice == 0) {
			ref_lane = lane;
		}

		argon2_instance_t instance;
		instance.segment_length = segment_length_2;
		instance.lane_length = lane_length;

		argon2_position_t position{ 0, lane , (uint8_t)slice, pos_index };
		uint32_t ref_index = index_beta(&instance, &position, pseudo_rand,
			ref_lane == position.lane);

		uint32_t computed_ref_block = (lane_length * ref_lane) + ref_index;

		uint8_t digest_ref[MERKLE_TREE_ELEMENT_SIZE_B];
		compute_blake2b(ref_block, digest_ref);
		MerkleTree::Buffer hash_ref(digest_ref, digest_ref + sizeof(digest_ref));
		if (!MerkleTree::checkProofOrdered(proof_blocks[(j * 3) - 1],
			root, hash_ref, computed_ref_block + 1)) {
			debuglog("error : checkProofOrdered in x[ij_ref]");
			std::memset(mtpHashValue, 0xff, 32);
			return;
		}

		// compute x[ij]
		block block_ij;
		fill_block_withIndex(&blocks[(j * 2) - 2], &blocks[(j * 2) - 1], &block_ij, 0, (uint32_t*)h0, computed_ref_block);
	
		// verify opening
		// hash x[ij]
		uint8_t digest_ij[MERKLE_TREE_ELEMENT_SIZE_B];
		compute_blake2b(block_ij, digest_ij);
		MerkleTree::Buffer hash_ij(digest_ij, digest_ij + sizeof(digest_ij));

		if (!MerkleTree::checkProofOrdered(proof_blocks[(j * 3) - 3], root,
			hash_ij, ij + 1)) {
			debuglog("error : checkProofOrdered in x[ij]\n");
			std::memset(mtpHashValue, 0xff, 32);
			return;
		}

		// compute y(j)
		block blockhash;
		copy_block(&blockhash, &block_ij);
		uint8_t blockhash_bytes[ARGON2_BLOCK_SIZE];
		StoreBlock(&blockhash_bytes, &blockhash);
		blake2b_state ctx_yj;
		blake2b_init(&ctx_yj, 32);
		blake2b_update(&ctx_yj, &y[j - 1], 32);
		blake2b_update(&ctx_yj, blockhash_bytes, ARGON2_BLOCK_SIZE);
		blake2b_final(&ctx_yj, &y[j], 32);
		clear_internal_memory(block_ij.v, ARGON2_BLOCK_SIZE);
		clear_internal_memory(blockhash.v, ARGON2_BLOCK_SIZE);
		clear_internal_memory(blockhash_bytes, ARGON2_BLOCK_SIZE);
	}
	for (int i = 0; i < (L * 2); ++i) {
		clear_internal_memory(blocks[i].v, ARGON2_BLOCK_SIZE);
	}

/*
	// step 9
	bool negative;
	bool overflow;
	arith_uint256 bn_target;
	bn_target.SetCompact(target, &negative, &overflow); // diff = 1



	if (mtpHashValue)
		*mtpHashValue = y[L];

	if (negative || (bn_target == 0) || overflow
		|| (bn_target > UintToArith256(pow_limit))
		|| (UintToArith256(y[L]) > bn_target)) {
		return false;
	}
	return true;
*/
        memcpy(mtpHashValue,((unsigned char*)&y[L]),32);

}



