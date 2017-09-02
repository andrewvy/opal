#ifndef MAIN_H
#define MAIN_H

#define OPALCOIN_VERSION "v0.0.1"

enum command { CMD_NONE, CMD_HELP, CMD_VERSION };

struct opal_command {
  char *key;
  int val;
};

void print_help();
void print_version();
int command(char *cmd);

#endif
