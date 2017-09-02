#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

struct InputTransaction {
  int8_t transaction[64];
  uint32_t index;
  uint32_t amount;

  int8_t address[64];
  int8_t signature[64];
};

struct OutputTransaction {
  uint32_t amount;
  int8_t address[64];
};

struct Transaction {
  int8_t id[64];
  uint8_t input_transaction_count;
  uint8_t output_transaction_count;
  struct InputTransaction **input_transactions;
  struct OutputTransaction **output_transactions;
};

struct Block {
  uint32_t index;
  int8_t previous_hash[64];
  int8_t hash[64];
  uint32_t epoch_timestamp;
  uint32_t nonce;
  uint32_t transaction_count;
  struct Transaction **transactions;
};

struct Block *make_block();

int free_block(struct Block *block);

#endif
