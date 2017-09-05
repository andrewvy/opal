#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

#include "transaction.h"
#include "block.pb-c.h"

// MAX_BLOCK_SIZE: Max serialized size of a block (1MB)
#define MAX_BLOCK_SIZE 1000000

// MAX_FUTURE_BLOCK_TIME: How far in the future to accept block timestamps (secs)
#define MAX_FUTURE_BLOCK_TIME (60 * 60 * 2)

// OPALITES_PER_OPAL: How many fractions to an Opal
#define OPALITES_PER_OPAL 100

// TOTAL_OPALS: How many Opals that will ever exist
#define TOTAL_OPALS 200000000

// MAX_MONEY: Maximum number of opal/opalites that will ever exist
#define MAX_MONEY (OPALITES_PER_OPAL * TOTAL_OPALS)

// TIME_BETWEEN_BLOCKS_IN_SECS_TARGET: Target duration between blocks being mined (secs)
#define TIME_BETWEEN_BLOCKS_IN_SECS_TARGET (1 * 60)

// DIFFICULTY_PERIOD_IN_SECS_TARGET: How long difficulty should last (secs)
#define DIFFICULTY_PERIOD_IN_SECS_TARGET (60 * 60 * 10)

// DIFFICULTY_PERIOD_IN_BLOCKS_TARGET: How long difficulty should last (blocks)
#define DIFFICULTY_PERIOD_IN_BLOCKS_TARGET (DIFFICULTY_PERIOD_IN_SECS_TARGET / TIME_BETWEEN_BLOCKS_IN_SECS_TARGET)

#define INITIAL_DIFFICULTY_BITS 85

#define HALVE_SUBSIDY_AFTER_BLOCKS_NUM = 200000

#define BLOCK_HEADER_SIZE (32 + 32 + 4 + 4 + 1 + 1)

#define BLOCK_VERSION 0x01

struct Block {
  uint8_t version;
  uint8_t bits;

  uint8_t previous_hash[32];
  uint8_t hash[32];

  uint32_t timestamp;
  uint32_t nonce;

  uint8_t merkle_root[32];
  uint32_t transaction_count;
  struct Transaction **transactions;
};

struct Block *make_block();

static struct Block genesis_block = {
  .version = BLOCK_VERSION,
  .nonce = 0,
  .timestamp = 1504395525,
  .bits = INITIAL_DIFFICULTY_BITS,
  .previous_hash = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  },
  .merkle_root = {
    0xe3, 0xb0, 0xc4, 0x42,
    0xfc, 0x1c, 0x14, 0x9a,
    0xfb, 0xf4, 0xc8, 0x99,
    0x6f, 0xb9, 0x24, 0x27,
    0xae, 0x41, 0xe4, 0x64,
    0x55, 0x93, 0x4c, 0xa4,
    0x95, 0x99, 0x1b, 0x78,
    0x52, 0xb8, 0x00, 0x00
  },
  .hash = {
    0x40, 0xe2, 0xba, 0xde,
    0x03, 0xe4, 0x0f, 0xf7,
    0xa1, 0x21, 0x20, 0xb3,
    0xc6, 0xa3, 0xfb, 0xd4,
    0xbe, 0x84, 0xde, 0xf4,
    0xbc, 0xea, 0xc2, 0x2f,
    0x4a, 0xd5, 0xd7, 0x7a,
    0x96, 0x5c, 0x0f, 0xfd
  }
};


int free_block(struct Block *block);
int hash_block(struct Block *block);
int get_block_header(uint8_t *block_header, struct Block *block);
int valid_block_hash(struct Block *block);
int print_block(struct Block *block);
int compare_with_genesis_block(struct Block *block);

int compute_merkle_root(uint8_t *merkle_root, struct Block *block);

PBlock *block_to_proto(struct Block *block);
int free_proto_block(PBlock *proto_block);
int block_to_serialized(uint8_t **buffer, uint32_t *buffer_len, struct Block *block);
struct Block *block_from_proto(PBlock *proto_block);
struct Block *block_from_serialized(uint8_t *buffer, uint32_t buffer_len);

#endif
