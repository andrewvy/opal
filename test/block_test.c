#include <stdint.h>
#include <sodium.h>

#include "deps/greatest.h"

#include "../src/opal.pb-c.h"
#include "../src/block.h"
#include "../src/transaction.h"

SUITE(block_suite);

TEST can_convert_block_to_proto(void) {
  uint8_t transaction[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  uint8_t address[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };

  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  txin->txout_index = 0;
  txout->amount = 50;
  memcpy(txin->transaction, transaction, 32);
  memcpy(txout->address, address, 32);

  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(txin, tx, pk, sk);

  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;

  struct Block *block = make_block();
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  PBlock *proto_block = block_to_proto(block);

  ASSERT_MEM_EQ(proto_block->hash.data, block->hash, 32);

  free_block(block);
  free_proto_block(proto_block);

  PASS();
}

TEST can_serialize_block(void) {
  uint8_t transaction[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  uint8_t address[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };

  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  txin->txout_index = 0;
  txout->amount = 50;
  memcpy(txin->transaction, transaction, 32);
  memcpy(txout->address, address, 32);

  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(txin, tx, pk, sk);

  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;

  struct Block *block = make_block();
  block->nonce = 50;
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  uint32_t buffer_len = 0;
  uint8_t *buffer = NULL;

  block_to_serialized(&buffer, &buffer_len, block);

  struct Block *deserialized_block = block_from_serialized(buffer, buffer_len);

  ASSERT_EQ(block->version, deserialized_block->version);
  ASSERT_EQ(block->timestamp, deserialized_block->timestamp);
  ASSERT_EQ(block->nonce, deserialized_block->nonce);
  ASSERT_EQ(block->transaction_count, deserialized_block->transaction_count);

  ASSERT_MEM_EQ(block->hash, deserialized_block->hash, 32);
  ASSERT_MEM_EQ(block->previous_hash, deserialized_block->previous_hash, 32);
  ASSERT_MEM_EQ(block->merkle_root, deserialized_block->merkle_root, 32);
  ASSERT_MEM_EQ(txout->address, deserialized_block->transactions[0]->txouts[0]->address, 32);

  free(buffer);
  free_block(block);

  PASS();
}

TEST invalid_block_by_same_tx_hashes(void) {
  uint8_t transaction[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  uint8_t address[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };

  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  txin->txout_index = 0;
  txout->amount = 50;
  memcpy(txin->transaction, transaction, 32);
  memcpy(txout->address, address, 32);

  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(txin, tx, pk, sk);

  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;
  compute_self_tx_id(tx);

  struct Block *block = make_block();
  block->transaction_count = 2;
  block->transactions = malloc(sizeof(struct Transaction *) * 2);
  block->transactions[0] = tx;
  block->transactions[1] = tx;

  ASSERT(valid_block(block) == 0);
  PASS();
}

TEST invalid_block_by_reused_txout(void) {
  uint8_t transaction[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  uint8_t address[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };
  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));
  txin->txout_index = 0;
  txout->amount = 50;
  memcpy(txin->transaction, transaction, 32);
  memcpy(txout->address, address, 32);
  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;
  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(pk, sk);
  sign_txin(txin, tx, pk, sk);
  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;
  compute_self_tx_id(tx);

  uint8_t transaction_2[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  uint8_t address_2[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };
  struct InputTransaction *txin_2 = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout_2 = malloc(sizeof(struct OutputTransaction));
  txin_2->txout_index = 0;
  txout_2->amount = 50;
  memcpy(txin_2->transaction, transaction_2, 32);
  memcpy(txout_2->address, address_2, 32);
  struct Transaction *tx_2 = malloc(sizeof(struct Transaction));
  tx_2->txout_count = 1;
  tx_2->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx_2->txouts[0] = txout_2;
  unsigned char pk_2[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk_2[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(pk_2, sk_2);
  sign_txin(txin_2, tx_2, pk_2, sk_2);
  tx_2->txin_count = 1;
  tx_2->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx_2->txins[0] = txin_2;
  compute_self_tx_id(tx_2);

  struct Block *block = make_block();
  block->transaction_count = 2;
  block->transactions = malloc(sizeof(struct Transaction *) * 2);
  block->transactions[0] = tx;
  block->transactions[1] = tx_2;

  ASSERT(valid_block(block) == 0);
  PASS();
}

TEST invalid_block_by_merkle_hash(void) {
  uint8_t transaction[32] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  uint8_t address[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };
  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));
  txin->txout_index = 0;
  txout->amount = 50;
  memcpy(txin->transaction, transaction, 32);
  memcpy(txout->address, address, 32);
  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;
  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(pk, sk);
  sign_txin(txin, tx, pk, sk);
  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;
  compute_self_tx_id(tx);

  uint8_t transaction_2[32] = {
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00
  };
  uint8_t address_2[32] = {
    0x01, 0x3e, 0x46, 0xa5,
    0xc6, 0x99, 0x4e, 0x35,
    0x55, 0x50, 0x1c, 0xba,
    0xc0, 0x7c, 0x06, 0x77
  };
  struct InputTransaction *txin_2 = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout_2 = malloc(sizeof(struct OutputTransaction));
  txin_2->txout_index = 0;
  txout_2->amount = 50;
  memcpy(txin_2->transaction, transaction_2, 32);
  memcpy(txout_2->address, address_2, 32);
  struct Transaction *tx_2 = malloc(sizeof(struct Transaction));
  tx_2->txout_count = 1;
  tx_2->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx_2->txouts[0] = txout_2;
  unsigned char pk_2[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk_2[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(pk_2, sk_2);
  sign_txin(txin_2, tx_2, pk_2, sk_2);
  tx_2->txin_count = 1;
  tx_2->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx_2->txins[0] = txin_2;
  compute_self_tx_id(tx_2);

  struct Block *block = make_block();
  block->transaction_count = 2;
  block->transactions = malloc(sizeof(struct Transaction *) * 2);
  block->transactions[0] = tx;
  block->transactions[1] = tx_2;

  ASSERT(valid_block(block) == 0);
  PASS();
}

GREATEST_SUITE(block_suite) {
  RUN_TEST(can_convert_block_to_proto);
  RUN_TEST(can_serialize_block);
  RUN_TEST(invalid_block_by_same_tx_hashes);
  RUN_TEST(invalid_block_by_reused_txout);
  RUN_TEST(invalid_block_by_merkle_hash);
}
