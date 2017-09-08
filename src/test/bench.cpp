#include "bench.hpp"
#include "../util/util.hpp"
#include "../util/config.hpp"

using namespace po;

associative::Parameters* associative::test::TestParameters::parameters = 0;

associative::Parameters& associative::test::TestParameters::get()
{
	if (!parameters)
		throw Exception("parameters have not been parsed yet");
	return *parameters;
}

bool associative::test::TestParameters::parseCommandLine(int argc, char** argv)
{
	auto pair = Parameters::fromCommandLine(Parameters::argvToVector(argc, argv));
	if (!pair)
		return false;
	parameters = new Parameters(pair->first);
	return true;
}


associative::test::Bench::Bench(const associative::Parameters& parameters)
: logger(parameters.logger),
  process(new Process(parameters.target, parameters.dataSource, logger)),
  conn(ConnectionProvider::dispatch(parameters.dataSource, process, logger)), 
  vfs(new VFS(parameters.target, process, logger)), 
  env(process, vfs, conn, parameters.logger)
{
}

associative::test::Bench::~Bench()
{
}
