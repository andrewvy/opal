#include <string.h>
#include <sodium.h>

#include "merkle.h"

struct MerkleTree *construct_merkle_tree_from_leaves(uint8_t *hashes, uint32_t num_of_hashes) {
  if (num_of_hashes <= 1) {
    return NULL;
  }

  struct MerkleTree *tree = malloc(sizeof(struct MerkleTree));
  uint32_t num_of_nodes = 0;

  struct MerkleNode **nodes = malloc(
    sizeof(struct MerkleNode *) * num_of_hashes
  );

  construct_merkle_leaves_from_hashes(nodes, &num_of_nodes, hashes, num_of_hashes);

  while (num_of_nodes > 1) {
    collapse_merkle_nodes(nodes, &num_of_nodes);
  }

  tree->root = nodes[0];

  free(nodes);

  return tree;
}

int construct_merkle_leaves_from_hashes(struct MerkleNode **nodes, uint32_t *num_of_nodes, uint8_t *hashes, uint32_t num_of_hashes) {
  for (int i = 0; i < num_of_hashes; i++) {
    struct MerkleNode *node = malloc(sizeof(struct MerkleNode));
    memcpy(node->hash, &hashes[i * 32], 32);
    nodes[i] = node;
  }

  *num_of_nodes = num_of_hashes;

  return 0;
}

int collapse_merkle_nodes(struct MerkleNode **nodes, uint32_t *num_of_nodes) {
  int current_node_idx = 0;
  struct MerkleNode **temp_nodes = malloc(sizeof(struct MerkleNode *) * (*num_of_nodes));

  for (int i = 0; i < (*num_of_nodes - 1); i += 2) {
    temp_nodes[current_node_idx] = construct_merkle_node(nodes[i], nodes[i + 1]);
    current_node_idx++;
  }

  if (*num_of_nodes % 2 != 0) {
    temp_nodes[current_node_idx] = construct_merkle_node(nodes[*num_of_nodes - 1], nodes[*num_of_nodes - 1]);
    current_node_idx++;
  }

  for (int i = 0; i < current_node_idx; i++) {
    nodes[i] = temp_nodes[i];
  }

  *num_of_nodes = current_node_idx;

  free(temp_nodes);

  return 0;
}

struct MerkleNode *construct_merkle_node(struct MerkleNode *left, struct MerkleNode *right) {
  uint8_t *combined_hash = malloc(sizeof(uint8_t) * 64);
  uint8_t *node_hash = malloc(sizeof(uint8_t) * 32);

  memcpy(combined_hash, left->hash, 32);
  memcpy(combined_hash + 32, right->hash, 32);

  crypto_hash_sha256(node_hash, combined_hash, 64);

  struct MerkleNode *node = malloc(sizeof(struct MerkleNode));
  memcpy(node->hash, node_hash, 32);

  free(combined_hash);
  free(node_hash);

  return node;
}

int free_merkle_tree(struct MerkleTree *tree) {
  free_merkle_node(tree->root);
  free(tree);

  return 0;
}

int free_merkle_node(struct MerkleNode *node) {
  if (node == NULL) {
    return 1;
  }

  free_merkle_node(node->left);
  free_merkle_node(node->right);
  free(node);

  return 0;
}
