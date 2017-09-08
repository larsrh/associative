#include <gtest/gtest.h>

#include "test.hpp"
#include "bench.hpp"

using namespace associative::test;

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	
	if (!TestParameters::parseCommandLine(argc, argv))
		return 1;
	
	return RUN_ALL_TESTS();
}
