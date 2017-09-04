#include <stdint.h>

#include "transaction.h"
#include "mempool.h"

int MEMPOOL_INITIALIZED = 0;
static struct MemPool *mempool;

int start_mempool() {
  if (MEMPOOL_INITIALIZED) {
    return 1;
  } else {
    mempool = malloc(sizeof(struct MemPool));
    mempool->size = 0;
    mempool->transactions = malloc(sizeof(struct Transaction *));
    MEMPOOL_INITIALIZED = 1;
    return 0;
  }
}

int push_tx_to_mempool(struct Transaction *tx) {
  if (MEMPOOL_INITIALIZED != 1) {
    return 1;
  }

  mempool->size++;
  realloc(mempool->transactions, sizeof(struct Transaction *) * mempool->size);
  mempool->transactions[mempool->size - 1] = tx;

  return 0;
}

struct Transaction *pop_tx_from_mempool() {
  if (MEMPOOL_INITIALIZED != 1) {
    return NULL;
  }

  struct Transaction *tx = mempool->transactions[mempool->size - 1];
  mempool->size--;
  realloc(mempool->transactions, sizeof(struct Transaction *) * mempool->size);

  return tx;
}

int get_number_of_tx_from_mempool() {
  return mempool->size;
}

int stop_mempool() {
  if (MEMPOOL_INITIALIZED) {
    free(mempool);
    MEMPOOL_INITIALIZED = 0;
    return 0;
  } else {
    return 1;
  }
}
