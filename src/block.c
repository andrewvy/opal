#include "block.h"

#include <stdlib.h>

struct Block *make_block() {
  struct Block *block = malloc(sizeof(struct Block));
  struct Transaction **transactions = malloc(sizeof(struct Transaction) * 1);

  block->index = 0;
  block->nonce = 0;

  return block;
}

int free_block(struct Block *block) {
  free(block->transactions);
  free(block);

  return 0;
}
