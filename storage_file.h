#pragma once
#include <string>
#include <memory>
#include <set>
#include <cstdio>
#include "common.h"

class storage_file
{
public:
	storage_file();

	static bool exist(const std::string& file_name);
	void open(const std::string& file_name);
	void create(const std::string& file_name);

	void read_block(uint32_t idx, data_block& data);
	void write_block(uint32_t idx, const data_block& data);
	void free_block(uint32_t idx);
	uint32_t next_available_block_idx();
private: 
	// returns if list was changed really
	bool set_block_free(uint32_t idx, bool free);
	bool is_block_free(uint32_t idx);
	uint32_t append_block();
	void write_free_blocks_info();
	void read_free_blocks_info();

	typedef int(*file_closer)(FILE*);
	std::unique_ptr<FILE, file_closer> file_;
	std::set<uint32_t> free_blocks_;
	uint32_t blocks_amount_;
};

