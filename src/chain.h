#ifndef CHAIN_H
#define CHAIN_H

#include <stdint.h>

#include "block.h"
#include "transaction.h"

int open_blockchain();
int close_blockchain();
int init_blockchain();

uint32_t get_block_height();
int insert_block_into_blockchain(struct Block *block);
struct Block *get_block_from_blockchain(uint8_t *block_hash);

int insert_tx_into_index(uint8_t *block_key, struct Transaction *tx);
int insert_unspent_tx_into_index(struct Transaction *tx);
int insert_proto_unspent_tx_into_index(PUnspentTransaction *tx);
PUnspentTransaction *get_unspent_tx_from_index(uint8_t *tx_id);

uint8_t *get_block_hash_from_tx_id(uint8_t *tx_id);
struct Block *get_block_from_tx_id(uint8_t *tx_id);

int delete_block_from_blockchain(uint8_t *block_hash);
int delete_tx_from_index(uint8_t *tx_id);
int delete_unspent_tx_from_index(uint8_t *tx_id);

uint8_t *get_current_block_hash();
int set_current_block_hash(uint8_t *hash);

int get_tx_key(uint8_t *buffer, uint8_t *tx_id);
int get_unspent_tx_key(uint8_t *buffer, uint8_t *tx_id);
int get_block_key(uint8_t *buffer, uint8_t *block_hash);

uint32_t get_balance_for_address(uint8_t *address);

#endif
