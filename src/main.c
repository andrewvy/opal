#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <sodium.h>

#include "main.h"
#include "block.h"

enum command {
  CMD_NONE,
  CMD_HELP,
  CMD_VERSION,
  CMD_HASH,
  CMD_BLOCK
};

static struct opal_command commands[] = {
  {"none", CMD_NONE},
  {"help", CMD_HELP},
  {"version", CMD_VERSION},
  {"hash", CMD_HASH},
  {"block", CMD_BLOCK}
};

#define MAX_COMMANDS (sizeof(commands) / sizeof(struct opal_command))

void make_hash(char *digest, unsigned char *string) {
  unsigned char hash[crypto_hash_sha256_BYTES];

  crypto_hash_sha256(hash, string, strlen((char *) string));

  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    sprintf(&digest[i*2], "%02x", (unsigned int) hash[i]);
  }
}

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
      case CMD_HASH: {
        char digest[(crypto_hash_sha256_BYTES * 2) + 1];
        unsigned char message[] = "Hello";

        make_hash(digest, message);
        printf("SHA256 Digest: %s\n", digest);
        break;
      }
      case CMD_BLOCK: {
        char digest[(crypto_hash_sha256_BYTES * 2) + 1];

        struct Block *block = make_block();
        hash_block(block);

        for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
          sprintf(&digest[i*2], "%02x", (unsigned int) block->hash[i]);
        }

        printf("Block Hash: %s\n", digest);

        free_block(block);
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

  return 0;
}

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
