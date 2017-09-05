#ifndef MINER_H
#define MINER_H

int start_mining();
struct Block *compute_next_block(uint8_t *prev_block_hash);

#endif
