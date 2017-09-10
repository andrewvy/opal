#include <stdint.h>
#include <string.h>
#include <time.h>

#include "block.h"
#include "chain.h"
#include "miner.h"
#include "wallet.h"

static int IS_MINING = 0;

int start_mining() {
  IS_MINING = 1;

  printf("Started mining...\n");

  while (1) {
    uint8_t *previous_hash = get_current_block_hash();
    struct Block *block = compute_next_block(previous_hash);
    insert_block_into_blockchain(block);
    uint32_t block_height = get_block_height();

    printf("Inserted block #%d\n", block_height);
    print_block(block);
    set_current_block_hash(block->hash);
    free(block);
  }
}

struct Block *compute_next_block(uint8_t *prev_block_hash) {
  uint32_t nonce = 0;
  time_t current_time = time(NULL);

  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  memset(txin->transaction, 0, 32);
  txin->txout_index = get_block_height();
  txout->amount = 50 * OPALITES_PER_OPAL;

  PWallet *wallet = get_wallet();
  memcpy(txout->address, wallet->address.data, ADDRESS_SIZE);
  pwallet__free_unpacked(wallet, NULL);

  struct Transaction *tx = malloc(sizeof(struct Transaction));
  tx->txout_count = 1;
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * 1);
  tx->txouts[0] = txout;

  sign_txin(txin, tx, wallet->public_key.data, wallet->secret_key.data);

  tx->txin_count = 1;
  tx->txins = malloc(sizeof(struct InputTransaction *) * 1);
  tx->txins[0] = txin;
  compute_self_tx_id(tx);

  struct Block *block = make_block();
  block->transaction_count = 1;
  block->transactions = malloc(sizeof(struct Transaction *) * 1);
  block->transactions[0] = tx;
  block->timestamp = current_time;
  memcpy(block->previous_hash, get_current_block_hash(), 32);

  compute_self_merkle_root(block);
  hash_block(block);

  while (!valid_block_hash(block)) {
    block->nonce = nonce;

    hash_block(block);
    nonce++;
  }

  return block;
}
