#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdint.h>
#include <sodium.h>

/*
 * Transactions can contain multiple InputTXs and multiple OutputTXs.
 *
 * InputTXs are treated as sources of value, pooled together and then split apart into one or more OutputTXs
 * InputTXs reference a previous transaction hash where the value is coming from an earlier OutputTX (solidified in the blockchain)
 * InputTXs contain a signature of the InputTX header, as well as the public key that signed it.
 *   - The public key is converted into an Address to verify that the prev OutputTX was sent to the pubkey's address.
 *   - This "unlocks" the previous OutputTX to be used as this current InputTX.
 *   - The public key is used to verify the signature. (meaning the header has not been tampered.)
 *   - From this, we know that:
 *     - The person that is making this transaction owns the value designated by the transaction the InputTX(s) refer to.
 *     - The person confirms that this transaction should be taking place.
 *
 * Once an OutputTX is used as an InputTX, it is considered spent. (All value is used from a OutputTX when being used as input)
 * - If you don't want to spend everything from an InputTX, you can create a new OutputTX to send back to yourself as leftover-change.
 */

#define TXIN_HEADER_SIZE (32 + 4)
#define TXOUT_HEADER_SIZE (32 + 4)

struct InputTransaction {
  // --- Header
  uint8_t transaction[32]; // Previous tx hash/id
  uint32_t txout_index; // Referenced txout index in previous tx
  // ---

  uint8_t signature[crypto_sign_BYTES];
  uint8_t public_key[crypto_sign_PUBLICKEYBYTES];
};

struct OutputTransaction {
  uint32_t amount;
  uint8_t address[32];
};

struct Transaction {
  uint8_t id[32];
  uint8_t txin_count;
  uint8_t txout_count;
  struct InputTransaction **txins;
  struct OutputTransaction **txouts;
};

int sign_txin(struct InputTransaction *txin, struct Transaction *tx, uint8_t *public_key, uint8_t *secret_key);
int get_txin_header(uint8_t *header, struct InputTransaction *txin);
int get_txout_header(uint8_t *header, struct OutputTransaction *txout);
uint32_t get_tx_sign_header_size(struct Transaction *tx);
uint32_t get_tx_header_size(struct Transaction *tx);
int get_tx_sign_header(uint8_t *header, struct Transaction *tx);

int transaction_to_serialized(uint8_t *buffer, uint32_t *buffer_len, struct Transaction *tx);

#endif
