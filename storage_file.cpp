#include "storage_file.h"
#include "utils.h"
#include "storage_block_parser.h"
#include <array>
#include <iterator>

storage_file::storage_file():
	file_(nullptr, fclose), 
	blocks_amount_(0)
{
}

bool storage_file::exist(const std::string& file_name)
{
	return is_file_exists(file_name);
}

void storage_file::open(const std::string& file_name)
{
	if (!storage_file::exist(file_name))
		throw std::runtime_error("Failed to open file");
	file_ = std::unique_ptr<FILE, file_closer>(
		fopen(file_name.c_str(), "rb+"), 
		fclose
		);
	int n = fseek(file_.get(), 0, SEEK_END);
	if (n)
		throw std::runtime_error("Failed to position cursor");
	long pos = ftell(file_.get());
	blocks_amount_ = pos / BLOCK_SIZE;
	read_free_blocks_info();
}

void storage_file::create(const std::string& file_name)
{
	if (storage_file::exist(file_name))
		throw std::runtime_error("File already exists");
	file_ = std::unique_ptr<FILE, file_closer>(
		fopen(file_name.c_str(), "wb+"),
		fclose
		);
	uint32_t idx = append_block();
	data_block first;
	first.fill(0);
	first[0] = 1;
	write_block(idx, first);
}

void storage_file::read_block(uint32_t idx, data_block& data)
{
	if (!file_)
		throw std::runtime_error("Reading from uninitialized object");
	if (is_block_free(idx))
		throw std::runtime_error("Reading from free block");
	if (idx >= blocks_amount_)
		throw std::runtime_error("Invalid block index");
	int n = fseek(file_.get(), idx * BLOCK_SIZE, SEEK_SET);
	if (n > 0)
		throw std::runtime_error("Failed to seek file to block position");
	n = fread(data.data(), data.size(), 1, file_.get());
	if (n != 1)
		throw std::runtime_error("Failed to read block");
}

void storage_file::write_block(uint32_t idx, const data_block& data)
{
	if (!file_)
		throw std::runtime_error("Writing to uninitialized object");
	if (idx >= blocks_amount_)
		throw std::runtime_error("Invalid block index");
	int n = fseek(file_.get(), idx * BLOCK_SIZE, SEEK_SET);
	if (n > 0)
		throw std::runtime_error("Failed to seek file to block position");
	n = fwrite(data.data(), data.size(), 1, file_.get());
	if (n != 1)
		throw std::runtime_error("Failed to write block");
	if(set_block_free(idx, false))
		write_free_blocks_info();
}

void storage_file::free_block(uint32_t idx)
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	if (idx >= blocks_amount_)
		throw std::runtime_error("Invalid block index");
	if(set_block_free(idx, true))
		write_free_blocks_info();
}

uint32_t storage_file::next_available_block_idx()
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	if (free_blocks_.empty())
		free_blocks_.insert(append_block());
	return *(free_blocks_.begin());
}

bool storage_file::set_block_free(uint32_t idx, bool free)
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	bool changed = false;
	auto it = free_blocks_.find(idx);
	if (free)
	{
		changed = (it == free_blocks_.end());
		free_blocks_.insert(idx);
	}
	else
	{
		changed = (it != free_blocks_.end());
		free_blocks_.erase(idx);
	}
	return changed;
}

bool storage_file::is_block_free(uint32_t idx)
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	return (free_blocks_.find(idx) != free_blocks_.end());
}

uint32_t storage_file::append_block()
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	int n = fseek(file_.get(), 0, SEEK_END);
	if (n)
		throw std::runtime_error("Failed to position cursor");
	long pos = ftell(file_.get());
	data_block b;
	b.fill(0);
	n = fwrite(b.data(), b.size(), 1, file_.get());
	if (n != 1)
		throw std::runtime_error("Failed to append block");
	blocks_amount_++;
	return blocks_amount_ - 1;
}

void storage_file::write_free_blocks_info()
{
	// add old blocks with free block info to the list
	uint32_t idx = 0;
	data_block data;
	storage_block_parser parser(data);
	while (true)
	{
		read_block(idx, data);
		idx = parser.get_first_child_id();
		if(idx > 0)
			free_blocks_.insert(idx);
		else
			break;
	}
	uint32_t parent_idx = 0;
	uint32_t count = 0;
	uint32_t max_count = BLOCK_VALUE_SIZE / sizeof(uint32_t);
	std::set<uint32_t> free_blocks(free_blocks_);
	while (true)
	{
		parser.clear();
		parser.set_type(STORAGE_FREE_INFO_BLOCK_TYPE);
		count = 0;
		while (!free_blocks.empty())
		{
			parser.set_32value(count, *free_blocks.begin());
			count++;
			free_blocks.erase(free_blocks.begin());
			if (count == max_count)
				break;
		}
		parser.set_parent_id(parent_idx);
		parser.set_second_child_id(count);
		if (free_blocks.empty())
		{
			write_block(idx, data);
			break;
		}
		parent_idx = idx;
		idx = *free_blocks.begin();
		parser.set_first_child_id(idx);
		free_blocks.erase(idx);
		free_blocks_.erase(idx);
		write_block(parent_idx, data);
	}
}

void storage_file::read_free_blocks_info()
{
	if (!file_)
		throw std::runtime_error("Uninitialized object");
	free_blocks_.clear();
	uint32_t idx = 0;
	data_block data;
	storage_block_parser parser(data);
	while (true)
	{
		read_block(idx, data);
		idx = parser.get_first_child_id();
		uint32_t count = parser.get_second_child_id();
		for (uint32_t i = 0; i < count; i++)
			free_blocks_.insert(parser.get_32value(i));
		if (idx == 0)
			break;
	}
}
