#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include "../storage_block_parser.h"
#include "../merkle_storage.h"
#include "../storage_file.h"
#include "../utils.h"

using namespace std;

struct NoTestDBFixture
{
	NoTestDBFixture()
	{
		if (is_file_exists("test.db"))
			delete_file("test.db");
	}
	~NoTestDBFixture()
	{
		if (is_file_exists("test.db"))
			delete_file("test.db");
	}
};

BOOST_AUTO_TEST_CASE(storage_block_parser_test)
{
	data_block data;
	storage_block_parser parser(data);
	uint8_t type = 233;
	uint32_t idx = (ULONG_MAX >> 1) - 2;
	bi::uint256_t val1 = (bi::uint256_max >> 1) - 2;
	parser.set_type(type);
	parser.set_parent_id(idx);
	parser.set_first_child_id(idx - 2);
	parser.set_second_child_id(idx - 4);
	parser.set_value(val1);
	BOOST_REQUIRE_EQUAL(parser.get_type(), type);
	BOOST_REQUIRE_EQUAL(parser.get_parent_id(), idx);
	BOOST_REQUIRE_EQUAL(parser.get_first_child_id(), idx - 2);
	BOOST_REQUIRE_EQUAL(parser.get_second_child_id(), idx - 4);
	bi::uint256_t val2(0);
	parser.get_value(val2);
	BOOST_REQUIRE_EQUAL(val1, val2);
}

BOOST_FIXTURE_TEST_CASE(storage_file_create_open, NoTestDBFixture)
{
	BOOST_REQUIRE_EQUAL(storage_file::exist("test.db"), false);
	{
		storage_file storage;
		BOOST_REQUIRE_THROW(storage.open("test.db"), std::exception);
		BOOST_REQUIRE_NO_THROW(storage.create("test.db"));
	}
	BOOST_REQUIRE_EQUAL(storage_file::exist("test.db"), true);
	storage_file storage;
	BOOST_REQUIRE_THROW(storage.create("test.db"), std::exception);
	BOOST_REQUIRE_NO_THROW(storage.open("test.db"));
}

BOOST_FIXTURE_TEST_CASE(storage_file_read_write, NoTestDBFixture)
{
	storage_file storage;
	storage.create("test.db");
	uint32_t idx = storage.next_available_block_idx();
	BOOST_REQUIRE_EQUAL(idx, 1);
	data_block b1, b2;
	b1.fill(5);
	storage.write_block(idx, b1);
	storage.read_block(idx, b2);
	for (size_t i = 0; i < b1.size(); i++)
		BOOST_REQUIRE_EQUAL(b1[i], b2[i]);
	uint32_t idx2 = storage.next_available_block_idx();
	BOOST_REQUIRE_EQUAL(idx2, 2);
	storage.write_block(idx2, b1);
	BOOST_REQUIRE_THROW(storage.read_block(3, b2), std::exception);
	BOOST_REQUIRE_THROW(storage.write_block(3, b2), std::exception);
	storage.free_block(idx);
	uint32_t idx3 = storage.next_available_block_idx();
	BOOST_REQUIRE_EQUAL(idx3, idx);
	BOOST_REQUIRE_THROW(storage.read_block(idx3, b1), std::exception);
}

BOOST_FIXTURE_TEST_CASE(storage_file_free_blocks, NoTestDBFixture)
{
	uint32_t idx1, idx2;
	{
		storage_file storage;
		storage.create("test.db");
		data_block b;
		b.fill(5);
		uint32_t idx = storage.next_available_block_idx();
		storage.write_block(idx, b);
		idx = storage.next_available_block_idx();
		storage.write_block(idx, b);
		idx = storage.next_available_block_idx();
		storage.write_block(idx, b);
		storage.free_block(1);
		idx1 = storage.next_available_block_idx();
	}
	{
		storage_file storage;
		storage.open("test.db");
		idx2 = storage.next_available_block_idx();
	}
	BOOST_REQUIRE_EQUAL(idx1, idx2);
	storage_file storage;
	storage.open("test.db");
	uint32_t idx;
	data_block b;
	b.fill(5);
	for (unsigned i = 0; i < 100; i++)
	{
		idx = storage.next_available_block_idx();
		BOOST_REQUIRE_NO_THROW(storage.write_block(idx, b));
	}
	for (unsigned i = 2; i < 90; i++)
	{
		BOOST_REQUIRE_NO_THROW(storage.free_block(i));
	}
}

BOOST_FIXTURE_TEST_CASE(merkle_storage_create_open, NoTestDBFixture)
{
	{
		BOOST_REQUIRE_THROW(merkle_storage::open("test.db"), std::exception);
		BOOST_REQUIRE_NO_THROW(merkle_storage::create("test.db"));
	}
	BOOST_REQUIRE_EQUAL(is_file_exists("test.db"), true);
	BOOST_REQUIRE_THROW(merkle_storage::create("test.db"), std::exception);
	BOOST_REQUIRE_NO_THROW(merkle_storage::open("test.db"));
}

BOOST_FIXTURE_TEST_CASE(merkle_storage_simple_read_write, NoTestDBFixture)
{
	auto ms = merkle_storage::create("test.db");
	bi::uint256_t key(100500);
	bi::uint256_t value(1);
	BOOST_REQUIRE_NO_THROW(ms->write_value(key, value));
	bi::uint256_t value2;
	BOOST_REQUIRE_NO_THROW(ms->read_value(key, value2));
	BOOST_REQUIRE_EQUAL(value, value2);
}

BOOST_FIXTURE_TEST_CASE(merkle_storage_read_write_delete, NoTestDBFixture)
{
	auto ms = merkle_storage::create("test.db");
	bi::uint256_t key1(100500);
	bi::uint256_t value1(1);
	bi::uint256_t key2(123456);
	bi::uint256_t value2(2);
	ms->write_value(key1, value1);
	ms->write_value(key2, value2);
	bi::uint256_t value3;
	ms->read_value(key1, value3);
	BOOST_REQUIRE_EQUAL(value3, value1);
	ms->read_value(key2, value3);
	BOOST_REQUIRE_EQUAL(value3, value2);
	ms->write_value(key1, value2);
	ms->read_value(key1, value3);
	BOOST_REQUIRE_EQUAL(value3, value2);
	BOOST_REQUIRE_EQUAL(ms->does_key_exist(key1), true);
	BOOST_REQUIRE_EQUAL(ms->does_key_exist(key2), true);
	BOOST_REQUIRE_NO_THROW(ms->delete_value(key1));
	BOOST_REQUIRE_EQUAL(ms->does_key_exist(key1), false);
	BOOST_REQUIRE_EQUAL(ms->does_key_exist(key2), true);
	BOOST_REQUIRE_NO_THROW(ms->read_value(key2, value3));
	BOOST_REQUIRE_EQUAL(value3, value2);
	BOOST_REQUIRE_THROW(ms->read_value(key1, value3), std::exception);
	BOOST_REQUIRE_THROW(ms->delete_value(key1), std::exception);
	BOOST_REQUIRE_NO_THROW(ms->write_value(key1, value1));
}

