#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>

#include "block.h"

#define BLOCK_VERSION 0x01

struct Block *make_block() {
  struct Block *block = malloc(sizeof(struct Block));
  struct Transaction **transactions = malloc(sizeof(struct Transaction) * 1);

  block->version = BLOCK_VERSION;
  block->nonce = 0;

  for (int i = 0; i < 64; i++) {
    block->previous_hash[i] = 0x00;
  }

  for (int i = 0; i < 64; i++) {
    block->hash[i] = 0x00;
  }

  block->epoch_timestamp = 0;
  block->transaction_count = 0;

  return block;
}

int free_block(struct Block *block) {
  free(block->transactions);
  free(block);

  return 0;
}

int hash_block(struct Block *block) {
  uint8_t to_hash[32 + 1 + 1];
  uint32_t position = 0;

  memcpy(to_hash + position, &block->version, 1);
  position += 1;
  memcpy(to_hash + position, &block->nonce, 1);
  position += 1;
  memcpy(to_hash + position, &block->previous_hash, 32);

  // 0x010000000000000000000000000000000000

  crypto_hash_sha256(block->hash, to_hash, 32 + 1 +1);

  return 0;
}
