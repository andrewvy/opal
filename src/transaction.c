#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "chain.h"
#include "opal.pb-c.h"
#include "transaction.h"
#include "wallet.h"

uint8_t zero_hash[32] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

/*
 * This function takes in:
 * - a single TXIN to sign for.
 * - a partially filled out TX that contains all the TXOUTs to sign for.
 * - public key for signature + address verification
 * - secret key to sign
 */
int sign_txin(struct InputTransaction *txin, struct Transaction *tx, uint8_t *public_key, uint8_t *secret_key) {
  uint32_t header_size = get_tx_sign_header_size(tx) + TXIN_HEADER_SIZE;
  uint8_t header[header_size];
  uint8_t hash[32];

  get_txin_header(header, txin);
  get_tx_sign_header(header + TXIN_HEADER_SIZE, tx);
  crypto_hash_sha256(hash, header, header_size);
  crypto_sign_detached(txin->signature, NULL, header, header_size, secret_key);
  memcpy(&txin->public_key, public_key, crypto_sign_PUBLICKEYBYTES);

  return 0;
}

int get_txin_header(uint8_t *header, struct InputTransaction *txin) {
  memcpy(header, &txin->transaction, 32);
  memcpy(header + 32, &txin->txout_index, 4);
  return 0;
}

int get_txout_header(uint8_t *header, struct OutputTransaction *txout) {
  memcpy(header, &txout->amount, 4);
  memcpy(header, &txout->address, ADDRESS_SIZE);
  return 0;
}

uint32_t get_tx_header_size(struct Transaction *tx) {
  uint32_t txin_header_sizes = TXIN_HEADER_SIZE * tx->txin_count;
  uint32_t txout_header_sizes = TXOUT_HEADER_SIZE * tx->txout_count;

  return txin_header_sizes + txout_header_sizes;
}

/*
 * The reason why sign header is different from full header is that
 * the signing header only contains TXOUTs. This is used in the context
 * for a TXIN, as it needs to sign consent to spend value to these
 * TXOUTs.
 */
uint32_t get_tx_sign_header_size(struct Transaction *tx) {
  uint32_t txout_header_sizes = TXOUT_HEADER_SIZE * tx->txout_count;
  return txout_header_sizes;
}

int get_tx_sign_header(uint8_t *header, struct Transaction *tx) {
  for (int i = 0; i < tx->txout_count; i++) {
    get_txout_header(header + (TXOUT_HEADER_SIZE * i), tx->txouts[i]);
  }

  return 0;
}

/*
 * A transaction is valid if:
 * - It is a generation tx
 * - It has TXINs that reference valid unspent TXOUTs
 * - Its combined TXIN UTXO values equal the combined amount of TXOUTs.
 */
int valid_transaction(struct Transaction *tx) {
  if (tx->txin_count > 0 && tx->txout_count > 0) {
    return (
      is_generation_tx(tx) || (
        tx->txin_count > 0 &&
        tx->txout_count > 0 &&
        do_txins_reference_unspent_txouts(tx)
      )
    );
  } else {
    return 0;
  }
}

int do_txins_reference_unspent_txouts(struct Transaction *tx) {
  int valid_txins = 0;
  int input_money = 0;
  int required_money = 0;

  for (int i = 0; i < tx->txin_count; i++) {
    struct InputTransaction *txin = tx->txins[i];
    PUnspentTransaction *unspent_tx = get_unspent_tx_from_index(txin->transaction);

    if (unspent_tx != NULL) {
      if (((unspent_tx->n_unspent_txouts - 1) < txin->txout_index) ||
          (unspent_tx->unspent_txouts[txin->txout_index] == NULL)) {
      } else {
        PUnspentOutputTransaction *unspent_txout = unspent_tx->unspent_txouts[txin->txout_index];

        if (unspent_txout->spent == 0) {
          input_money += unspent_txout->amount;
          valid_txins++;
        }
      }

      free_proto_unspent_transaction(unspent_tx);
    }
  }

  for (int i = 0; i < tx->txout_count; i++) {
    struct OutputTransaction *txout = tx->txouts[i];
    required_money += txout->amount;

    if (valid_address(txout->address) == 0) {
      return 0;
    }
  }

  return (valid_txins == tx->txin_count) && (input_money == required_money);
}

int is_generation_tx(struct Transaction *tx) {
  return (tx->txin_count == 1 && tx->txout_count == 1 && memcmp(tx->txins[0]->transaction, zero_hash, 32) == 0);
}

