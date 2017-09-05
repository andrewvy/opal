#include <stdint.h>
#include <sodium.h>

#include "deps/greatest.h"
#include "../src/merkle.h"

SUITE(merkle_suite);

/*
 * A: 06f961b802bc46ee168555f066d28f4f0e9afdf3f88174c1ee6f9de004fc30a0
 * B: c0cde77fa8fef97d476c10aad3d2d54fcc2f336140d073651c2dcccf1e379fd6
 * C: 12f37a8a84034d3e623d726fe10e5031f4df997ac13f4d5571b5a90c41fb84fe
 *
 */

TEST can_construct_merkle_tree(void) {
  uint8_t hash_size = 32;
  uint32_t number_of_hashes = 3;
  uint8_t *hash_region = malloc(sizeof(uint8_t) * hash_size * number_of_hashes);

  uint8_t hash_a[32] = {
    0x06, 0xf9, 0x61, 0xb8,
    0x02, 0xbc, 0x46, 0xee,
    0x16, 0x85, 0x55, 0xf0,
    0x66, 0xd2, 0x8f, 0x4f,
    0x0e, 0x9a, 0xfd, 0xf3,
    0xf8, 0x81, 0x74, 0xc1,
    0xee, 0x6f, 0x9d, 0xe0,
    0x04, 0xfc, 0x30, 0xa0
  };

  uint8_t hash_b[32] = {
    0xc0, 0xcd, 0xe7, 0x7f,
    0xa8, 0xfe, 0xf9, 0x7d,
    0x47, 0x6c, 0x10, 0xaa,
    0xd3, 0xd2, 0xd5, 0x4f,
    0xcc, 0x2f, 0x33, 0x61,
    0x40, 0xd0, 0x73, 0x65,
    0x1c, 0x2d, 0xcc, 0xcf,
    0x1e, 0x37, 0x9f, 0xd6
  };

  uint8_t hash_c[32] = {
    0x12, 0xf3, 0x7a, 0x8a,
    0x84, 0x03, 0x4d, 0x3e,
    0x62, 0x3d, 0x72, 0x6f,
    0xe1, 0x0e, 0x50, 0x31,
    0xf4, 0xdf, 0x99, 0x7a,
    0xc1, 0x3f, 0x4d, 0x55,
    0x71, 0xb5, 0xa9, 0x0c,
    0x41, 0xfb, 0x84, 0xfe
  };

  uint8_t root_hash[32] = {
    0xb4, 0xc6, 0xbc, 0xf1,
    0x50, 0xb6, 0x2b, 0x09,
    0x3f, 0x5e, 0x43, 0x9a,
    0x3d, 0xf5, 0x77, 0xd8,
    0x25, 0xdf, 0x04, 0x11,
    0x56, 0xce, 0x94, 0xf4,
    0x41, 0xf1, 0x43, 0xaf,
    0x49, 0x2f, 0x73, 0x5a
  };


  memcpy(hash_region, hash_a, 32);
  memcpy(hash_region + 32, hash_b, 32);
  memcpy(hash_region + 64, hash_c, 32);

  struct MerkleTree *tree = construct_merkle_tree_from_leaves(hash_region, 3);

  uint8_t *hash_ab = malloc(sizeof(uint8_t) * 32);
  uint8_t *hash_cc = malloc(sizeof(uint8_t) * 32);
  uint8_t *hash_abcc = malloc(sizeof(uint8_t) * 32);

  uint8_t *hash_buffer = malloc(sizeof(uint8_t) * 32 * 2);

  memcpy(hash_buffer, hash_a, 32);
  memcpy(hash_buffer + 32, hash_b, 32);
  crypto_hash_sha256(hash_ab, hash_buffer, 64);

  memcpy(hash_buffer, hash_c, 32);
  memcpy(hash_buffer + 32, hash_c, 32);
  crypto_hash_sha256(hash_cc, hash_buffer, 64);

  memcpy(hash_buffer, hash_ab, 32);
  memcpy(hash_buffer + 32, hash_cc, 32);
  crypto_hash_sha256(hash_abcc, hash_buffer, 64);

  ASSERT_MEM_EQ(tree->root->hash, hash_abcc, 32);

  free(hash_ab);
  free(hash_cc);
  free(hash_abcc);
  free(hash_buffer);
  free(hash_region);

  PASS();
}

GREATEST_SUITE(merkle_suite) {
  RUN_TEST(can_construct_merkle_tree);
}
