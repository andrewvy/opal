#include "main.h"

#include <stdio.h>
#include <getopt.h>
#include <string.h>

static struct opal_command commands[] = {
  {"none", CMD_NONE},
  {"help", CMD_HELP},
  {"version", CMD_VERSION}
};

#define MAX_COMMANDS (sizeof(commands) / sizeof(struct opal_command))

int main(int argc, char **argv) {
  if (argc > 1) {
    switch(command(argv[1])) {
      case CMD_NONE:
        break;

      case CMD_HELP:
        print_help();
        break;

      case CMD_VERSION:
        print_version();
        break;

      default:
        fprintf(stderr, "No options passed.\n");
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