int compute_tx_id(uint8_t *header, struct Transaction *tx) {
  uint8_t *buffer = NULL;
  uint32_t buffer_len = 0;

  transaction_to_serialized(&buffer, &buffer_len, tx);
  crypto_hash_sha256(header, buffer, buffer_len);
  free(buffer);

  return 0;
}

int compute_self_tx_id(struct Transaction *tx) {
  compute_tx_id(tx->id, tx);

  return 0;
}

/*
 * Converts an allocated tx to a newly allocated protobuf
 * tx struct.
 *
 * Later to be free'd with `free_proto_transaction`
 */
PTransaction *transaction_to_proto(struct Transaction *tx) {
  PTransaction *msg = malloc(sizeof(PTransaction));
  ptransaction__init(msg);

  msg->id.len = 32;
  msg->id.data = malloc(sizeof(char) * 32);
  memcpy(msg->id.data, tx->id, 32);

  msg->n_txins = tx->txin_count;
  msg->n_txouts = tx->txout_count;

  msg->txins = malloc(sizeof(PInputTransaction *) * msg->n_txins);
  msg->txouts = malloc(sizeof(POutputTransaction *) * msg->n_txouts);

  for (int i = 0; i < msg->n_txins; i++) {
    msg->txins[i] = malloc(sizeof(PInputTransaction));
    pinput_transaction__init(msg->txins[i]);

    msg->txins[i]->txout_index = tx->txins[i]->txout_index;

    msg->txins[i]->transaction.len = 32;
    msg->txins[i]->transaction.data = malloc(sizeof(uint8_t) * 32);
    memcpy(msg->txins[i]->transaction.data, tx->txins[i]->transaction, 32);

    msg->txins[i]->signature.len = crypto_sign_BYTES;
    msg->txins[i]->signature.data = malloc(crypto_sign_BYTES);
    memcpy(msg->txins[i]->signature.data, tx->txins[i]->signature, crypto_sign_BYTES);

    msg->txins[i]->public_key.len = crypto_sign_PUBLICKEYBYTES;
    msg->txins[i]->public_key.data = malloc(crypto_sign_PUBLICKEYBYTES);
    memcpy(msg->txins[i]->public_key.data, tx->txins[i]->public_key, crypto_sign_PUBLICKEYBYTES);
  }

  for (int i = 0; i < msg->n_txouts; i++) {
    msg->txouts[i] = malloc(sizeof(POutputTransaction));
    poutput_transaction__init(msg->txouts[i]);

    msg->txouts[i]->amount = tx->txouts[i]->amount;

    msg->txouts[i]->address.len = ADDRESS_SIZE;
    msg->txouts[i]->address.data = malloc(sizeof(uint8_t) * ADDRESS_SIZE);
    memcpy(msg->txouts[i]->address.data, tx->txouts[i]->address, ADDRESS_SIZE);
  }

  return msg;
}

PUnspentTransaction *unspent_transaction_to_proto(struct Transaction *tx) {
  PUnspentTransaction *msg = malloc(sizeof(PTransaction));
  punspent_transaction__init(msg);

  msg->id.len = 32;
  msg->id.data = malloc(sizeof(char) * 32);
  memcpy(msg->id.data, tx->id, 32);

  msg->coinbase = is_generation_tx(tx);
  msg->n_unspent_txouts = tx->txout_count;
  msg->unspent_txouts = malloc(sizeof(POutputTransaction *) * msg->n_unspent_txouts);

  for (int i = 0; i < msg->n_unspent_txouts; i++) {
    msg->unspent_txouts[i] = malloc(sizeof(PUnspentOutputTransaction));
    punspent_output_transaction__init(msg->unspent_txouts[i]);

    msg->unspent_txouts[i]->amount = tx->txouts[i]->amount;

    msg->unspent_txouts[i]->address.len = ADDRESS_SIZE;
    msg->unspent_txouts[i]->address.data = malloc(sizeof(uint8_t) * ADDRESS_SIZE);
    memcpy(msg->unspent_txouts[i]->address.data, tx->txouts[i]->address, ADDRESS_SIZE);

    msg->unspent_txouts[i]->spent = 0;
  }

  return msg;
}

int transaction_to_serialized(uint8_t **buffer, uint32_t *buffer_len, struct Transaction *tx) {
  PTransaction *msg = transaction_to_proto(tx);
  unsigned int len = ptransaction__get_packed_size(msg);
  *buffer_len = len;

  *buffer = malloc(len);
  ptransaction__pack(msg, *buffer);
  free_proto_transaction(msg);

  return 0;
}

