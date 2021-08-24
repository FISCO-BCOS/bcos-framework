#include "../../../testutils/TestPromptFixture.h"
#include "Hash.h"
#include "libtable/TableFactory.h"
#include "libutilities/Common.h"
#include "testutils/faker/FakeStorage.h"
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <future>

namespace bcos::test
{
struct TablePerfFixture
{
    TablePerfFixture()
    {
        auto hashImpl = std::make_shared<bcos::crypto::Header256Hash>();
        auto memoryStorage = std::make_shared<FakeStorage>();
        BOOST_TEST(memoryStorage != nullptr);
        tableFactory = std::make_shared<TableFactory>(memoryStorage, hashImpl, 0);
        BOOST_TEST(tableFactory != nullptr);
    }

    std::vector<Entry::Ptr> createTestData(TableInterface::Ptr table)
    {
        std::vector<Entry::Ptr> entries;
        entries.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            auto entry = table->newEntry();
            entry->setField("key", "key_" + boost::lexical_cast<std::string>(i));
            entry->setField("field1", "value1");
            entry->setField("field2", "value2");
            entry->setField("field3", "value" + boost::lexical_cast<std::string>(i));

            entries.emplace_back(std::move(entry));
        }

        return entries;
    }

    std::shared_ptr<TableFactory> tableFactory;
    size_t count = 100 * 10000;
};

BOOST_FIXTURE_TEST_SUITE(TablePerf, TablePerfFixture)

BOOST_AUTO_TEST_CASE(sync)
{
    tableFactory->createTable("test_table", "key", "field1,field2,field3");
    auto table = tableFactory->openTable("test_table");

    auto entries = createTestData(table);

    for (auto& entry : entries)
    {
        table->setRow(entry->getField("key"), entry);
    }

    auto now = bcos::utcSteadyTime();
    for (size_t i = 0; i < count; ++i)
    {
        std::string key = "key_" + boost::lexical_cast<std::string>(i);
        auto entry = table->getRow(key);

        BOOST_CHECK_EQUAL(entry->getFieldConst("field1"), "value1");
        BOOST_CHECK_EQUAL(entry->getFieldConst("field2"), "value2");
        BOOST_CHECK_EQUAL(
            entry->getFieldConst("field3"), "value" + boost::lexical_cast<std::string>(i));
    }

    std::cout << "sync cost: " << bcos::utcSteadyTime() - now << std::endl;
}

BOOST_AUTO_TEST_CASE(async)
{
    tableFactory->createTable("test_table", "key", "field1,field2,field3");
    auto table = tableFactory->openTable("test_table");

    auto entries = createTestData(table);

    for (auto& entry : entries)
    {
        table->setRow(entry->getField("key"), entry);
    }

    auto total = count;

    auto now = bcos::utcSteadyTime();
    std::promise<bool> finished;
    std::atomic<size_t> done(0);
    for (size_t i = 0; i < count; ++i)
    {
        std::string key = "key_" + boost::lexical_cast<std::string>(i);
        table->asyncGetRow(
            key, [i, &total, &finished, &done](const Error::Ptr&, const Entry::Ptr& entry) {
                BOOST_CHECK_EQUAL(entry->getFieldConst("field1"), "value1");
                BOOST_CHECK_EQUAL(entry->getFieldConst("field2"), "value2");
                BOOST_CHECK_EQUAL(
                    entry->getFieldConst("field3"), "value" + boost::lexical_cast<std::string>(i));

                auto current = done.fetch_add(1);
                if (current + 1 >= total)
                {
                    finished.set_value(true);
                }
            });
    }
    finished.get_future().get();

    std::cout << "async cost: " << bcos::utcSteadyTime() - now << std::endl;
}

BOOST_AUTO_TEST_CASE(asyncToSync)
{
    tableFactory->createTable("test_table", "key", "field1,field2,field3");
    auto table = tableFactory->openTable("test_table");

    auto entries = createTestData(table);

    for (auto& entry : entries)
    {
        table->setRow(entry->getField("key"), entry);
    }

    auto now = bcos::utcSteadyTime();
    for (size_t i = 0; i < count; ++i)
    {
        std::string key = "key_" + boost::lexical_cast<std::string>(i);
        std::promise<bool> finished;
        table->asyncGetRow(key, [i, &finished](const Error::Ptr&, const Entry::Ptr& entry) {
            BOOST_CHECK_EQUAL(entry->getFieldConst("field1"), "value1");
            BOOST_CHECK_EQUAL(entry->getFieldConst("field2"), "value2");
            BOOST_CHECK_EQUAL(
                entry->getFieldConst("field3"), "value" + boost::lexical_cast<std::string>(i));
            finished.set_value(true);
        });
        finished.get_future().get();
    }

    std::cout << "asyncToSync cost: " << bcos::utcSteadyTime() - now << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace bcos::test