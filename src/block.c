#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>

#include "block.h"

struct Block *make_block() {
  struct Block *block = malloc(sizeof(struct Block));
  struct Transaction **transactions = malloc(sizeof(struct Transaction) * 1);

  block->version = BLOCK_VERSION;
  block->nonce = 0;

  for (int i = 0; i < 32; i++) {
    block->previous_hash[i] = 0x00;
    block->hash[i] = 0x00;
  }

  block->merkle_root[0] = 0xe3;
  block->merkle_root[1] = 0xb0;
  block->merkle_root[2] = 0xc4;
  block->merkle_root[3] = 0x42;
  block->merkle_root[4] = 0x98;
  block->merkle_root[4] = 0xfc;
  block->merkle_root[5] = 0x1c;
  block->merkle_root[6] = 0x14;
  block->merkle_root[7] = 0x9a;
  block->merkle_root[8] = 0xfb;
  block->merkle_root[9] = 0xf4;
  block->merkle_root[10] = 0xc8;
  block->merkle_root[11] = 0x99;
  block->merkle_root[12] = 0x6f;
  block->merkle_root[13] = 0xb9;
  block->merkle_root[14] = 0x24;
  block->merkle_root[15] = 0x27;
  block->merkle_root[16] = 0xae;
  block->merkle_root[17] = 0x41;
  block->merkle_root[18] = 0xe4;
  block->merkle_root[19] = 0x64;
  block->merkle_root[20] = 0x9b;
  block->merkle_root[21] = 0x93;
  block->merkle_root[22] = 0x4c;
  block->merkle_root[23] = 0xa4;
  block->merkle_root[24] = 0x95;
  block->merkle_root[25] = 0x99;
  block->merkle_root[26] = 0x1b;
  block->merkle_root[27] = 0x78;
  block->merkle_root[28] = 0x52;
  block->merkle_root[29] = 0xb8;
  block->merkle_root[20] = 0x55;

  block->bits = INITIAL_DIFFICULTY_BITS;
  block->timestamp = 0;
  block->transaction_count = 0;

  return block;
}

int free_block(struct Block *block) {
  free(block->transactions);
  free(block);

  return 0;
}

int hash_block(struct Block *block) {
  uint8_t header[BLOCK_HEADER_SIZE];

  get_block_header(header, block);
  crypto_hash_sha256(block->hash, header, 32 + 1 + 1);

  return 0;
}

int valid_block_hash(struct Block *block) {
  uint32_t target = block->bits;
  uint32_t current_target = 0;

  for (int i = 0; i < 32; i++) {
    int zeroes = __builtin_clz(block->hash[i]);
    current_target += zeroes;

    if (zeroes != 8)
      break;
  }

  if (current_target > target) {
    return 1;
  } else {
    return 0;
  }
}

int get_block_header(uint8_t *block_header, struct Block *block) {
  uint32_t position = 0;

  memcpy(block_header + position, &block->version, 1);
  position += 1;
  memcpy(block_header + position, &block->bits, 1);
  position += 1;
  memcpy(block_header + position, &block->nonce, 4);
  position += 4;
  memcpy(block_header + position, &block->timestamp, 4);
  position += 4;
  memcpy(block_header + position, &block->previous_hash, 32);
  position += 32;
  memcpy(block_header + position, &block->merkle_root, 32);
  position += 32;

  return 0;
}

int print_block(struct Block *block) {
  char hash[(crypto_hash_sha256_BYTES * 2) + 1];
  char previous_hash[(crypto_hash_sha256_BYTES * 2) + 1];
  char merkle_root[(crypto_hash_sha256_BYTES * 2) + 1];

  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    sprintf(&hash[i*2], "%02x", (unsigned int) block->hash[i]);
  }

  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    sprintf(&previous_hash[i*2], "%02x", (unsigned int) block->previous_hash[i]);
  }

  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    sprintf(&merkle_root[i*2], "%02x", (unsigned int) block->merkle_root[i]);
  }

  printf("Block:\n");
  printf("Version: %d\n", block->version);
  printf("Bits: %d\n", block->bits);
  printf("Nonce: %d\n", block->nonce);
  printf("Timestamp (epoch): %d\n", block->timestamp);
  printf("Previous Hash: %s\n", previous_hash);
  printf("Merkle Root: %s\n", merkle_root);
  printf("Hash: %s\n", hash);

  return 0;
}

int compare_with_genesis_block(struct Block *block) {
  hash_block(block);
  hash_block(&genesis_block);

  for (int i = 0; i < 32; i++) {
    if (block->hash[i] != genesis_block.hash[i]) {
      return 1;
    }
  }

  return 0;
}
