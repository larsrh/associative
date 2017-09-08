#ifndef ASSOCIATIVE_TEST_HPP
#define ASSOCIATIVE_TEST_HPP

#include <unordered_set>

#include <gtest/gtest.h>

#include "bench.hpp"

namespace associative
{
	
	namespace test
	{
	
		class Test : public testing::Test
		{
		private:
			std::unordered_set<Bench*> benches;
			
		protected:
			virtual void SetUp();
			virtual void TearDown();
			Bench* createBench();
		};
		
		class SingleTest : public Test
		{
		protected:
			virtual void SetUp();
			Bench* bench;
		};
	
	}
	
}

#endif