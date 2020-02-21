//
//

#ifndef ZCOIN_MTP_H
#define ZCOIN_MTP_H

#endif //ZCOIN_MTP_H

#include "merkle-tree.hpp"

#include <immintrin.h>
#include "../argon2ref/core.h"
#include "../argon2ref/argon2.h"
#include "../argon2ref/thread.h"
#include "../argon2ref/blake2.h"
#include "../argon2ref/blake2-impl.h"
#include "../argon2ref/blamka-round-opt.h"
//#include "merkletree/sha.h"

//#include "openssl\sha.h"

#include "uint256.h"
//#include "serialize.h"
class CBlock;

/* Size of MTP proof */
#ifndef MTP_L
#define MTP_L  16
#endif
#ifndef MTP_PROOF_SIZE
#define  MTP_PROOF_SIZE 1471
#endif
#ifndef MTP_BLOCK_SIZE
#define  MTP_BLOCK_SIZE  140
#endif

/*
void mtp_verify(const char* input, 
	const uint8_t hash_root_mtp[16], 
	const uint64_t block_mtp[MTP_L * 2][128],
	const uint8_t nProofMTP[MTP_L * 3 * 353],
	uint256 pow_limit,
	uint256 *mtpHashValue);
*/

void mtp_verify(const char* input, const uint8_t hash_root_mtp[16] /*, uint32_t nonce */,
	const uint64_t block_mtp[MTP_L * 2][128], const uint8_t nProofMTP[MTP_L * 3 * 353], uint8_t *mtpHashValue);
