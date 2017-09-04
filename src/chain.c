#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <leveldb/c.h>

#include "block.h"
#include "chain.h"

static int IS_BLOCKCHAIN_OPEN = 0;
static leveldb_t *db;

int open_blockchain() {
  if (IS_BLOCKCHAIN_OPEN) {
    return 0;
  }

  char *err = NULL;
  leveldb_options_t *options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);
  db = leveldb_open(options, "blockchain", &err);

  if (err != NULL) {
    fprintf(stderr, "Could not open blockchain db %s\n", err);
    return 1;
  }

  IS_BLOCKCHAIN_OPEN = 1;
  leveldb_free(err);

  return 0;
}

int close_blockchain() {
  if (IS_BLOCKCHAIN_OPEN) {
    leveldb_close(db);
    IS_BLOCKCHAIN_OPEN = 0;
    return 0;
  } else {
    return 0;
  }
}

int init_blockchain() {
  open_blockchain();

  return 0;
}

int insert_block_into_blockchain(struct Block *block) {
  uint8_t key[33];

  key[0] = 'b';
  for (int i = 0; i < 32; i++) {
    key[i + 1] = block->hash[i];
  }

  return 0;
}

/*
 * This function gets the block height by iterating all keys in the blockchain db.
 * All blocks get prefixed with "b + <block_hash>".
 *
 * For the sake of dev time, only blocks in the db are valid + main chain.
 */
uint32_t get_block_height() {
  uint32_t block_height = 0;

  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  leveldb_iterator_t *iterator = leveldb_create_iterator(db, roptions);

  for (leveldb_iter_seek(iterator, "b", 1); leveldb_iter_valid(iterator); leveldb_iter_next(iterator)) {
    block_height++;
  }

  leveldb_readoptions_destroy(roptions);
  leveldb_iter_destroy(iterator);

  return block_height;
}
