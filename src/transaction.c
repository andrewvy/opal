#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "transaction.h"

int sign_txin(struct InputTransaction *txin, uint8_t *public_key, uint8_t *secret_key) {
  uint8_t header[INPUT_TXIN_HEADER_SIZE];

  get_txin_header(header, txin);
  crypto_sign_detached(txin->signature, NULL, header, INPUT_TXIN_HEADER_SIZE, secret_key);
  memcpy(&txin->public_key, public_key, crypto_sign_PUBLICKEYBYTES);

  return 0;
}

int get_txin_header(uint8_t *header, struct InputTransaction *txin) {
  memcpy(header, &txin->transaction, 32);
  memcpy(header + 32, &txin->txout_index, 4);
  return 0;
}
