#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>

#include "block.h"

struct Block *make_block() {
  struct Block *block = malloc(sizeof(struct Block));

  block->version = BLOCK_VERSION;
  block->nonce = 0;

  for (int i = 0; i < 32; i++) {
    block->previous_hash[i] = 0x00;
    block->hash[i] = 0x00;
  }

  memcpy(block->merkle_root, &genesis_block.merkle_root, 32);

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
