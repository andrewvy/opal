#include <stdio.h>

#include "wallet.h"

int new_wallet() {
  FILE *wallet_file = fopen("wallet.dat", "rw");

  if (!wallet_file) {
    return -1;
  }

  fclose(wallet_file);

  return 0;
}
