#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

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

