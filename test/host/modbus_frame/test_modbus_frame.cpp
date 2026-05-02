#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "modbus_frame.hpp"


TEST_CASE("sanity") {
    CHECK(1 + 1 == 2);
}