#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "deps/greatest.h"

#include "../src/chain.h"

SUITE_EXTERN(transaction_suite);
SUITE_EXTERN(block_suite);
SUITE_EXTERN(mempool_suite);
SUITE_EXTERN(merkle_suite);
SUITE_EXTERN(chain_suite);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  if (sodium_init() == -1) {
    return 1;
  }

  init_blockchain();

  GREATEST_MAIN_BEGIN();

  RUN_SUITE(transaction_suite);
  RUN_SUITE(block_suite);
  RUN_SUITE(mempool_suite);
  RUN_SUITE(merkle_suite);
  RUN_SUITE(chain_suite);

  GREATEST_MAIN_END();

  close_blockchain();
}
