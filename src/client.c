/*
 * This client connects to an already running daemon server.
 */

#include <sodium.h>
#include <stdio.h>
#include <protobuf-c-rpc/protobuf-c-rpc.h>

#include "wallet.h"
#include "client.h"
#include "opal.pb-c.h"

static void handle_get_wallet(const PWallet *wallet, void *closure_data) {
  int public_address_len = (ADDRESS_SIZE * 2) + 1;
  char public_address[public_address_len];

  for (int i = 0; i < ADDRESS_SIZE; i++) {
    sprintf(&public_address[i*2], "%02x", (int) wallet->address.data[i]);
  }

  printf("public_address: %s\n", public_address);

  *(protobuf_c_boolean *) closure_data = 1;
}

int rpc_get_wallet() {
  ProtobufCService *service;
  ProtobufC_RPC_Client *client;
  ProtobufC_RPC_AddressType address_type = 0;

  service = protobuf_c_rpc_client_new(address_type, "9898", &pinternal__descriptor, NULL);

  if (service == NULL) {
    fprintf(stderr, "Could not create protobuf service\n");
  }

  client = (ProtobufC_RPC_Client *) service;

  printf("Connecting to daemon..\n");

  while(!protobuf_c_rpc_client_is_connected(client)) {
    protobuf_c_rpc_dispatch_run(protobuf_c_rpc_dispatch_default());
  }

  printf("Connected!\n");

  protobuf_c_boolean is_done = 0;
  PEmpty empty = PEMPTY__INIT;
  pinternal__get_wallet(service, &empty, handle_get_wallet, &is_done);

  while (!is_done) {
    protobuf_c_rpc_dispatch_run(protobuf_c_rpc_dispatch_default());
  }

  return 0;
}
