#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "deps/greatest.h"

#include "../src/block.h"

SUITE_EXTERN(transaction_suite);

TEST standalone_pass(void) {
  struct Block *block = make_block();

  ASSERT_EQ(block->version, BLOCK_VERSION);

  free_block(block);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  if (sodium_init() == -1) {
    return 1;
  }

  GREATEST_MAIN_BEGIN();

  RUN_TEST(standalone_pass);
  RUN_SUITE(transaction_suite);

  GREATEST_MAIN_END();
}
