#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include <stdlib.h>
#include <leveldb/c.h>

#include "wallet.h"

/*
 * open_wallet()
 * Opens a LevelDB instance for the wallet
 */
leveldb_t *open_wallet(char *err) {
  leveldb_t *db;
  leveldb_options_t *options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);

  return leveldb_open(options, "wallet", &err);
}

int new_wallet() {
  // Open DB

  char *err = NULL;
  leveldb_t *db = open_wallet(err);

  if (err != NULL) {
    fprintf(stderr, "Could not open wallet\n");
    return 1;
  }

  leveldb_free(err);
  err = NULL;

  // ----

  size_t read_len;
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  char *initialized = leveldb_get(db, roptions, "0", 1, &read_len, &err);

  if (initialized != NULL) {
    leveldb_free(initialized);
    fprintf(stderr, "Already initialized.\n");
    return 1;
  }

  leveldb_free(err);

  // ----

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];
  unsigned char seed[crypto_sign_SEEDBYTES];
  unsigned char address[ADDRESS_SIZE];

  crypto_sign_keypair(pk, sk);
  crypto_sign_ed25519_sk_to_seed(seed, sk);
  public_key_to_address(address, pk);

  // ---

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();

  PWallet *wallet = malloc(sizeof(PWallet));
  pwallet__init(wallet);

  wallet->secret_key.len = crypto_sign_SECRETKEYBYTES;
  wallet->secret_key.data = malloc(sizeof(uint8_t) * crypto_sign_SECRETKEYBYTES);
  memcpy(wallet->secret_key.data, sk, crypto_sign_SECRETKEYBYTES);

  wallet->public_key.len = crypto_sign_PUBLICKEYBYTES;
  wallet->public_key.data = malloc(sizeof(uint8_t) * crypto_sign_PUBLICKEYBYTES);
  memcpy(wallet->public_key.data, pk, crypto_sign_PUBLICKEYBYTES);

  wallet->address.len = ADDRESS_SIZE;
  wallet->address.data = malloc(sizeof(uint8_t) * ADDRESS_SIZE);
  public_key_to_address(wallet->address.data, pk);

  wallet->balance = 0;

  uint32_t buffer_len = pwallet__get_packed_size(wallet);
  uint8_t *buffer = malloc(buffer_len);

  pwallet__pack(wallet, buffer);

  leveldb_put(db, woptions, "0", 1, (char *) buffer, buffer_len, &err);

  pwallet__free_unpacked(wallet, NULL);
  free(buffer);

  if (err != NULL) {
    fprintf(stderr, "Could not write to wallet database\n");
    return 1;
  }

  leveldb_free(err);
  err = NULL;

  // Close DB
  leveldb_close(db);

  return 0;
}

PWallet *get_wallet() {
  char *err = NULL;
  leveldb_t *db = open_wallet(err);

  if (err != NULL) {
    fprintf(stderr, "Could not open wallet database\n");
    leveldb_free(err);
  }

  size_t buffer_len;
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();
  uint8_t *buffer = (uint8_t *) leveldb_get(db, roptions, "0", 1, &buffer_len, &err);

  PWallet *proto_wallet = pwallet__unpack(NULL, buffer_len, buffer);

  leveldb_free(roptions);
  leveldb_close(db);

  return proto_wallet;
}

int public_key_to_address(uint8_t *address, uint8_t *pk) {
  uint8_t address_id = PROD_NET_ADDRESS_ID;
  memcpy(address, &address_id, sizeof(uint8_t) * 1);
  crypto_hash_sha256(address + 1, pk, crypto_sign_PUBLICKEYBYTES);

  return 0;
}

uint8_t get_address_id(uint8_t *address) {
  uint8_t address_id = address[0];

  return address_id;
}
