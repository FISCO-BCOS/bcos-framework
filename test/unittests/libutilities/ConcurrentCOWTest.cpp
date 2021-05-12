#include "libutilities/ConcurrentCOW.h"
#include <boost/test/unit_test.hpp>

using namespace bcos;

struct TestFixture {

};

BOOST_FIXTURE_TEST_SUITE(ConcurrentCOWTest, TestFixture)

BOOST_AUTO_TEST_CASE(cow) {
    ConcurrentCOW<int> num(10);
    
    ConcurrentCOW<int> num1 = num;
    ConcurrentCOW<int> num2 = num;

    BOOST_CHECK_EQUAL(num.refCount(), 3);
}

BOOST_AUTO_TEST_SUITE_END()