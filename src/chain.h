#ifndef CHAIN_H
#define CHAIN_H

#include <block.h>
#include <stdint.h>

int open_blockchain();
int close_blockchain();

uint32_t get_block_height();
int insert_block_into_blockchain(struct Block *block);

int init_blockchain();

#endif
