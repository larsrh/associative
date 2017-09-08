#include <iostream>

#include <boost/lambda/construct.hpp>

#include "test.hpp"
#include "../util/util.hpp"

associative::test::Bench* associative::test::Test::createBench()
{
	auto bench = new Bench(TestParameters::get());
	bench->env.cleanSessions(true);
	benches.insert(bench);
	return bench;
}

void associative::test::Test::SetUp()
{
	testing::Test::SetUp();
	Process::clearSharedMemory(TestParameters::get().dataSource);
}

void associative::test::Test::TearDown()
{
	forEach(benches, lambda::delete_ptr());
	benches.clear();
}

void associative::test::SingleTest::SetUp()
{
	Test::SetUp();
	bench = createBench();
}
