#include <sodium.h>

#include "deps/greatest.h"
#include "../src/transaction.h"

SUITE(transaction_suite);

TEST can_sign_txin(void) {
  struct InputTransaction txin = {
    .transaction = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00
    },
    .txout_index = 0
  };

  struct OutputTransaction txout = {
    .amount = 50,
    .address = {
      0x01, 0x3e, 0x46, 0xa5,
      0xc6, 0x99, 0x4e, 0x35,
      0x55, 0x50, 0x1c, 0xba,
      0xc0, 0x7c, 0x06, 0x77
    }
  };

  struct OutputTransaction *txout_p = &txout;
  struct Transaction tx = {
    .txin_count = 1,
    .txout_count = 1,
    .txouts = &txout_p
  };

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(&txin, &tx, pk, sk);

  ASSERT_MEM_EQ(pk, txin.public_key, crypto_sign_PUBLICKEYBYTES);

  // --- Now to verify the TXIN signature

  uint32_t header_size = get_tx_sign_header_size(&tx) + TXIN_HEADER_SIZE;
  uint8_t header[header_size];
  uint8_t hash[32];

  get_txin_header(header, &txin);
  get_tx_sign_header(header + TXIN_HEADER_SIZE, &tx);

  ASSERT(crypto_sign_verify_detached(txin.signature, header, header_size, pk) == 0);

  PASS();
}

TEST can_serialize_tx(void) {
  struct InputTransaction txin = {
    .transaction = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00
    },
    .txout_index = 0
  };

  struct OutputTransaction txout = {
    .amount = 50,
    .address = {
      0x01, 0x3e, 0x46, 0xa5,
      0xc6, 0x99, 0x4e, 0x35,
      0x55, 0x50, 0x1c, 0xba,
      0xc0, 0x7c, 0x06, 0x77
    }
  };

  struct InputTransaction *txin_p = &txin;
  struct OutputTransaction *txout_p = &txout;

  struct Transaction tx = {
    .txout_count = 1,
    .txouts = &txout_p
  };

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  crypto_sign_keypair(pk, sk);
  sign_txin(&txin, &tx, pk, sk);

  tx.txin_count = 1;
  tx.txins = &txin_p;

  transaction_to_serialized(&tx);

  PASS();
}

GREATEST_SUITE(transaction_suite) {
  RUN_TEST(can_sign_txin);
  RUN_TEST(can_serialize_tx);
}
