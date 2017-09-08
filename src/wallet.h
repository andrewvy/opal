#ifndef WALLET_H
#define WALLET_H

#include <stdint.h>
#include <rocksdb/c.h>

#include "opal.pb-c.h"

#define ADDRESS_SIZE (crypto_hash_sha256_BYTES + 1)
#define PROD_NET_ADDRESS_ID 0x01
#define TEST_NET_ADDRESS_ID 0x03

rocksdb_t *open_wallet();
int new_wallet();
int public_key_to_address(unsigned char *address, unsigned char *pk);
uint8_t get_address_id(uint8_t *address);
int valid_address(uint8_t *address);

PWallet *get_wallet();

#endif
