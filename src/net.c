#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include <protobuf-c-rpc/protobuf-c-rpc.h>

#include "opal.pb-c.h"
#include "wallet.h"

static void internal_rpc__get_wallet(PInternal_Service *server,
                                     const PEmpty *input,
                                     PWallet_Closure closure,
                                     void *closure_data)
{
  PWallet *wallet = get_wallet();
  free(wallet->secret_key.data);
  wallet->secret_key.len = 0;
  wallet->secret_key.data = NULL;
  closure(wallet, closure_data);
}

static void internal_rpc__send_transaction(PInternal_Service *service,
                                 const PSendTransactionRequest *input,
                                 PSendTransactionResponse_Closure closure,
                                 void *closure_data)
{
}

static PInternal_Service internal_service = PINTERNAL__INIT(internal_rpc__);

int start_server() {
  ProtobufC_RPC_Server *server;
  ProtobufC_RPC_AddressType address_type = 0;

  server = protobuf_c_rpc_server_new(address_type, "9898", (ProtobufCService *) &internal_service, NULL);
  printf("Internal RPC Server started on port: 9898\n");

  while (1) {
    protobuf_c_rpc_dispatch_run(protobuf_c_rpc_dispatch_default());
  }

  return 0;
}
