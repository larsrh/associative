#ifndef ASSOCIATIVE_TEST_BENCH_HPP
#define ASSOCIATIVE_TEST_BENCH_HPP

#include <string>

#include <boost/optional.hpp>

#include "../util/parameters.hpp"
#include "../env/environment.hpp"

namespace associative
{
	
	namespace test
	{
		
		class TestParameters
		{
		private:
			static Parameters* parameters;
			
		public:
			static Parameters& get();
			static bool parseCommandLine(int argc, char** argv);
		};
		
		class Bench
		{
		public:
			boost::shared_ptr<Logger> logger;
			boost::shared_ptr<Process> process;
			boost::shared_ptr<Connection> conn;
			boost::shared_ptr<VFS> vfs;
			Environment env;
			
			Bench(const Parameters& parameters);
			virtual ~Bench();
			
			Bench& operator=(Bench&) = delete;
			Bench(Bench&) = delete;
		};
		
	}
	
}

#endif