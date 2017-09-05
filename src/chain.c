#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <leveldb/c.h>

#include "block.h"
#include "chain.h"

static uint8_t current_block_hash[32];
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

  insert_block_into_blockchain(&genesis_block);
  set_current_block_hash(genesis_block.hash);

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
  char *err = NULL;
  uint8_t key[33];

  key[0] = 'b';
  for (int i = 0; i < 32; i++) {
    key[i + 1] = block->hash[i];
  }

  uint8_t *buffer = NULL;
  uint32_t buffer_len = 0;

  block_to_serialized(&buffer, &buffer_len, block);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, (char *) key, 33, (char *) buffer, buffer_len, &err);

  free(buffer);

  if (err != NULL) {
    fprintf(stderr, "Could not insert block into blockchain: %s\n", err);
    return 1;
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

uint8_t *get_current_block_hash() {
  return current_block_hash;
}

int set_current_block_hash(uint8_t *hash) {
  memcpy(current_block_hash, hash, 32);
  return 0;
}
