#include "merkle_storage.h"
#include "utils.h"
#include "storage_block_parser.h"

using namespace bi;

#define MERKLE_ROOT_BLOCK 1

merkle_storage::merkle_storage()
{
}

std::unique_ptr<merkle_storage> merkle_storage::create(const std::string& file_name)
{
	std::unique_ptr<merkle_storage> res(new merkle_storage());
	res->init_new_db(file_name);
	return res;
}

std::unique_ptr<merkle_storage> merkle_storage::open(const std::string& file_name)
{
	std::unique_ptr<merkle_storage> res(new merkle_storage());
	res->file_.open(file_name);
	return res;
}

void merkle_storage::read_value(const uint256_t& key, uint256_t& value)
{
	read_value(key, value, local_path_stub_);
}

void merkle_storage::read_value(const uint256_t& key, uint256_t& value, merkle_path& path)
{
	if (!does_key_exist(key, path))
		throw std::runtime_error("Reading nonexisting key");
	uint32_t value_block_idx = get_value_block_id(key, path);
	data_block data;
	file_.read_block(value_block_idx, data);
	storage_block_parser parser(data);
	parser.get_value(value);
}

void merkle_storage::write_value(const uint256_t& key, const uint256_t& value)
{
	write_value(key, value, local_path_stub_);
}

void merkle_storage::write_value(const uint256_t& key, const uint256_t& value,
	merkle_path& path)
{
	if (!does_key_exist(key, path))
		create_key(key, path);
	uint32_t value_block_idx = get_value_block_id(key, path);
	uint32_t parent_block_idx = get_value_parent_block_idx(key, path);
	data_block data;
	storage_block_parser parser(data);
	parser.clear();
	parser.set_type(VALUE_BLOCK_TYPE);
	parser.set_parent_id(parent_block_idx);
	parser.set_value(value);
	file_.write_block(value_block_idx, data);
}

void merkle_storage::delete_value(const uint256_t& key)
{
	delete_value(key, local_path_stub_);
}

void merkle_storage::delete_value(const uint256_t& key, merkle_path& path)
{
	if (!does_key_exist(key, path))
		throw std::runtime_error("Deleting nonexisting key");
	uint32_t value_block_idx = get_value_block_id(key, path);
	file_.free_block(value_block_idx);
	uint32_t idx = get_value_parent_block_idx(key, path);
	data_block data;
	storage_block_parser parser(data);
	file_.read_block(idx, data);
	file_.free_block(idx);
	uint32_t parent_idx = parser.get_parent_id();
	while (parent_idx != 0)
	{
		file_.read_block(parent_idx, data);
		if (parser.get_first_child_id() == idx)
		{
			if (parser.get_second_child_id() != 0)
			{
				parser.set_first_child_id(0);
				file_.write_block(parent_idx, data);
				break;
			}
		}
		else
		{
			if (parser.get_first_child_id() != 0)
			{
				parser.set_second_child_id(0);
				file_.write_block(parent_idx, data);
				break;
			}
		}
		if (parent_idx == MERKLE_ROOT_BLOCK)
		{
			parser.set_first_child_id(0);
			parser.set_second_child_id(0);
			file_.write_block(parent_idx, data);
		}
		else
		{
			file_.free_block(parent_idx);
		}
		idx = parent_idx;
		parent_idx = parser.get_parent_id();
	}
}

void merkle_storage::init_new_db(const std::string & file_name)
{
	file_.create(file_name);
	uint32_t root_idx = file_.next_available_block_idx();
	data_block root;
	storage_block_parser parser(root);
	parser.clear();
	parser.set_type(MERKLE_NODE_BLOCK_TYPE);
	file_.write_block(root_idx, root);
}

bool merkle_storage::does_key_exist(const bi::uint256_t & key)
{
	return does_key_exist(key, local_path_stub_);
}

bool merkle_storage::does_key_exist(const uint256_t& key, merkle_path& path)
{
	uint32_t idx = MERKLE_ROOT_BLOCK;
	data_block data;
	storage_block_parser parser(data);
	for (unsigned i = 0; i < KEY_LENGTH; i++)
	{
		file_.read_block(idx, data);
		path[i].first.block_ = parser.get_first_child_id();
		path[i].second.block_ = parser.get_second_child_id();
		if ((key >> i) % 2 == 0)
			idx = path[i].first.block_;
		else
			idx = path[i].second.block_;
		if (idx == 0)
			return false;
	}
	return true;
}

void merkle_storage::create_key(const uint256_t& key, merkle_path& path)
{
	uint32_t idx = MERKLE_ROOT_BLOCK;
	data_block data;
	storage_block_parser parser(data);
	for (unsigned i = 0; i < KEY_LENGTH; i++)
	{
		file_.read_block(idx, data);
		path[i].first.block_ = parser.get_first_child_id();
		path[i].second.block_ = parser.get_second_child_id();
		uint32_t new_idx;
		if ((key >> i) % 2 == 0)
			new_idx = path[i].first.block_;
		else
			new_idx = path[i].second.block_;
		if (new_idx == 0)
		{
			new_idx = file_.next_available_block_idx();
			if ((key >> i) % 2 == 0)
			{
				parser.set_first_child_id(new_idx);
				path[i].first.block_ = new_idx;
			}
			else
			{
				parser.set_second_child_id(new_idx);
				path[i].second.block_ = new_idx;
			}
			file_.write_block(idx, data);
			parser.clear();
			parser.set_type(MERKLE_NODE_BLOCK_TYPE);
			parser.set_parent_id(idx);
			file_.write_block(new_idx, data);
		}
		idx = new_idx;
	}
	// create value block
	uint32_t new_idx = file_.next_available_block_idx();
	file_.read_block(idx, data);
	parser.set_first_child_id(new_idx);
	file_.write_block(idx, data);
	parser.clear();
	parser.set_type(VALUE_BLOCK_TYPE);
	parser.set_parent_id(idx);
	file_.write_block(new_idx, data);
}

void merkle_storage::delete_key(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

void merkle_storage::update_key_hashes(const uint256_t& key, merkle_path& path)
{
	throw std::runtime_error("Not implemented");
}

uint32_t merkle_storage::get_value_block_id(const bi::uint256_t & key, const merkle_path & path)
{
	uint32_t parent = get_value_parent_block_idx(key, path);
	data_block data;
	storage_block_parser parser(data);
	file_.read_block(parent, data);
	return parser.get_first_child_id();
}

uint32_t merkle_storage::get_value_parent_block_idx(const bi::uint256_t & key, const merkle_path & path)
{
	if (key >> (KEY_LENGTH - 1))
		return path[KEY_LENGTH - 1].second.block_;
	return path[KEY_LENGTH - 1].first.block_;
}
