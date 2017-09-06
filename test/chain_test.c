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
    PASS();
  } else {
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
    PASS();
  } else {
    FAIL();
  }
}

GREATEST_SUITE(chain_suite) {
  RUN_TEST(can_insert_block_into_blockchain);
  RUN_TEST(inserting_block_into_blockchain_also_inserts_tx);
  RUN_TEST(can_get_block_from_tx_id);
}
