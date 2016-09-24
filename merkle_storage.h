#pragma once
#include <memory>
#include <string>
#include <array>
#include "common.h"
#include "storage_file.h"

struct record
{
	uint32_t block_;
	bi::uint256_t value_;
};

typedef std::array<std::pair<record, record>, KEY_LENGTH> merkle_path;


class merkle_storage
{
public:
	static std::unique_ptr<merkle_storage> create(const std::string& file_name);
	static std::unique_ptr<merkle_storage> open(const std::string& file_name);

	void read_value(const bi::uint256_t& key, bi::uint256_t& value);
	void read_value(const bi::uint256_t& key, bi::uint256_t& value, merkle_path& path);
	void write_value(const bi::uint256_t& key, const bi::uint256_t& value);
	void write_value(const bi::uint256_t& key, const bi::uint256_t& value,
		merkle_path& path);
	void delete_value(const bi::uint256_t& key);
	void delete_value(const bi::uint256_t& key, merkle_path& path);
private:
	void init_new_db(const std::string& file_name);
	bool does_key_exist(const bi::uint256_t& key, merkle_path& path);
	void create_key(const bi::uint256_t& key, merkle_path& path);
	void delete_key(const bi::uint256_t& key, merkle_path& path);
	void update_key_hashes(const bi::uint256_t& key, merkle_path& path);

	uint32_t get_value_block_id(const bi::uint256_t& key, const merkle_path& path);
	uint32_t get_value_parent_block_idx(const bi::uint256_t& key, const merkle_path& path);


	merkle_storage();

	merkle_path local_path_stub_;
	storage_file file_;
};

