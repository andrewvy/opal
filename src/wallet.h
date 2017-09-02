#ifndef WALLET_H
#define WALLET_H

#include <stdint.h>

struct Wallet {
  uint32_t private_key_size;
  uint32_t public_key_size;
  uint8_t *private_key;
  uint8_t *public_key;
};

int new_wallet();

#endif
