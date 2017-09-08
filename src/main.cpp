#include <iostream>
#include <typeinfo>

#include "util/config.hpp"
#include "util/exception.hpp"
#include "actions/action.hpp"
#include "env/process.hpp"
#include "env/environment.hpp"
#include "db/connection.hpp"

using namespace associative;

int main(int argc, char **argv)
{
#ifndef ASSOCIATIVE_DEBUG
	try
	{
#endif
		auto params = MainParameters::fromCommandLine(Parameters::argvToVector(argc - 1, argv + 1));
		if (!params)
			return 1;
		
		auto process = boost::shared_ptr<Process>(new Process(params->target, params->dataSource, params->logger, params->options.count("clear-shm")));
		auto vfs = boost::shared_ptr<VFS>(new VFS(params->target, process, params->logger));
		auto conn = boost::shared_ptr<Connection>(ConnectionProvider::dispatch(params->dataSource, process, params->logger));
		Environment env(process, vfs, conn, params->logger);
		
		env.startSession();
		int ret = params->parseFurther()->dispatch(env);
		env.commitSession(IsolationLevel::getIsolationLevel(params->options["isolation-level"].as<std::string>()));
#ifdef ASSOCIATIVE_DEBUG
		std::cout << "Result: " << ret << std::endl;
#endif
		return ret;
#ifndef ASSOCIATIVE_DEBUG
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception of type " << typeid(e).name() << " thrown" << std::endl;
		std::cout << e.what() << std::endl;
		return 1;
	}
#endif
}
