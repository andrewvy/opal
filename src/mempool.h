#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdint.h>

#include "transaction.h"

struct MemPool {
  uint32_t size;
  struct Transaction **transactions;
};

int start_mempool();
int push_tx_to_mempool(struct Transaction *tx);
struct Transaction *pop_tx_from_mempool();
int get_number_of_tx_from_mempool();
int stop_mempool();

#endif
