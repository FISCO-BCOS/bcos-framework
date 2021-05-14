#include "libutilities/ConcurrentCOW.h"
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>

using namespace bcos;

struct TestFixture
{
};

BOOST_FIXTURE_TEST_SUITE(ConcurrentCOWTest, TestFixture)

class Item
{
public:
    Item() {}
    Item(const Item&) { ++copyCount; }
    Item(Item&&) { ++moveCount; }

    int copyCount = 0;
    int moveCount = 0;
    int value = 0;

private:
    ConcurrentCOW<int> abc;
};

BOOST_AUTO_TEST_CASE(cow)
{
    ConcurrentCOW<int> num(10);

    ConcurrentCOW<int> num1 = num;
    ConcurrentCOW<int> num2 = num;

    BOOST_CHECK_EQUAL(num.refCount(), 3);

    using COWType = ConcurrentCOW<Item>;
    COWType c((Item()));

    BOOST_CHECK_EQUAL(c->copyCount, 0);
    BOOST_CHECK_EQUAL(c->moveCount, 1);
    BOOST_CHECK_EQUAL(c.refCount(), 1);

    COWType c1 = c;
    COWType c2 = c;
    COWType c3 = c;

    BOOST_CHECK_EQUAL(c.refCount(), 4);
    BOOST_CHECK_EQUAL(c1.refCount(), 4);
    BOOST_CHECK_EQUAL(c2.refCount(), 4);
    BOOST_CHECK_EQUAL(c3.refCount(), 4);

    c3.mutableGet().value = 100;

    BOOST_CHECK_EQUAL(c3.refCount(), 1);
    BOOST_CHECK_EQUAL(c2.refCount(), 3);
    BOOST_CHECK_EQUAL(c1.refCount(), 3);
    BOOST_CHECK_EQUAL(c.refCount(), 3);

    BOOST_CHECK_EQUAL(c1->value, 0);
    BOOST_CHECK_EQUAL(c3->value, 100);

    BOOST_CHECK_EQUAL(c3->copyCount, 1);
    BOOST_CHECK_EQUAL(c->copyCount, 0);

    BOOST_CHECK_EQUAL(c3.empty(), false);

    c3.reset(Item());
    BOOST_CHECK_EQUAL(c3.refCount(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
