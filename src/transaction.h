#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdint.h>
#include <sodium.h>

#define INPUT_TXIN_HEADER_SIZE (32 + 4)

struct InputTransaction {
  // --- Header
  uint8_t transaction[32]; // Previous tx hash/id
  uint32_t txout_index; // Referenced txout index in previous tx
  // ---

  uint8_t signature[crypto_sign_BYTES];
  uint8_t public_key[crypto_sign_PUBLICKEYBYTES];
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

int sign_txin(struct InputTransaction *txin, uint8_t *public_key, uint8_t *secret_key);
int get_txin_header(uint8_t *header, struct InputTransaction *txin);

#endif
