#include <iostream>

#include <boost/functional.hpp>

#include "../action.hpp"
#include "../../util/io.hpp"

using namespace po;

namespace associative
{
	
	class ShellAction : public Action
	{
		COMMANDLINE_DECL;
		
	private:
		static int performLine(const std::string& line, Environment& env)
		{
			auto tokens = parseArguments(line);
			if (!tokens.size())
				return 0;
			
			if (tokens[0] == ":commit")
			{
				auto& level = IsolationLevel::getIsolationLevel(tokens.size() > 1 ? boost::make_optional(tokens[1]) : boost::none);
				env.commitSession(level);
				env.startSession();
				return 0;
			}
			else if (tokens[0] == ":rollback")
			{
				env.rollbackSession();
				env.startSession();
				return 0;
			}
			else
			{
				auto params = ActionParameters::fromCommandLine(tokens);
				int result = params ? params->dispatch(env) : -1;
				std::cout << "Result: " << result << std::endl;
				return result;
			}
		}
		
	protected:
		virtual options_description* desc()
		{
			return new options_description();
		}
		
	public:
		ShellAction()
		: Action("shell")
		{
		}
		
		virtual int perform(const variables_map&, const std::vector<std::string>& parameters, Environment& env)
		{
			doREPL(parameters, boost::bind2nd(boost::ptr_fun(&performLine), env));
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(Shell);
