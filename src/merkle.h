#ifndef MERKLE_H
#define MERKLE_H

#include <stdint.h>

struct MerkleNode;
struct MerkleNode {
  struct MerkleNode *left;
  struct MerkleNode *right;
  uint8_t hash[32];
};

struct MerkleTree {
  struct MerkleNode *root;
};

struct MerkleTree *construct_merkle_tree_from_leaves(uint8_t *hashes, uint32_t num_of_hashes);
struct MerkleNode *construct_merkle_node(struct MerkleNode *left, struct MerkleNode *right);

int construct_merkle_leaves_from_hashes(struct MerkleNode **nodes, uint32_t *num_of_nodes, uint8_t *hashes, uint32_t num_of_hashes);
int collapse_merkle_nodes(struct MerkleNode **nodes, uint32_t *num_of_nodes);

int free_merkle_tree(struct MerkleTree *tree);
int free_merkle_node(struct MerkleNode *node);

#endif
