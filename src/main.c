#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <sodium.h>

#include "main.h"
#include "block.h"
#include "wallet.h"
#include "net.h"
#include "chain.h"
#include "miner.h"
#include "client.h"

enum command {
  CMD_NONE,
  CMD_HELP,
  CMD_VERSION,
  CMD_GENESIS,
  CMD_GET_WALLET,
  CMD_NEW_WALLET,
  CMD_SERVER,
  CMD_BLOCKHEIGHT,
  CMD_MINE
};

static struct opal_command commands[] = {
  {"none", CMD_NONE},
  {"help", CMD_HELP},
  {"version", CMD_VERSION},
  {"genesis", CMD_GENESIS},
  {"wallet", CMD_GET_WALLET},
  {"new_wallet", CMD_NEW_WALLET},
  {"server", CMD_SERVER},
  {"blockheight", CMD_BLOCKHEIGHT},
  {"mine", CMD_MINE},
};

#define MAX_COMMANDS (sizeof(commands) / sizeof(struct opal_command))

void make_hash(char *digest, unsigned char *string) {
  unsigned char hash[crypto_hash_sha256_BYTES];

  crypto_hash_sha256(hash, string, strlen((char *) string));

  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    sprintf(&digest[i*2], "%02x", (unsigned int) hash[i]);
  }
}

#ifndef OPAL_TEST
int main(int argc, char **argv) {
  if (sodium_init() == -1) {
    return 1;
  }

  if (argc > 1) {
    switch(command(argv[1])) {
      case CMD_NONE: {
        break;
      }
      case CMD_HELP: {
        print_help();
        break;
      }
      case CMD_VERSION: {
        print_version();
        break;
      }
      case CMD_GENESIS: {
        init_blockchain();
        struct Block *block = make_block();
        block->timestamp = genesis_block.timestamp;
        hash_block(block);

        int i = 0;
        while (!valid_block_hash(block)) {
          block->nonce = i;
          hash_block(block);
          i++;
        }

        if (compare_with_genesis_block(block) == 0) {
          printf("Verified genesis block!\n");
          print_block(block);
        } else {
          fprintf(stderr, "Genesis block mismatch! Generated a hash that is different than recorded.\n");
          print_block(block);
        }

        free_block(block);
        break;
      }
      case CMD_GET_WALLET: {
        rpc_get_wallet();
        break;
      }
      case CMD_NEW_WALLET: {
        new_wallet();
        break;
      }
      case CMD_SERVER: {
        init_blockchain();
        start_server();
        break;
      }
      case CMD_BLOCKHEIGHT: {
        init_blockchain();
        uint32_t height = get_block_height();
        printf("Current local blockheight: %d\n", height);
        break;
      }
      case CMD_MINE: {
        init_blockchain();
        start_mining();
        break;
      }
      default: {
        fprintf(stderr, "No options passed.\n");
      }
    }
  } else {
    print_version();
    print_help();
  }

  close_blockchain();

  return 0;
}
#endif

int command(char *cmd_string) {
  for (int i = 0; i < MAX_COMMANDS; i++) {
    struct opal_command cmd = commands[i];

    if (strcmp(cmd.key, cmd_string) == 0) {
      return cmd.val;
    }
  }

  return CMD_NONE;
}

void print_help() {
  printf(""
    "usage: opal <command> [<args>]\n"
  );
}

void print_version() {
  printf("opal - " OPALCOIN_VERSION "\n");
}
