#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "block.pb-c.h"
#include "transaction.h"

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
  memcpy(header, &txout->address, 32);
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

int valid_transaction(struct Transaction *tx) {
  if (tx) {
    return 1;
  } else {
    return 0;
  }
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

    msg->txouts[i]->address.len = 32;
    msg->txouts[i]->address.data = malloc(sizeof(uint8_t) * 32);
    memcpy(msg->txouts[i]->address.data, tx->txouts[i]->address, 32);
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
  memcpy(txout->address, proto_txout->address.data, 32);

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
