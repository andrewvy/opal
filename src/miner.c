#include <stdint.h>
#include <string.h>
#include <time.h>

#include "block.h"
#include "chain.h"
#include "miner.h"

static int IS_MINING = 0;

int start_mining() {
  IS_MINING = 1;

  printf("Started mining...\n");

  while (1) {
    uint8_t *previous_hash = get_current_block_hash();
    struct Block *block = compute_next_block(previous_hash);
    insert_block_into_blockchain(block);
    uint32_t block_height = get_block_height();

    printf("Inserted block #%d\n", block_height);
    print_block(block);
    set_current_block_hash(block->hash);
    free(block);
  }
}

struct Block *compute_next_block(uint8_t *prev_block_hash) {
  uint32_t nonce = 0;
  time_t current_time = time(NULL);

  struct Block *block = make_block();
  memcpy(block->previous_hash, prev_block_hash, 32);
  block->timestamp = current_time;

  hash_block(block);
  while (!valid_block_hash(block)) {
    block->nonce = nonce;

    hash_block(block);
    nonce++;
  }

  return block;
}
