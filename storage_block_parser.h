#pragma once
#include "common.h"

class storage_block_parser
{
public:
	storage_block_parser(data_block& data) : data_(data) {}

	operator data_block&();

	void set_type(uint8_t t);
	uint8_t get_type();

	void set_parent_id(uint32_t id);
	uint32_t get_parent_id();

	void set_first_child_id(uint32_t id);
	uint32_t get_first_child_id();

	void set_second_child_id(uint32_t id);
	uint32_t get_second_child_id();

	void set_value(const bi::uint256_t& val);
	void get_value(bi::uint256_t& val);

	void set_32value(uint32_t idx, uint32_t val);
	uint32_t get_32value(uint32_t idx);

	void clear();
	void fill_as_empty_root();
private:
	data_block& data_;
};

