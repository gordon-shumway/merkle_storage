#pragma once

#include "uint256_t\uint256_t.h"
#include <array>

#define KEY_LENGTH 256

#define FREE_BLOCK_TYPE 0
#define STORAGE_FREE_INFO_BLOCK_TYPE 1
#define MERKLE_NODE_BLOCK_TYPE 2
#define VALUE_BLOCK_TYPE 3

// header: type  + parent block idx + 2 child block idxs
#define BLOCK_HEADER_SIZE (1 + 4 + 4 + 4)
#define BLOCK_VALUE_SIZE 32
#define BLOCK_SIZE (BLOCK_HEADER_SIZE + BLOCK_VALUE_SIZE)

typedef std::array<uint8_t, BLOCK_SIZE> data_block;
