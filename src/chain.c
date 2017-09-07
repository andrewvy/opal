#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <leveldb/c.h>

#include "block.pb-c.h"
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

/* After we insert block into blockchain
 * Mark unspent txouts as spent for current txins
 * Add current TX w/ unspent txouts to unspent index
 */
int insert_block_into_blockchain(struct Block *block) {
  char *err = NULL;
  uint8_t key[33];
  get_block_key(key, block->hash);

  uint8_t *buffer = NULL;
  uint32_t buffer_len = 0;

  block_to_serialized(&buffer, &buffer_len, block);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, (char *) key, 33, (char *) buffer, buffer_len, &err);

  free(buffer);

  for (int i = 0; i < block->transaction_count; i++) {
    struct Transaction *tx = block->transactions[i];

    insert_tx_into_index(key, tx);
    insert_unspent_tx_into_index(tx);

    if (is_generation_tx(tx)) {
      continue;
    }

    // Mark unspent txouts as spent for current txins
    for (int txin_index = 0; txin_index < tx->txin_count; txin_index++) {
      struct InputTransaction *txin = tx->txins[txin_index];
      PUnspentTransaction *unspent_tx = get_unspent_tx_from_index(txin->transaction);

      if (((unspent_tx->n_unspent_txouts - 1) < txin->txout_index) || unspent_tx->unspent_txouts[txin->txout_index] == NULL) {
        free_proto_unspent_transaction(unspent_tx);
        fprintf(stderr, "A txin tried to mark a unspent txout as spent, but it was not found\n");
        continue;
      } else {
        PUnspentOutputTransaction *unspent_txout = unspent_tx->unspent_txouts[txin->txout_index];
        if (unspent_txout->spent == 1) {
          free_proto_unspent_transaction(unspent_tx);
          fprintf(stderr, "A txin tried to mark a unspent txout as spent, but it was already spent\n");
          continue;
        }

        unspent_txout->spent = 1;

        int spent_txs = 0;
        for (int j = 0; j < unspent_tx->n_unspent_txouts; j++) {
          if (unspent_txout->spent == 1)
            spent_txs++;
        }

        if (spent_txs == unspent_tx->n_unspent_txouts) {
          delete_unspent_tx_from_index(unspent_tx->id.data);
        } else {
          insert_proto_unspent_tx_into_index(unspent_tx);
        }

        free_proto_unspent_transaction(unspent_tx);
      }
    }
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
  get_block_key(key, block_hash);

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
  get_tx_key(key, tx->id);

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

int insert_unspent_tx_into_index(struct Transaction *tx) {
  char *err = NULL;
  uint8_t key[33];
  get_unspent_tx_key(key, tx->id);

  uint8_t *buffer = NULL;
  uint32_t buffer_len = 0;
  unspent_transaction_to_serialized(&buffer, &buffer_len, tx);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, (char *) key, 33, (char *) buffer, buffer_len, &err);

  free(buffer);

  if (err != NULL) {
    fprintf(stderr, "Could not insert tx into blockchain: %s\n", err);
    return 1;
  }

  leveldb_free(err);
  leveldb_free(woptions);

  return 0;
}

int insert_proto_unspent_tx_into_index(PUnspentTransaction *tx) {
  char *err = NULL;
  uint8_t key[33];
  get_unspent_tx_key(key, tx->id.data);

  uint8_t *buffer = NULL;
  uint32_t buffer_len = 0;
  proto_unspent_transaction_to_serialized(&buffer, &buffer_len, tx);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, (char *) key, 33, (char *) buffer, buffer_len, &err);

  free(buffer);

  if (err != NULL) {
    fprintf(stderr, "Could not insert tx into blockchain: %s\n", err);
    return 1;
  }

  leveldb_free(err);
  leveldb_free(woptions);

  return 0;
}


PUnspentTransaction *get_unspent_tx_from_index(uint8_t *tx_id) {
  char *err = NULL;
  uint8_t key[33];
  get_unspent_tx_key(key, tx_id);

  size_t read_len;
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  uint8_t *serialized_tx = (uint8_t *) leveldb_get(db, roptions, (char *) key, 33, &read_len, &err);

  if (err != NULL || serialized_tx == NULL) {
    fprintf(stderr, "Could not retrieve unspent tx from index\n");

    leveldb_free(err);
    leveldb_free(roptions);
    return NULL;
  }

  PUnspentTransaction *tx = unspent_transaction_from_serialized(serialized_tx, read_len);

  leveldb_free(serialized_tx);
  leveldb_free(err);
  leveldb_free(roptions);

  return tx;
}

uint8_t *get_block_hash_from_tx_id(uint8_t *tx_id) {
  char *err = NULL;
  uint8_t key[33];
  get_tx_key(key, tx_id);

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

int delete_block_from_blockchain(uint8_t *block_hash) {
  char *err = NULL;
  uint8_t key[33];
  get_block_key(key, block_hash);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_delete(db, woptions, (char *) key, 33, &err);

  if (err != NULL) {
    fprintf(stderr, "Could not delete block from blockchain\n");
    free(woptions);
    free(err);

    return 0;
  }

  free(woptions);
  return 1;
}

int delete_tx_from_index(uint8_t *tx_id) {
  char *err = NULL;
  uint8_t key[33];
  get_tx_key(key, tx_id);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_delete(db, woptions, (char *) key, 33, &err);

  if (err != NULL) {
    fprintf(stderr, "Could not delete tx from index\n");
    free(woptions);
    free(err);

    return 0;
  }

  free(woptions);
  return 1;
}

int delete_unspent_tx_from_index(uint8_t *tx_id) {
  char *err = NULL;
  uint8_t key[33];
  get_unspent_tx_key(key, tx_id);

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_delete(db, woptions, (char *) key, 33, &err);

  if (err != NULL) {
    fprintf(stderr, "Could not delete tx from unspent index\n");
    free(woptions);
    free(err);

    return 0;
  }

  free(woptions);
  return 1;
}

uint8_t *get_current_block_hash() {
  return current_block_hash;
}

int set_current_block_hash(uint8_t *hash) {
  memcpy(current_block_hash, hash, 32);
  return 0;
}

int get_tx_key(uint8_t *buffer, uint8_t *tx_id) {
  buffer[0] = 't';
  memcpy(buffer + 1, tx_id, 32);

  return 0;
}

int get_unspent_tx_key(uint8_t *buffer, uint8_t *tx_id) {
  buffer[0] = 'c';
  memcpy(buffer + 1, tx_id, 32);

  return 0;
}

int get_block_key(uint8_t *buffer, uint8_t *block_hash) {
  buffer[0] = 'b';
  memcpy(buffer + 1, block_hash, 32);

  return 0;
}
