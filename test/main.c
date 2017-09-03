#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "deps/greatest.h"

#include "../src/block.h"

TEST standalone_pass(void) {
  struct Block *block = make_block();

  ASSERT_EQ(block->version, BLOCK_VERSION);

  free_block(block);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(standalone_pass);

  GREATEST_MAIN_END();
}
