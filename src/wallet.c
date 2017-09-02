#include <stdio.h>
#include <sodium.h>
#include <leveldb/c.h>

#include "wallet.h"

int new_wallet() {
  // Open DB

  leveldb_t *db;
  leveldb_options_t *options;
  leveldb_readoptions_t *roptions;
  leveldb_writeoptions_t *woptions;

  char *err = NULL;

  options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);
  db = leveldb_open(options, "wallet", &err);

  if (err != NULL) {
    fprintf(stderr, "Could not open wallet database\n");
    return 1;
  }

  leveldb_free(err);
  err = NULL;

  // ----

  size_t read_len;
  roptions = leveldb_readoptions_create();
  char *initialized = leveldb_get(db, roptions, "initialized", 11, &read_len, &err);

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

  woptions = leveldb_writeoptions_create();
  leveldb_put(db, woptions, "initialized", 11, "true", 4, &err);
  leveldb_put(db, woptions, "public_key", 10, (char *) pk, crypto_sign_PUBLICKEYBYTES, &err);
  leveldb_put(db, woptions, "secret_key", 10, (char *) sk, crypto_sign_SECRETKEYBYTES, &err);
  leveldb_put(db, woptions, "seed", 4, (char *) seed, crypto_sign_SEEDBYTES, &err);
  leveldb_put(db, woptions, "address", 7, (char *) address, ADDRESS_SIZE, &err);

  uint32_t balance = 0;
  leveldb_put(db, woptions, "balance", 7, (char *) &balance, sizeof(uint32_t), &err);

  if (err != NULL) {
    fprintf(stderr, "Could not write to wallet database\n");
    return 1;
  }

  leveldb_free(err);
  err = NULL;

  // Close DB

  leveldb_free(err);
  leveldb_close(db);

  return 0;
}

/*
  char pk_digest[(crypto_sign_PUBLICKEYBYTES * 2) + 1];
  for (int i = 0; i < crypto_sign_PUBLICKEYBYTES; i++) {
    sprintf(&pk_digest[i*2], "%02x", (unsigned int) pk[i]);
  }

  printf("Public Key: %s\n", pk_digest);

  char sk_digest[(crypto_sign_SECRETKEYBYTES * 2) + 1];
  for (int i = 0; i < crypto_sign_SECRETKEYBYTES; i++) {
    sprintf(&sk_digest[i*2], "%02x", (unsigned int) sk[i]);
  }

  printf("Secret Key: %s\n", sk_digest);

  char seed_digest[(crypto_sign_SEEDBYTES * 2) + 1];
  for (int i = 0; i < crypto_sign_SEEDBYTES; i++) {
    sprintf(&seed_digest[i*2], "%02x", (unsigned int) seed[i]);
  }

  printf("Seed: %s\n", seed_digest);


  char address_digest[(ADDRESS_SIZE * 2) + 1];
  for (int i = 0; i < ADDRESS_SIZE; i++) {
    sprintf(&address_digest[i*2], "%02x", (unsigned int) address[i]);
  }

  printf("Address: %s\n", seed_digest);
*/

/*
  roptions = leveldb_readoptions_create();
  leveldb_iterator_t *iterator = leveldb_create_iterator(db, roptions);

  for (leveldb_iter_seek(iterator, "key", 3); leveldb_iter_valid(iterator); leveldb_iter_next(iterator)) {
    size_t key_length;
    size_t value_length;
    const char *key = leveldb_iter_key(iterator, &key_length);
    const char *value = leveldb_iter_value(iterator, &value_length);

    printf("Key: %s - Value: %s\n", key, value);
  }

  leveldb_iter_destroy(iterator);
*/


int public_key_to_address(unsigned char *address, unsigned char *pk) {
  crypto_hash_sha256(address, pk, crypto_sign_PUBLICKEYBYTES);
  return 0;
}
