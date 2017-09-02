#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

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

#define INITIAL_DIFFICULTY_BITS 24

#define HALVE_SUBSIDY_AFTER_BLOCKS_NUM = 210000

struct InputTransaction {
  uint8_t transaction[32];
  uint32_t amount;

  uint8_t address[32];
  uint8_t signature[32];
};

struct OutputTransaction {
  uint32_t amount;
  uint8_t address[32];
};

struct Transaction {
  uint8_t id[32];
  uint8_t input_transaction_count;
  uint8_t output_transaction_count;
  struct InputTransaction **input_transactions;
  struct OutputTransaction **output_transactions;
};

struct Block {
  uint8_t version;

  uint8_t previous_hash[32];
  uint8_t hash[32];

  uint32_t epoch_timestamp;
  uint32_t nonce;

  uint32_t transaction_count;
  struct Transaction **transactions;
};

struct Block *make_block();

int free_block(struct Block *block);

int hash_block(struct Block *block);

#endif