int unspent_transaction_to_serialized(uint8_t **buffer, uint32_t *buffer_len, struct Transaction *tx) {
  PUnspentTransaction *msg = unspent_transaction_to_proto(tx);
  unsigned int len = punspent_transaction__get_packed_size(msg);
  *buffer_len = len;

  *buffer = malloc(len);
  punspent_transaction__pack(msg, *buffer);
  free_proto_unspent_transaction(msg);

  return 0;
}

int proto_unspent_transaction_to_serialized(uint8_t **buffer, uint32_t *buffer_len, PUnspentTransaction *msg) {
  unsigned int len = punspent_transaction__get_packed_size(msg);
  *buffer_len = len;

  *buffer = malloc(len);
  punspent_transaction__pack(msg, *buffer);

  return 0;
}

struct InputTransaction *txin_from_proto(PInputTransaction *proto_txin) {
  struct InputTransaction *txin = malloc(sizeof(struct InputTransaction));

  memcpy(txin->transaction, proto_txin->transaction.data, 32);
  txin->txout_index = proto_txin->txout_index;
  memcpy(txin->signature, proto_txin->signature.data, crypto_sign_BYTES);
  memcpy(txin->public_key, proto_txin->public_key.data, crypto_sign_PUBLICKEYBYTES);

  return txin;
}

struct OutputTransaction *txout_from_proto(POutputTransaction *proto_txout) {
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  txout->amount = proto_txout->amount;
  memcpy(txout->address, proto_txout->address.data, ADDRESS_SIZE);

  return txout;
}

struct OutputTransaction *unspent_txout_from_proto(PUnspentOutputTransaction *proto_txout) {
  struct OutputTransaction *txout = malloc(sizeof(struct OutputTransaction));

  txout->amount = proto_txout->amount;
  memcpy(txout->address, proto_txout->address.data, ADDRESS_SIZE);

  return txout;
}

struct Transaction *transaction_from_proto(PTransaction *proto_tx) {
  struct Transaction *tx = malloc(sizeof(struct Transaction));

  memcpy(tx->id, proto_tx->id.data, 32);
  tx->txin_count = proto_tx->n_txins;
  tx->txout_count = proto_tx->n_txouts;

  tx->txins = malloc(sizeof(struct InputTransaction *) * tx->txin_count);
  tx->txouts = malloc(sizeof(struct OutputTransaction *) * tx->txout_count);

  for (int i = 0; i < tx->txin_count; i++) {
    tx->txins[i] = txin_from_proto(proto_tx->txins[i]);
  }

  for (int i = 0; i < tx->txout_count; i++) {
    tx->txouts[i] = txout_from_proto(proto_tx->txouts[i]);
  }

  return tx;
}

struct Transaction *transaction_from_serialized(uint8_t *buffer, uint32_t buffer_len) {
  PTransaction *proto_tx = ptransaction__unpack(NULL, buffer_len, buffer);
  struct Transaction *tx = transaction_from_proto(proto_tx);
  ptransaction__free_unpacked(proto_tx, NULL);

  return tx;
}

PUnspentTransaction *unspent_transaction_from_serialized(uint8_t *buffer, uint32_t buffer_len) {
  PUnspentTransaction *proto_unspent_tx = punspent_transaction__unpack(NULL, buffer_len, buffer);
  return proto_unspent_tx;
}

int free_proto_transaction(PTransaction *proto_transaction) {
  for (int i = 0; i < proto_transaction->n_txins; i++) {
    free(proto_transaction->txins[i]->transaction.data);
    free(proto_transaction->txins[i]->signature.data);
    free(proto_transaction->txins[i]->public_key.data);
    free(proto_transaction->txins[i]);
  }

  for (int i = 0; i < proto_transaction->n_txouts; i++) {
    free(proto_transaction->txouts[i]->address.data);
    free(proto_transaction->txouts[i]);
  }

  free(proto_transaction->txins);
  free(proto_transaction->txouts);
  free(proto_transaction);

  return 0;
}

int free_proto_unspent_transaction(PUnspentTransaction *proto_unspent_tx) {
  for (int i = 0; i < proto_unspent_tx->n_unspent_txouts; i++) {
    free(proto_unspent_tx->unspent_txouts[i]->address.data);
    free(proto_unspent_tx->unspent_txouts[i]);
  }

  free(proto_unspent_tx);

  return 0;
}

/*
 * Frees an allocated TX, and its corresponding allocated
 * TXINs and TXOUTs.
 */
int free_transaction(struct Transaction *tx) {
  for (int i = 0; i < tx->txin_count; i++) {
    free(tx->txins[i]);
  }

  for (int i = 0; i < tx->txout_count; i++) {
    free(tx->txouts[i]);
  }

  free(tx);

  return 0;
}
