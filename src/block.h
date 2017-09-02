#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

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
