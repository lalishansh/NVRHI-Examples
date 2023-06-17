#define BOOST_TEST_MODULE HelloWorldTests
#include <boost/test/included/unit_test.hpp>

#include <fmt/core.h>
#include <nvrhi/nvrhi.h>

#include <version.h>

BOOST_AUTO_TEST_CASE(hello_world_test) {
    std::string output = fmt::format("{}.{}.{}",
                                     NVRHI_EXAMPLES_VERSION_MAJOR,
                                     NVRHI_EXAMPLES_VERSION_MINOR,
                                     NVRHI_EXAMPLES_VERSION_PATCH);
    BOOST_TEST(output == NVRHI_EXAMPLES_VERSION_STRING);
}

BOOST_AUTO_TEST_CASE(nvrhi_test) {
	BOOST_TEST(nvrhi::verifyHeaderVersion());
}
