#include "deps/greatest.h"

#include "../src/block.h"
#include "../src/chain.h"

SUITE(chain_suite);

static uint8_t block_hash[32] = {
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04,
  0x01, 0x02, 0x03, 0x04
};

static uint8_t tx_id[32] = {
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01,
  0x04, 0x03, 0x02, 0x01
};

TEST can_insert_block_into_blockchain(void) {
  struct Block *block = make_block();
  memcpy(block->hash, block_hash, 32);
  block->nonce = 123456;
  block->transaction_count = 0;

  insert_block_into_blockchain(block);
  struct Block *block_from_db = get_block_from_blockchain(block->hash);

  ASSERT_MEM_EQ(block->hash, block_hash, 32);
  ASSERT_EQ(block->nonce, 123456);

  free_block(block);

  PASS();
}

TEST inserting_block_into_blockchain_also_inserts_tx(void) {
  struct Transaction *tx = malloc(sizeof(struct Transaction));
  memcpy(tx->id, tx_id, 32);
  tx->txin_count = 0;
  tx->txout_count = 0;
  tx->txins = NULL;
  tx->txouts = NULL;

  struct Block *block = make_block();
  memcpy(block->hash, block_hash, 32);
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  insert_block_into_blockchain(block);
  uint8_t *block_hash_from_tx = get_block_hash_from_tx_id(tx_id);

  if (block_hash_from_tx != NULL) {
    ASSERT_MEM_EQ(block->hash, block_hash_from_tx, 32);
    free_block(block);
    PASS();
  } else {
    free_block(block);
    FAIL();
  }
}

TEST can_get_block_from_tx_id(void) {
  struct Transaction *tx = malloc(sizeof(struct Transaction));
  memcpy(tx->id, tx_id, 32);
  tx->txin_count = 0;
  tx->txout_count = 0;
  tx->txins = NULL;
  tx->txouts = NULL;

  struct Block *block = make_block();
  memcpy(block->hash, block_hash, 32);
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  insert_block_into_blockchain(block);
  struct Block *block_from_db = get_block_from_tx_id(tx_id);

  if (block_from_db != NULL) {
    ASSERT_MEM_EQ(block->hash, block_from_db->hash, 32);
    free_block(block);
    free_block(block_from_db);
    PASS();
  } else {
    free_block(block);
    FAIL();
  }
}

TEST can_delete_block_from_blockchain(void) {
  struct Block *block = make_block();
  memcpy(block->hash, block_hash, 32);

  insert_block_into_blockchain(block);
  struct Block *block_from_db = get_block_from_blockchain(block_hash);

  ASSERT(block_from_db != NULL);

  delete_block_from_blockchain(block_hash);
  struct Block *deleted_block = get_block_from_blockchain(block_hash);

  ASSERT(deleted_block == NULL);

  free_block(block);
  free_block(block_from_db);

  return 0;
}

TEST can_delete_tx_from_index(void) {
  struct Transaction *tx = malloc(sizeof(struct Transaction));
  memcpy(tx->id, tx_id, 32);
  tx->txin_count = 0;
  tx->txout_count = 0;
  tx->txins = NULL;
  tx->txouts = NULL;

  struct Block *block = make_block();
  memcpy(block->hash, block_hash, 32);
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  insert_block_into_blockchain(block);
  struct Block *block_from_db = get_block_from_tx_id(tx_id);

  if (block_from_db != NULL) {
    ASSERT_MEM_EQ(block->hash, block_from_db->hash, 32);

    delete_tx_from_index(tx_id);
    struct Block *deleted_block = get_block_from_tx_id(tx_id);

    ASSERT(deleted_block == NULL);

    free_block(block);
    free_block(block_from_db);

    PASS();
  } else {
    free_block(block);
    FAIL();
  }
}

TEST can_insert_unspent_tx_into_index(void) {
  struct InputTransaction txin = {
    .transaction = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00
    },
    .txout_index = 0
  };

  struct OutputTransaction txout = {
    .amount = 50,
    .address = {
      0x01, 0x3e, 0x46, 0xa5,
      0xc6, 0x99, 0x4e, 0x35,
      0x55, 0x50, 0x1c, 0xba,
      0xc0, 0x7c, 0x06, 0x77
    }
  };

  struct OutputTransaction *txout_p = &txout;
  struct Transaction tx = {
    .txin_count = 0,
    .txout_count = 1,
    .txouts = &txout_p
  };

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(&txin, &tx, pk, sk);

  insert_unspent_tx_into_index(&tx);
  PUnspentTransaction *unspent_tx = get_unspent_tx_from_index(tx.id);

  if (unspent_tx != NULL) {
    ASSERT_MEM_EQ(tx.txouts[0]->address, unspent_tx->unspent_txouts[0]->address.data, 32);
    delete_unspent_tx_from_index(tx.id);
    PUnspentTransaction *deleted_tx = get_unspent_tx_from_index(tx.id);

    ASSERT(deleted_tx == NULL);

    free_proto_unspent_transaction(unspent_tx);

    PASS();
  } else {
    FAIL();
  }
}

TEST inserting_block_into_blockchain_marks_txouts_as_spent(void) {
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
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;

  insert_block_into_blockchain(block);
  PUnspentTransaction *unspent_tx = get_unspent_tx_from_index(tx->id);
  free_block(block);

  if (unspent_tx != NULL) {
    ASSERT(unspent_tx->n_unspent_txouts == 1);
    ASSERT(unspent_tx->unspent_txouts[0]->spent == 0);

    struct InputTransaction *txin_2 = malloc(sizeof(struct InputTransaction));
    struct OutputTransaction *txout_2 = malloc(sizeof(struct OutputTransaction));

    txin_2->txout_index = 0;
    txout_2->amount = 50;
    memcpy(txin_2->transaction, unspent_tx->id.data, 32);
    memcpy(txout_2->address, address, 32);

    struct Transaction *tx_2 = malloc(sizeof(struct Transaction));
    tx_2->txout_count = 1;
    tx_2->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
    tx_2->txouts[0] = txout_2;

    crypto_sign_keypair(pk, sk);
    sign_txin(txin_2, tx_2, pk, sk);

    tx_2->txin_count = 1;
    tx_2->txins = malloc(sizeof(struct InputTransaction *) * 1);
    tx_2->txins[0] = txin_2;
    compute_self_tx_id(tx_2);

    struct Block *block_2 = make_block();
    block_2->transaction_count = 1;
    block_2->transactions = malloc(sizeof(struct Transaction *) * 1);
    block_2->transactions[0] = tx_2;

    insert_block_into_blockchain(block_2);

    PUnspentTransaction *unspent_tx_2 = get_unspent_tx_from_index(unspent_tx->id.data);

    ASSERT(unspent_tx_2 == NULL);

    if (unspent_tx_2 != NULL) {
      free_proto_unspent_transaction(unspent_tx_2);
    }

    free_proto_unspent_transaction(unspent_tx);
    free_block(block_2);

    PASS();
  } else {
    FAIL();
  }
}

GREATEST_SUITE(chain_suite) {
  RUN_TEST(can_insert_block_into_blockchain);
  RUN_TEST(inserting_block_into_blockchain_also_inserts_tx);
  RUN_TEST(can_get_block_from_tx_id);
  RUN_TEST(can_delete_block_from_blockchain);
  RUN_TEST(can_delete_tx_from_index);
  RUN_TEST(can_insert_unspent_tx_into_index);
  RUN_TEST(inserting_block_into_blockchain_marks_txouts_as_spent);
}
