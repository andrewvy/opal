#include "deps/greatest.h"

#include "../src/block.h"
#include "../src/chain.h"

SUITE(chain_suite);

TEST can_insert_block_into_blockchain(void) {
  uint8_t hash[32] = {
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x02, 0x03, 0x04
  };

  struct Block *block = make_block();
  memcpy(block->hash, hash, 32);
  block->nonce = 123456;
  block->transaction_count = 0;

  insert_block_into_blockchain(block);
  struct Block *block_from_db = get_block_from_blockchain(block->hash);

  ASSERT_MEM_EQ(block->hash, hash, 32);
  ASSERT_EQ(block->nonce, 123456);

  close_blockchain();
  free_block(block);

  PASS();
}

GREATEST_SUITE(chain_suite) {
  RUN_TEST(can_insert_block_into_blockchain);
}
