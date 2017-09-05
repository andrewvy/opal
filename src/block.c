#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>

#include "block.h"
#include "block.pb-c.h"

/* Allocates a block for usage.
 *
 * Later to be free'd with `free_block`
 */
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
    uint8_t byte = block->hash[i];
    uint32_t n = 0;

    if (byte <= 0x0F) {
      n = n + 4;
      byte = byte << 4;
    }

    if (byte <= 0x3F) {
      n = n + 2;
      byte = byte << 2;
    }

    if (byte <= 0x7F) {
      n = n + 1;
    }

    current_target += n;
  }

  if (current_target > target) {
    return 1;
  } else {
    return 0;
  }
}

int compute_merkle_root(uint8_t *merkle_root, struct Block *block) {
  uint8_t **hashes = malloc(sizeof(uint8_t) * 32 * block->transaction_count);

  for (int i = 0; i < block->transaction_count; i++) {
    compute_tx_id(hashes[i], block->transactions[i]);
  }

  free(hashes);

  return 0;
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

/*
 * Converts an allocated block to a newly allocated protobuf
 * block struct.
 *
 * Later to be free'd with `free_proto_block`
 */
PBlock *block_to_proto(struct Block *block) {
  PBlock *msg = malloc(sizeof(PBlock));
  pblock__init(msg);

  msg->version = block->version;
  msg->bits = block->bits;

  msg->previous_hash.len = 32;
  msg->previous_hash.data = malloc(sizeof(char) * 32);
  memcpy(msg->previous_hash.data, block->previous_hash, 32);

  msg->hash.len = 32;
  msg->hash.data = malloc(sizeof(char) * 32);
  memcpy(msg->hash.data, block->hash, 32);

  msg->timestamp = block->timestamp;
  msg->nonce = block->nonce;

  msg->merkle_root.len = 32;
  msg->merkle_root.data = malloc(sizeof(char) * 32);
  memcpy(msg->merkle_root.data, block->merkle_root, 32);

  msg->n_transactions = block->transaction_count;
  msg->transactions = malloc(sizeof(PTransaction *) * msg->n_transactions);

  for (int i = 0; i < msg->n_transactions; i++) {
    msg->transactions[i] = transaction_to_proto(block->transactions[i]);
  }

  return msg;
}

int block_to_serialized(uint8_t **buffer, uint32_t *buffer_len, struct Block *block) {
  PBlock *msg = block_to_proto(block);
  *buffer_len = pblock__get_packed_size(msg);

  *buffer = malloc(*buffer_len);
  pblock__pack(msg, *buffer);
  free_proto_block(msg);

  return 0;
}

struct Block *block_from_proto(PBlock *proto_block) {
  struct Block *block = malloc(sizeof(struct Block));

  block->version = proto_block->version;
  block->bits = proto_block->bits;

  memcpy(block->previous_hash, proto_block->previous_hash.data, 32);
  memcpy(block->hash, proto_block->hash.data, 32);
  memcpy(block->merkle_root, proto_block->merkle_root.data, 32);

  block->timestamp = proto_block->timestamp;
  block->nonce = proto_block->nonce;

  // @TODO(vy): unpack transactions into struct
  block->transaction_count = proto_block->n_transactions;
  block->transactions = malloc(sizeof(struct Transaction *) * block->transaction_count);

  for (int i = 0; i < block->transaction_count; i++) {
    block->transactions[i] = transaction_from_proto(proto_block->transactions[i]);
  }

  return block;
}

struct Block *block_from_serialized(uint8_t *buffer, uint32_t buffer_len) {
  PBlock *proto_block = pblock__unpack(NULL, buffer_len, buffer);
  struct Block *block = block_from_proto(proto_block);
  pblock__free_unpacked(proto_block, NULL);

  return block;
}

int free_proto_block(PBlock *proto_block) {
  free(proto_block->previous_hash.data);
  free(proto_block->hash.data);
  free(proto_block->merkle_root.data);

  for (int i = 0; i < proto_block->n_transactions; i++) {
    free_proto_transaction(proto_block->transactions[i]);
  }

  free(proto_block->transactions);
  free(proto_block);

  return 0;
}

/*
 * Frees an allocated block, and its corresponding TXs.
 */
int free_block(struct Block *block) {
  for (int i = 0; i < block->transaction_count; i++) {
    free_transaction(block->transactions[i]);
  }

  if (block->transaction_count > 0) {
    free(block->transactions);
  }

  free(block);

  return 0;
}
