#include "merkle_storage.h"

using namespace bi;

merkle_storage::merkle_storage()
{
}

std::unique_ptr<merkle_storage> merkle_storage::create(const std::string& file_name)
{
	std::unique_ptr<merkle_storage> res(new merkle_storage());
	res->file_.create(file_name);
	return res;
}

std::unique_ptr<merkle_storage> merkle_storage::open(const std::string& file_name)
{
	std::unique_ptr<merkle_storage> res(new merkle_storage());
	res->file_.open(file_name);
	return res;
}

void merkle_storage::read_path(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::read_value(const uint256_t& key, uint256_t& value)
{
	read_value(key, value, local_path_stub_);
}

void merkle_storage::read_value(const uint256_t& key, uint256_t& value, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::write_value(const uint256_t& key, const uint256_t& value)
{
	write_value(key, value, local_path_stub_);
}

void merkle_storage::write_value(const uint256_t& key, const uint256_t& value,
	merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::delete_value(const uint256_t& key)
{
	delete_value(key, local_path_stub_);
}

void merkle_storage::delete_value(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

bool merkle_storage::does_key_exist(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::create_key(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::delete_key(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::update_key_hashes(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}
