#include <sodium.h>

#include "deps/greatest.h"
#include "../src/block.pb-c.h"
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

GREATEST_SUITE(block_suite) {
  RUN_TEST(can_convert_block_to_proto);
}
