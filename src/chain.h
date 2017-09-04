#ifndef CHAIN_H
#define CHAIN_H

#include <stdint.h>

int open_blockchain();
int close_blockchain();

uint32_t get_block_height();

int init_blockchain();

#endif
