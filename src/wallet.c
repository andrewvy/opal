#include <stdio.h>
#include <string.h>
#include <sodium.h>
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

  leveldb_writeoptions_t *woptions = leveldb_writeoptions_create();

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
  leveldb_close(db);

  return 0;
}

int read_wallet() {
  char *error = NULL;

  leveldb_t *wallet = open_wallet(error);
  leveldb_readoptions_t *roptions = leveldb_readoptions_create();

  if (error != NULL) {
    fprintf(stderr, "Could not open wallet, please check if it exists\n");
    return 1;
  }

  size_t read_len;
  char *initialized = leveldb_get(wallet, roptions, "initialized", 11, &read_len, &error);

  if (initialized == NULL) {
    leveldb_free(initialized);
    fprintf(stderr, "Wallet not initialized\n");
    return 1;
  }

  leveldb_free(initialized);

  size_t public_key_len;
  size_t secret_key_len;
  size_t seed_len;
  size_t address_len;

  unsigned char *pk = (unsigned char *) leveldb_get(wallet, roptions, "public_key", 10, &public_key_len, &error);
  unsigned char *sk = (unsigned char *) leveldb_get(wallet, roptions, "secret_key", 10, &secret_key_len, &error);
  unsigned char *seed = (unsigned char *) leveldb_get(wallet, roptions, "seed", 4, &seed_len, &error);
  unsigned char *address = (unsigned char *) leveldb_get(wallet, roptions, "address", 7, &address_len, &error);

  int pk_size = (crypto_sign_PUBLICKEYBYTES * 2) + 1;
  char pk_digest[pk_size];
  for (int i = 0; i < crypto_sign_PUBLICKEYBYTES; i++) {
    sprintf(&pk_digest[i*2], "%02x", (int) pk[i]);
  }

  int sk_size = (crypto_sign_SECRETKEYBYTES * 2) + 1;
  char sk_digest[sk_size];
  for (int i = 0; i < crypto_sign_SECRETKEYBYTES; i++) {
    sprintf(&sk_digest[i*2], "%02x", (int) sk[i]);
  }

  int seed_size = (crypto_sign_SEEDBYTES * 2) + 1;
  char seed_digest[seed_size];
  for (int i = 0; i < crypto_sign_SEEDBYTES; i++) {
    sprintf(&seed_digest[i*2], "%02x", (int) seed[i]);
  }

  int address_size = (ADDRESS_SIZE * 2) + 1;
  char address_digest[address_size];
  for (int i = 0; i < ADDRESS_SIZE; i++) {
    sprintf(&address_digest[i*2], "%02x", (int) address[i]);
  }

  printf("--- Wallet ---\n");
  printf("Public Key: %s\n", pk_digest);
  printf("Secret Key: %s\n", sk_digest);
  printf("Seed: %s\n", seed_digest);
  printf("Address: %s\n", address_digest);

  switch(get_address_id((uint8_t *) address)) {
    case PROD_NET_ADDRESS_ID: {
      printf("Address Type: PROD_NET\n");
      break;
    }
    case TEST_NET_ADDRESS_ID: {
      printf("Address Type: TEST_NET\n");
      break;
    }
    default: {
      printf("Address Type: unrecognized type %d\n", get_address_id((uint8_t *) address));
      break;
    }
  }

  leveldb_free(pk);
  leveldb_free(sk);
  leveldb_free(seed);
  leveldb_free(address);
  leveldb_free(error);

  leveldb_close(wallet);

  return 0;
}

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
