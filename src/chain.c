#include <string.h>
#include <stdio.h>
#include <leveldb/c.h>

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

  char *err = NULL;
  uint8_t bytes[4];
  uint32_t n = 2;

  bytes[0] = (n >> 24) & 0xFF;
  bytes[1] = (n >> 16) & 0xFF;
  bytes[2] = (n >> 8) & 0xFF;
  bytes[3] = n & 0xFF;

  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, "height", 6, (char *) bytes, 4, &err);

  size_t read_len;
  uint32_t read_block_height = 50;
  unsigned char *bh = (unsigned char *) leveldb_get(db, roptions, "height", 6, &read_len, &err);;

  read_block_height = (bh[3] & 0xFF << 24) | (bh[2] & 0xFF << 16) | (bh[1] & 0xFF << 8) | (bh[0] & 0xFF);
  printf("Returned blockheight: %d\n", read_block_height);

  return 0;
}
