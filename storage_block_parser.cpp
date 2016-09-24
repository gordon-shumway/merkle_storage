#include "storage_block_parser.h"
#include "utils.h"

storage_block_parser::operator data_block&()
{
	return data_;
}

void storage_block_parser::set_type(uint8_t t)
{
	data_[0] = t;
}

uint8_t storage_block_parser::get_type()
{
	return data_[0];
}

void storage_block_parser::set_parent_id(uint32_t id)
{
	split32to4x8(data_, 1, id);
}

uint32_t storage_block_parser::get_parent_id()
{
	return merge4x8to32(data_, 1);
}

void storage_block_parser::set_first_child_id(uint32_t id)
{
	split32to4x8(data_, 5, id);
}

uint32_t storage_block_parser::get_first_child_id()
{
	return merge4x8to32(data_, 5);
}

void storage_block_parser::set_second_child_id(uint32_t id)
{
	split32to4x8(data_, 9, id);
}

uint32_t storage_block_parser::get_second_child_id()
{
	return merge4x8to32(data_, 9);
}

void storage_block_parser::set_value(const bi::uint256_t & val)
{
	memcpy(&data_[BLOCK_HEADER_SIZE], &val, sizeof(val));
}

void storage_block_parser::get_value(bi::uint256_t & val)
{
	memcpy(&val, &data_[BLOCK_HEADER_SIZE], sizeof(val));
}

void storage_block_parser::set_32value(uint32_t idx, uint32_t val)
{
	if (idx >= BLOCK_VALUE_SIZE / sizeof(uint32_t))
		throw std::runtime_error("Invalid storage block index");
	split32to4x8(data_,
		BLOCK_HEADER_SIZE + idx * sizeof(uint32_t),
		val);
}

uint32_t storage_block_parser::get_32value(uint32_t idx)
{
	if (idx >= BLOCK_VALUE_SIZE / sizeof(uint32_t))
		throw std::runtime_error("Invalid storage block index");
	return merge4x8to32(data_, 
		BLOCK_HEADER_SIZE + idx * sizeof(uint32_t));
}

void storage_block_parser::clear()
{
	data_.fill(0);
}

void storage_block_parser::fill_as_empty_root()
{
	clear();
	set_type(MERKLE_NODE_BLOCK_TYPE);
}
