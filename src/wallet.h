#ifndef WALLET_H
#define WALLET_H

#include <stdint.h>

#define ADDRESS_SIZE crypto_hash_sha256_BYTES

struct Wallet {
  uint32_t private_key_size;
  uint32_t public_key_size;
  uint8_t *private_key;
  uint8_t *public_key;
};

int new_wallet();
int public_key_to_address(unsigned char *address, unsigned char *pk);

#endif
