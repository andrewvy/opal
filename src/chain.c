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
  leveldb_free(options);

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

  for (int i = 0; i < block->transaction_count; i++) {
    insert_tx_into_index(key, block->transactions[i]);
  }

  if (err != NULL) {
    fprintf(stderr, "Could not insert block into blockchain: %s\n", err);

    leveldb_free(err);
    leveldb_free(woptions);

    return 0;
  }

  leveldb_free(err);
  leveldb_free(woptions);

  return 1;
}

struct Block *get_block_from_blockchain(uint8_t *block_hash) {
  char *err = NULL;
  uint8_t key[33];

  key[0] = 'b';
  for (int i = 0; i < 32; i++) {
    key[i + 1] = block_hash[i];
  }

  size_t read_len;
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  uint8_t *serialized_block = (uint8_t *) leveldb_get(db, roptions, (char *) key, 33, &read_len, &err);

  if (err != NULL || serialized_block == NULL) {
    fprintf(stderr, "Could not retrieve block from blockchain: %s\n", err);

    leveldb_free(err);
    leveldb_free(roptions);
    return NULL;
  }

  struct Block *block = block_from_serialized(serialized_block, read_len);

  leveldb_free(serialized_block);
  leveldb_free(err);
  leveldb_free(roptions);

  return block;
}

int insert_tx_into_index(uint8_t *block_key, struct Transaction *tx) {
  char *err = NULL;
  uint8_t key[33];

  key[0] = 't';
  for (int i = 0; i < 32; i++) {
    key[i + 1] = tx->id[i];
  }

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, (char *) key, 33, (char *) block_key, 33, &err);

  if (err != NULL) {
    fprintf(stderr, "Could not insert tx into blockchain: %s\n", err);
    return 1;
  }

  leveldb_free(err);
  leveldb_free(woptions);

  return 0;
}

uint8_t *get_block_hash_from_tx_id(uint8_t *tx_id) {
  char *err = NULL;
  uint8_t key[33];

  key[0] = 't';
  for (int i = 0; i < 32; i++) {
    key[i + 1] = tx_id[i];
  }

  size_t read_len;
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  uint8_t *block_key = (uint8_t *) leveldb_get(db, roptions, (char *) key, 33, &read_len, &err);

  if (err != NULL || block_key == NULL) {
    fprintf(stderr, "Could not retrieve block from tx id\n");

    leveldb_free(err);
    leveldb_free(roptions);
    return NULL;
  }

  leveldb_free(err);
  leveldb_free(roptions);

  uint8_t *block_hash = malloc(sizeof(uint8_t) * 32);
  memcpy(block_hash, block_key + 1, 32);

  leveldb_free(block_key);

  return block_hash;
}

struct Block *get_block_from_tx_id(uint8_t *tx_id) {
  uint8_t *block_hash = get_block_hash_from_tx_id(tx_id);

  if (block_hash == NULL) {
    return NULL;
  }

  return get_block_from_blockchain(block_hash);
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
